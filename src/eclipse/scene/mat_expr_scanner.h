#pragma once

#include "eclipse/util/except.h"

#include <istream>

#undef yyFlexLexer
#include <FlexLexer.h>
#include "eclipse/scene/mat_expr_parser.hxx"

// Tell flex which function to define
#ifdef YY_DECL
#undef YY_DECL
#endif
#define YY_DECL                                                 \
    int eclipse::material::MatExprScanner::lex(                        \
            eclipse::material::MatExprParser::semantic_type* yylval,   \
            eclipse::material::MatExprParser::location_type* yylloc)

namespace eclipse { namespace material {

// To feed data back to bison, the yylex method needs yylval and
// yylloc parameters. Since the yyFlexLexer class is defined in the
// ssytem header <FlexLexer.h> the signature of its yylex() method
// cannot be changed anymore. This makes it necessary to derive a
// scanner class that provides a method with the desired signature:

class MatExprScanner : public yyFlexLexer
{
public:
    explicit MatExprScanner(std::istream* in = 0, std::ostream* out = 0);

    int lex(MatExprParser::semantic_type* yylval, MatExprParser::location_type* yylloc);
};

class MatExprSyntaxError : public Error
{
public:
    MatExprSyntaxError(const MatExprParser::location_type& l, const std::string& m)
        : Error(m), location(l)
    {
    }

    MatExprParser::location_type location;
};

} } // namespace eclipse::material
