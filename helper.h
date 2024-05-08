#ifndef HELPER_H
#define HELPER_H

#include <memory>
#include "ast.h"

inline std::unique_ptr<ExprAST> LogError(const char* Str){
    fprintf(stderr,"Error: %s\n",Str);
    return nullptr;
}

inline std::unique_ptr<ExprAST> LogErrorP(const char* Str){
    LogError(Str);
    return nullptr;
}

#endif