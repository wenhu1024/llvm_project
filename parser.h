#ifndef PARSER_H
#define PARSER_H


#include <map>
#include <memory>
#include <ast.h>

extern int CurTok;
extern std::map<char,int> BinopPrecedence;

int getNextToken();

std::unique_ptr<FunctionAST> ParseDefinition();
std::unique_ptr<FunctionAST> ParseTopLevelExpr();
std::unique_ptr<PrototypeAST> ParseExtern();

#endif