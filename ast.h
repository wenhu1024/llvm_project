#ifndef AST_H
#define AST_H

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


using namespace llvm;
using namespace llvm::orc;   // the name 'KaleidoscopeJIT' is in the namespace llvm::orc 

class ExprAST
{
public:
    virtual ~ExprAST() = default;
    virtual Value *codegen()=0;
};

class NumberExprAST : public ExprAST
{
private:
    double Val;

public:
    NumberExprAST(double Val) : Val(Val) {}
    Value *codegen() override;
};

class VariableExprAST : public ExprAST
{
private:
    std::string Name;

public:
    VariableExprAST(const std::string &Name) : Name(Name) {}
    
    Value *codegen() override;
};

class BinaryExprASTP : public ExprAST
{
private:
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;

public:
    BinaryExprASTP(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
        : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    
    Value *codegen() override;
};

class CallExprAST : public ExprAST
{
private:
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    CallExprAST(const std::string& Callee,
        std::vector<std::unique_ptr<ExprAST>> Args)
        :Callee(Callee),Args(std::move(Args)){}

    Value *codegen() override;
};

class PrototypeAST{
private:
    std::string Name;
    std::vector<std::string>  Args; 
public:
    PrototypeAST(const std::string & Name,std::vector<std::string> Args)
        :Name(Name),Args(std::move(Args)){}

    Function *codegen();
    
    const std::string &getName() const {return Name;}
};

class FunctionAST{
private:
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto,
        std::unique_ptr<ExprAST> Body)
        : Proto(std::move(Proto)),Body(std::move(Body)){}
    
    Function *codegen();
};

#endif