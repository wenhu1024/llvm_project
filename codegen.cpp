#include "KaleidoscopeJIT.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ast.h"
#include "log.h"

using namespace llvm;
using namespace llvm::orc;   // the name 'KaleidoscopeJIT' is in the namespace llvm::orc 

std::unique_ptr<LLVMContext> TheContext;
std::unique_ptr<Module> TheModule;
std::unique_ptr<IRBuilder<>> Builder;
std::map<std::string, Value *> NamedValues;
std::unique_ptr<legacy::FunctionPassManager> TheFPM;
std::unique_ptr<KaleidoscopeJIT> TheJIT;
std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;
ExitOnError ExitOnErr;

/*///////////////////////////////////////////
*
*   Code Generation
*   
*   LLVM IR generation
*
*///////////////////////////////////////////



Value *LogErrorV(const char * Str){
    LogError(Str);
    return nullptr;
}



Function *getFunction(std::string Name){
    //  first, look the function up in the current modlue
    if(auto *F=TheModule->getFunction(Name)){
        return F;
    }

    //  If not, check whether we can codegen the declaration from some existing
    //  prototype.
    auto FI=FunctionProtos.find(Name);
    if(FI != FunctionProtos.end()){
        return FI->second->codegen();
    }

    //  If no existing prototype exists, return null.
    return nullptr;
}



Value *NumberExprAST::codegen(){
    return ConstantFP::get(*TheContext,APFloat(Val));
}


Value *VariableExprAST::codegen(){
    //  look this variable up through 'NamedValues'
    //  In practice, the only values that can be in the NamedValues map are function arguments. 1
    Value *V =NamedValues[Name];
    if(!V){
        LogError("Unknown variable name");
    }
    return V;
}

Value *BinaryExprASTP::codegen(){
    Value *L=LHS->codegen();
    Value *R=RHS->codegen();

    if(!L || !R){
        return nullptr;
    }

    switch(Op){
    case '+':
        return Builder->CreateFAdd(L,R,"addtmp");
    case '-':
        return Builder->CreateFSub(L,R,"subtmp");
    case '*':
        return Builder->CreateFMul(L,R,"multmp");
    case '<':
        L=Builder->CreateFCmpULT(L,R,"cmptmp");
        // convert bool(ont bit, 0/1) to double (0.0 / 1.0)
        return Builder->CreateUIToFP(L,Type::getDoubleTy(*TheContext),"booltmp");
    default:
        return LogErrorV("invalid binary operator"); 
    }
}

Value *CallExprAST::codegen(){
    // look the fuction name through 'getFunction'
    Function *CalleeF=getFunction(Callee);
    if(!CalleeF){
        return LogErrorV("Unknown function referenced");
    }

    if(CalleeF->arg_size() != Args.size()){
        return LogErrorV("Incorrect # arguments passed");
    }

    std::vector<Value *> ArgsV;
    for(unsigned i=0,e=Args.size();i!=e;++i){
        ArgsV.push_back(Args[i]->codegen());
        if(!ArgsV.back()){
            return nullptr;
        }
    }

    return Builder->CreateCall(CalleeF,ArgsV,"calltmp");
}



Function *PrototypeAST::codegen(){
    // make the function type:  double(double,double...)
    std::vector<Type*> Doubles(Args.size(),Type::getDoubleTy(*TheContext));

    FunctionType *FT = 
        FunctionType::get(Type::getDoubleTy(*TheContext),Doubles,false);

    Function *F =
        Function::Create(FT,Function::ExternalLinkage,Name,TheModule.get());

    unsigned Idx=0;
    for(auto &Arg:F->args()){
        Arg.setName(Args[Idx++]);
    }

    return F;
}




/// NOTE: a small bug exists
/// extern foo(a);     # ok, defines foo.
/// def foo(b) b;      # Error: Unknown variable name. (decl using 'a' takes precedence).
Function *FunctionAST::codegen(){

    auto &P=*Proto;
    FunctionProtos[Proto->getName()] = std::move(Proto);
    Function *TheFunction =getFunction(P.getName());
    if(!TheFunction){
        return nullptr;
    }

    //  create a basic block named "entry",which is inserted into TheFunction
    BasicBlock *BB=BasicBlock::Create(*TheContext,"entry",TheFunction);

    //  tell the builder that new instructions should 
    //  be inserted into the end of the new basic block
    Builder->SetInsertPoint(BB);

    //  record the function args in the NameValues map
    //  map format: value_name_string(key) -> pointer to the namevalue(value) 
    NamedValues.clear();
    for(auto &Arg: TheFunction->args()){
        NamedValues[std::string(Arg.getName())]=&Arg;
    }


    if(Value *RetVal=Body->codegen()){
        //  finish off the function
        Builder->CreateRet(RetVal);

        //  Validate the generated code
        //  catch lots of bugs
        verifyFunction(*TheFunction);

        //  optimize the function
        TheFPM->run(*TheFunction);

        return TheFunction;
    }

    //  when reading error function body that RetVal is nullptr, 
    //  remove it 
    TheFunction->eraseFromParent();
    return nullptr;
}