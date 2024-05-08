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
#include "lexer.h"
#include "ast.h"
#include "helper.h"


/// NOTE: core measures:
/// Recursive Descent Parsing 
/// and Operator-Precedence Parsing



/// NOTE: because include "lexer.h", 
/// so the source code has two global varibales
/// std::string IdentifierStr;       double NumVal;


int CurTok;
int getNextToken(){
    return CurTok=gettok();
}


std::unique_ptr<ExprAST> ParseExpression();  // the significant function



/*////////////////////////////////
*
*    Basic Expression Parsing
*
*/////////////////////////////////



/// if Curtoke is tok_number , call this function
/// parse number
std::unique_ptr<ExprAST> ParseNumberExpr(){
    auto Result=std::make_unique<NumberExprAST>(NumVal);
    getNextToken();
    return std::move(Result);
}



/// if Curtoke is '(' , call this function 
/// parse the expression "(exp)" 
/// call ParseExpression()
std::unique_ptr<ExprAST> ParseParenExpr(){
    getNextToken();     // eat '('
    auto V=ParseExpression();

    if(!V){
        return nullptr;
    }

    if(CurTok!=')'){
        return LogErrorP("expected ')' ");
    }

    getNextToken();     // eat ')'
    return V;
}



/// if Curtoke is tok_identifier, call this function
/// parse the expression IdentifierExpr
/// handling variables references and function calls
/// if exp is function name,call ParseExpression()
std::unique_ptr<ExprAST> ParseIdentifierExpr(){
    std::string IdName= IdentifierStr;

    getNextToken();     // eat identifier

    // if variables reference
    if(CurTok!='('){   // look ahead to determine
        return std::make_unique<VariableExprAST>(IdName);
    }

    // else function calls
    getNextToken();  // eat '('

    std::vector<std::unique_ptr<ExprAST>> Args;

    if(CurTok != ')'){
        while(true){
            if(auto Arg=ParseExpression()){
                Args.push_back(std::move(Arg));
            }else{
                return nullptr;
            }

            if(CurTok == ')'){
                break;
            }

            if(CurTok != ','){
                return LogErrorP("Expected ')' or ',' in argument list");
            }
            getNextToken(); 
        }
    }

    getNextToken(); // eat ')'

    return std::make_unique<CallExprAST>(IdName,std::move(Args));
}




/// helper function  
/// provide one entry point
std::unique_ptr<ExprAST> ParsePrimary(){
    switch(CurTok){
    default:
        return LogErrorP("unknow token when excepting an expression");
    case tok_identifier:
        return ParseIdentifierExpr();
    case tok_number:
        return ParseNumberExpr();  
    case '(':
        return ParseParenExpr();
    // more case ...
    }
        
}



/*////////////////////////////////////
*
*   Binary Expression Parsing
*
*/////////////////////////////////////


// store precedence of each binary operator which defined
std::map<char,int> BinopPrecedence;


/// get precedence of CurTok
int GetTokPrecedence(){
    if(!isascii(CurTok)){
        return -1;
    }

    // make sure it is defined 
    int TokPrec=BinopPrecedence[CurTok];
    if(TokPrec<=0){
        return -1;
    }
    return TokPrec;
}


std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,std::unique_ptr<ExprAST> LHS){
    int TokPrec=GetTokPrecedence();

    if(TokPrec<ExprPrec){
        return LHS;
    }
}
 


/// [binop,primaryexpr]
/// potentional recursive function
std::unique_ptr<ExprAST> ParseExpression(){
    auto LHS=ParsePrimary();
    if(!LHS){
        return nullptr;
    }

    return ParseBinOpRHS(0,std::move(LHS));
}