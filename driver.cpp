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
// #include "codegen.h"
#include "parser.h"
#include "lexer.h"

/*/////////////////////////////////////
 *
 *   Top-level Parse
 *
 */
////////////////////////////////////

void HandleDefinition()
{
    if (ParseDefinition())
    {
        fprintf(stderr, "Parsed a function definition.\n");
    }
    else
    {
        // Skip token for error recovery.
        getNextToken();
    }
}

void HandleExtern()
{
    if (ParseExtern())
    {
        fprintf(stderr, "Parsed an extern.\n");
    }
    else
    {
        // Skip token for error recovery.
        getNextToken();
    }
}

void HandleTopLevelExpression()
{
    if (ParseTopLevelExpr())
    {
        fprintf(stderr, "Parsed a top-level expr.\n");
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

    // Run the main "interpreter loop" now.
    MainLoop();

    return 0;
}