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
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "ast.h"
#include "codegen.h"
#include "parser.h"
#include "lexer.h"
#include "log.h"

using namespace llvm;


/*/////////////////////////////////////
 *
 *   Top-level Parse and JIT Driver
 *
 */
////////////////////////////////////
void InitializeModule(){
    //  open a new context and module
    TheContext=std::make_unique<LLVMContext>();
    TheModule=std::make_unique<Module>("my cool jit",*TheContext);

    //  create a new builder for the module
    Builder=std::make_unique<IRBuilder<>>(*TheContext);
}


void HandleDefinition()
{
    if (auto FnAST=ParseDefinition())
    {
        if(auto *FnIR=FnAST->codegen()){
            fprintf(stderr, "Read a function definition:");
            FnIR->print(errs());
            fprintf(stderr,"\n");
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
    if (auto FnAST=ParseTopLevelExpr())
    {
        if(auto *FnIR=FnAST->codegen()){
            fprintf(stderr, "Read top-level expression:");
            FnIR->print(errs());
            fprintf(stderr,"\n");

            //  remove the annoymous expression
            FnIR->eraseFromParent();
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
    // Install standard binary operators.
    // 1 is lowest precedence.
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40; // highest.

    // Prime the first token.
    fprintf(stderr, "ready> ");
    getNextToken();


    // Make the module, which holds all the code.
    InitializeModule();

    // Run the main "interpreter loop" now.
    MainLoop();

    // Print out all of the generated code.
    TheModule->print(errs(), nullptr);

    return 0;
}