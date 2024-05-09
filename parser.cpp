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
#include "log.h"


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
    auto V=ParseExpression();    // recursive function

    if(!V){
        return nullptr;
    }

    if(CurTok!=')'){
        return LogError("expected ')' ");
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
                return LogError("Expected ')' or ',' in argument list");
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
        return LogError("unknow token when excepting an expression");
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



/// @brief parse the pair [binop,primaryexpr]
/// @param ExprPrec : the min precedence which need eat
/// @param LHS : 
/// @return 
std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,std::unique_ptr<ExprAST> LHS){
    while(true){
        int TokPrec=GetTokPrecedence();

        // if TokPrec < ExprPrec , can not eat CurTok,just return previous LHS.
        // implicit infomation: if the binop is invalid ,the TokPrec result is -1,
        // so the binary expression is end, just return previous LHS
        if(TokPrec<ExprPrec){
            return LHS;
        }


        // else : the binop is valid
        int BinOp=CurTok;
        getNextToken(); //  eat binop

        // parse primary expression which is after the binop
        auto RHS=ParsePrimary();
        if(!RHS){
            return nullptr;
        }

        // look ahead at binop to determine
        // “(a+b) binop unparsed” or “a + (b binop unparsed)”.
        // if NextPrec>=TokPrec, the format is “a + (b binop unparsed)”, call  recursion
        int NextPrec = GetTokPrecedence();
        if(TokPrec < NextPrec){
            RHS=ParseBinOpRHS(TokPrec+1,std::move(RHS));
            if(!RHS){
                return nullptr;
            }
        }

        // merge LHS and RHS and make BinaryExprAST 
        LHS=std::make_unique<BinaryExprASTP> (BinOp,std::move(LHS),std::move(RHS));
    }
}
 


/// an expression format: primaryexpr [binop,primaryexpr] pairs
/// potentional recursive function
std::unique_ptr<ExprAST> ParseExpression(){
    auto LHS=ParsePrimary();
    if(!LHS){
        return nullptr;
    }

    return ParseBinOpRHS(0,std::move(LHS));
}




/*/////////////////////////////////////////////
*
*Parsing the Rest
*
*//////////////////////////////////////////////




/// parse prototype expression
/// NOTE: prototype expression format ->   function_name(a  b  c ... )
/// so the list of arguments is not seperated by ','
std::unique_ptr<PrototypeAST> ParsePrototype(){
    if(CurTok!=tok_identifier){
        return LogErrorP("Expected function name in prototype");
    }

    std::string FnName = IdentifierStr;
    getNextToken(); // look ahead

    if(CurTok!='('){
        return LogErrorP("Expected '(' in prototye");
    }


    std::vector<std::string> ArgNames;
    // read the list of argument names
    while(getNextToken()==tok_identifier){
        ArgNames.push_back(IdentifierStr);
    }

    if(CurTok!=')'){
        return LogErrorP("Expected ')' in prototype");
    }

    getNextToken(); // eat ')'

    return std::make_unique<PrototypeAST>(FnName,std::move(ArgNames));
}



/// parse function definition expression
/// NOTE: function definition expression format is :
/// -> 'def' + 'prototype expression' + 'expression'
std::unique_ptr<FunctionAST> ParseDefinition(){
    getNextToken(); // eat 'def'
    auto Proto=ParsePrototype();
    if(!Proto){
        return nullptr;
    }

    if(auto E=ParseExpression()){
        return std::make_unique<FunctionAST>(std::move(Proto),std::move(E));
    }

    return nullptr;
}



/// parse 'extern'
std::unique_ptr<PrototypeAST> ParseExtern(){
    getNextToken(); // eat 'extern'
    return ParsePrototype();
}

/// parse top-level expression
/// top-level expression 
std::unique_ptr<FunctionAST> ParseTopLevelExpr(){
    if(auto E=ParseExpression()){

        // make an anonymous proto
        auto Proto=std::make_unique<PrototypeAST>("",std::vector<std::string>());
        return std::make_unique<FunctionAST>(std::move(Proto),std::move(E));
    }
    return nullptr;
}