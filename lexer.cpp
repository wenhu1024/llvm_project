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
#include "lexer.h"


using namespace llvm;

std::string IdentifierStr;
double NumVal;

int gettok()
{
    static int LastChar = ' ';

    while (isspace(LastChar))
    {
        LastChar = getchar();
    }

    if (isalpha(LastChar))
    {
        IdentifierStr = LastChar;

        while (isalnum((LastChar = getchar())))
        {
            IdentifierStr += LastChar;
        }

        if (IdentifierStr == "def")
            return tok_def;
        if (IdentifierStr == "extern")
            return tok_extern;
        if (IdentifierStr == "if")
            return tok_if;
        if (IdentifierStr == "then")
            return tok_then;
        if (IdentifierStr == "else")
            return tok_else;
        if (IdentifierStr == "for")
            return tok_for;
        if (IdentifierStr == "in")
            return tok_in;
        if (IdentifierStr == "binary")
            return tok_binary;
        if (IdentifierStr == "unary")
            return tok_unary;
        if (IdentifierStr == "var")
            return tok_var;


            
        return tok_identifier;
    }

    if(isdigit(LastChar) || LastChar == '.'){
        std::string NumStr;
        do{
            NumStr+=LastChar;
            LastChar=getchar();
        }while(isdigit(LastChar) || LastChar=='.');

        NumVal=strtod(NumStr.c_str(),nullptr);
        return tok_number;
    }

    if(LastChar=='#'){

        do{
            LastChar=getchar();
        }while(LastChar!=EOF && LastChar !='\n' && LastChar !='\r');

        if(LastChar!=EOF){
            return gettok();
        }
    }

    if(LastChar==EOF){
        return tok_eof;
    }

    int ThisChar=LastChar;
    LastChar=getchar();
    return ThisChar;
}