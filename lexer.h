#ifndef LEXER_H
#define LEXER_H

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

enum Token{
    tok_eof=-1,

    tok_def=-2,
    tok_extern =-3,

    tok_identifier=-4,
    tok_number=-5,

    tok_if=-6,
    tok_then=-7,
    tok_else=-8,
    tok_for=-9,
    tok_in=-10,

    tok_binary=-11,
    tok_unary=-12,

    tok_var=-13
};

int gettok();
extern std::string IdentifierStr;
extern double NumVal;

#endif