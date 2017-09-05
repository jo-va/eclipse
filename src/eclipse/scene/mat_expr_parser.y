%skeleton "lalr1.cc"
%require  "3.0"

%define api.namespace {eclipse::material}
%define parser_class_name {MatExprParser}
%parse-param { eclipse::material::MatExprScanner* scanner } { eclipse::material::ParserCallback* cb }

%locations

%define parse.error verbose
%define parse.assert

%define api.value.type variant
%define api.token.prefix {T_}

%code requires
{
    #include <string>

    #include "eclipse/scene/mat_expr.h"
    #include "eclipse/scene/location.hh"

    namespace eclipse { namespace material {
        class MatExprScanner;

        struct ParserCallback {
            virtual void set(ExprNode* expression) = 0;
        };
    } }
}

%code
{
    #include <utility>
    #include <memory>

    #include "eclipse/scene/mat_expr_scanner.h"

    #ifdef yylex
    #undef yylex
    #endif
    #define yylex scanner->lex

    std::string unquote(const std::string& s)
    {
        return s.substr(1, s.length() - 2);
    }
}

%token                      END 0 "end of file"
%token <BxdfType>           BXDF_TYPE
%token <std::string>        TEX MAT
%token <float>              NUM
%token                      MIX MMAP BMAP NMAP DISP
%token                      REFLECTANCE SPECULARITY TRANSMITTANCE RADIANCE IIOR EIOR SCALER ROUGHNESS

%type <ExprNode*>           bxdf bxdf_op bxdf_any
%type <NBxdfParamList>      param_list
%type <NBxdfParam>          param
%type <ParamValue>          f_or_mat f_or_tex f3_or_tex f3

%start mat_def

%%

mat_def     : bxdf                          { cb->set($1); }
            | bxdf_op                       { cb->set($1); }
            ;

bxdf        : BXDF_TYPE '(' param_list ')'  { $$ = new NBxdf($1, std::move($3)); }
            ;

param_list  : %empty                        { $$ = { }; }
            | param                         { $$ = { $1 }; }
            | param_list ',' param          { $1.push_back(std::move($3)); $$ = std::move($1); }
            ;

param       : REFLECTANCE   ':' f3_or_tex   { $$ = { REFLECTANCE, $3 }; }
            | SPECULARITY   ':' f3_or_tex   { $$ = { SPECULARITY, $3 }; }
            | TRANSMITTANCE ':' f3_or_tex   { $$ = { TRANSMITTANCE, $3 }; }
            | RADIANCE      ':' f3_or_tex   { $$ = { RADIANCE, $3 }; }
            | IIOR          ':' f_or_mat    { $$ = { INT_IOR, $3 }; }
            | EIOR          ':' f_or_mat    { $$ = { EXT_IOR, $3 }; }
            | SCALER        ':' NUM         { $$ = { SCALER, ParamValue::num($3) }; }
            | ROUGHNESS     ':' f_or_tex    { $$ = { ROUGHNESS, $3 }; }
            ;

f_or_mat    : NUM                           { $$ = ParamValue::num($1); }
            | MAT                           { $$ = ParamValue::known_ior(unquote($1)); }
            ;

f_or_tex    : NUM                           { $$ = ParamValue::num($1); }
            | TEX                           { $$ = ParamValue::texture(unquote($1)); }
            ;

f3_or_tex   : f3                            { $$ = $1; }
            | TEX                           { $$ = ParamValue::texture(unquote($1)); }
            ;

f3          : '{' NUM ',' NUM ',' NUM '}'   { $$ = ParamValue::vec3($2, $4, $6); }
            ;

bxdf_op     : MIX  '(' bxdf_any ',' bxdf_any ',' NUM ')' { $$ = new NMix($3, $5, $7); }
            | MMAP '(' bxdf_any ',' bxdf_any ',' TEX ')' { $$ = new NMixMap($3, $5, ParamValue::texture(unquote($7))); }
            | BMAP '(' bxdf_any ',' TEX ')'              { $$ = new NBumpMap($3, ParamValue::texture(unquote($5))); }
            | NMAP '(' bxdf_any ',' TEX ')'              { $$ = new NNormalMap($3, ParamValue::texture(unquote($5))); }
            | DISP '(' bxdf_any ',' IIOR ':' f3 ',' EIOR ':' f3 ')' { $$ = new NDisperse($3, $7, $11); }
            ;

bxdf_any    : bxdf                          { $$ = $1; }
            | bxdf_op                       { $$ = $1; }
            | MAT                           { $$ = new NMatRef(unquote($1)); }
            ;
%%

void eclipse::material::MatExprParser::error(const eclipse::material::MatExprParser::location_type& l, const std::string& m)
{
    throw eclipse::material::MatExprSyntaxError(l, m);
}
