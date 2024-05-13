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
#include "codegen.h"
#include "parser.h"
#include "lexer.h"
#include "log.h"

using namespace llvm;
using namespace llvm::orc;   // the name 'KaleidoscopeJIT' is in the namespace llvm::orc 

/*/////////////////////////////////////
 *
 *   Top-level Parse and JIT Driver
 *
 */
////////////////////////////////////
void InitializeModuleAndPassManager(){
    //  open a new context and module
    TheContext=std::make_unique<LLVMContext>();
    TheModule=std::make_unique<Module>("my cool jit",*TheContext);
    
    TheModule->setDataLayout(TheJIT->getDataLayout());


    //  create a new builder for the module
    Builder=std::make_unique<IRBuilder<>>(*TheContext);

    //  Create a new pass manager attached to it.
    TheFPM=std::make_unique<legacy::FunctionPassManager>(TheModule.get());

    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->add(createInstructionCombiningPass());
    // Reassociate expressions.
    TheFPM->add(createReassociatePass());
    // Eliminate Common SubExpressions.
    TheFPM->add(createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->add(createCFGSimplificationPass());

    TheFPM->doInitialization();
}


void HandleDefinition()
{
    if (auto FnAST=ParseDefinition())
    {
        if(auto *FnIR=FnAST->codegen()){
            fprintf(stderr, "Read a function definition:");
            FnIR->print(errs());
            fprintf(stderr,"\n");
            ExitOnErr(TheJIT->addModule(
                ThreadSafeModule(std::move(TheModule), std::move(TheContext))));
            InitializeModuleAndPassManager();
        }
    }
    else
    {
        // Skip token for error recovery.
        getNextToken();
    }
}

void HandleExtern()
{
    if (auto ProtoAST=ParseExtern())
    {
        if(auto *FnIR=ProtoAST->codegen()){
            fprintf(stderr, "Read extern:");
            FnIR->print(errs());
            fprintf(stderr,"\n");
            FunctionProtos[ProtoAST->getName()]=std::move(ProtoAST);
        }
    }
    else
    {
        // Skip token for error recovery.
        getNextToken();
    }
}

void HandleTopLevelExpression()
{
    //  Evaluate a top-level expression into an anonymous function.
    if (auto FnAST=ParseTopLevelExpr())
    {
        if(FnAST->codegen()){
            auto RT=TheJIT->getMainJITDylib().createResourceTracker();

            auto TSM=ThreadSafeModule(std::move(TheModule),std::move(TheContext));
            ExitOnErr(TheJIT->addModule(std::move(TSM),RT));
            InitializeModuleAndPassManager();
            
            auto ExprSymbol=ExitOnErr(TheJIT->lookup("__anon_expr"));
            double (*FP)() =(double (*)())(intptr_t)ExprSymbol.getAddress();
            fprintf(stderr,"Evaluated to %f\n",FP());

            //  Delete the anonymous expression module from the JIT.
            //  Accordingly, the defined function will be free along with it
            ExitOnErr(RT->remove());
        }
    }
    else
    {
        // Skip token for error recovery.
        getNextToken();
    }
}

/// eof  |  ';' | definition   |     extern  | expression
void MainLoop()
{
    while (true)
    {
        fprintf(stderr, "ready> ");
        switch (CurTok)
        {
        case tok_eof:
            return;
        case ';':
            getNextToken();
            break;
        case tok_def:
            HandleDefinition();
            break;
        case tok_extern:
            HandleExtern();
            break;
        default:
            HandleTopLevelExpression();
            break;
        }
    }
}

int main()
{
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();


    // Install standard binary operators.
    // 1 is lowest precedence.
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40; // highest.

    // Prime the first token.
    fprintf(stderr, "ready> ");
    getNextToken();


    TheJIT = ExitOnErr(KaleidoscopeJIT::Create());
    InitializeModuleAndPassManager();

    // Run the main "interpreter loop" now.
    MainLoop();

    return 0;
}