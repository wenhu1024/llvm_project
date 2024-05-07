#ifndef LEXER_H
#define LEXER_H

#include <string>

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