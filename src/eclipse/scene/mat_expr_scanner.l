%option c++
%option 8bit warn
%option noyywrap
%option yyclass="MatExprScanner"

%{
    #include <stdexcept>
    #include <string>
    #include <cstdlib> // atof

    #include "eclipse/scene/mat_expr_parser.hxx"
    #include "eclipse/scene/mat_expr_scanner.h"
    #include "eclipse/scene/mat_expr.h"

    #define YIELD_TOKEN(tok) return eclipse::material::MatExprParser::token::T_##tok;

    #define YY_USER_ACTION yylloc->columns(yyleng);
%}

num      [+-]?(([0-9]*\.?[0-9]+)|([0-9]+\.))
texture  \"[a-zA-Z_./][a-zA-Z0-9\-_./]*\.[a-zA-Z]+\"
mat_name \"[a-zA-Z_][a-zA-Z0-9_]*\"

%%

%{
    yylloc->step();
%}

[ \t]               { ; }
{num}               { yylval->build<float>(std::atof(yytext)); YIELD_TOKEN(NUM); }
{texture}           { yylval->build<std::string>(std::string(yytext, yyleng)); YIELD_TOKEN(TEX); }
{mat_name}          { yylval->build<std::string>(std::string(yytext, yyleng)); YIELD_TOKEN(MAT); }
"diffuse"           { yylval->build<NodeType>(BXDF_DIFFUSE);          YIELD_TOKEN(BXDF_TYPE); }
"conductor"         { yylval->build<NodeType>(BXDF_CONDUCTOR);        YIELD_TOKEN(BXDF_TYPE); }
"roughConductor"    { yylval->build<NodeType>(BXDF_ROUGH_CONDUCTOR);  YIELD_TOKEN(BXDF_TYPE); }
"dielectric"        { yylval->build<NodeType>(BXDF_DIELECTRIC);       YIELD_TOKEN(BXDF_TYPE); }
"roughDielectric"   { yylval->build<NodeType>(BXDF_ROUGH_DIELECTRIC); YIELD_TOKEN(BXDF_TYPE); }
"emissive"          { yylval->build<NodeType>(BXDF_EMISSIVE);         YIELD_TOKEN(BXDF_TYPE); }
"mix"               { YIELD_TOKEN(MIX); }
"mixMap"            { YIELD_TOKEN(MMAP); }
"bumpMap"           { YIELD_TOKEN(BMAP); }
"normalMap"         { YIELD_TOKEN(NMAP); }
"disperse"          { YIELD_TOKEN(DISP); }
"reflectance"       { YIELD_TOKEN(REFLECTANCE); }
"specularity"       { YIELD_TOKEN(SPECULARITY); }
"transmittance"     { YIELD_TOKEN(TRANSMITTANCE); }
"radiance"          { YIELD_TOKEN(RADIANCE); }
"intIOR"            { YIELD_TOKEN(IIOR); }
"extIOR"            { YIELD_TOKEN(EIOR); }
"scale"             { YIELD_TOKEN(SCALER); }
"roughness"         { YIELD_TOKEN(ROUGHNESS); }
.                   { return yytext[0]; }

%%

eclipse::material::MatExprScanner::MatExprScanner(std::istream* in, std::ostream* out)
    : yyFlexLexer(in, out)
{
}
