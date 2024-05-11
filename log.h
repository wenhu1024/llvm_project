#ifndef HELPER_H
#define HELPER_H

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ast.h"

using namespace llvm;

inline std::unique_ptr<ExprAST> LogError(const char* Str){
    fprintf(stderr,"Error: %s\n",Str);
    return nullptr;
}

inline std::unique_ptr<PrototypeAST> LogErrorP(const char* Str){
    LogError(Str);
    return nullptr;
}

#endif