%option yylineno
%pointer

%{
    #include <cstdlib>
    #include <cstring>
    #include "ast.hpp"
    #include "parser.hpp"

    /* void yyerror(const char *); */
%}

/** WRITE ME:
 *  Put any definitions you need here. Add your own regular expressions
 *  below for ALL tokens defined in the manual. You can choose not to use
 *  definitions here.
 */

/* HERE */
DECIMAL 0|([1-9][0-9]*)
HEX     ["0x""0X"][[0-9][A-F]]+
OCTAL   0[0-7]+
BINARY  [01]+b

/** The example below is very simple, but it shows almost everything you will
 *  need to do for this first part. The syntax is as follows:
 *
 *  regular_expression      {  c code  }
 *
 * The lexer will try to match each regular expression. When it does it will
 * execute the corresponding C code.
*/

%%

 /* WRITEME: Put your code from Project 3 here */
boolean     {return T_BOOL;}
char        {return T_CHAR;}
integer     {return T_INT;}
string      {return T_STRING;}
intptr      {return T_INTP;}
charptr     {return T_CHARP;}

if          {return IF;}
else        {return ELSE;}
while       {return WHILE;}


null        {return N;}
var         {return VAR;}
procedure   {return PROC;}
return      {return RET;}

";"         |
"|"         |
"{"         |
"}"         |
")"         |
"("         |
"["         |
"]"         |
"/"         |
"<"         |
">"         |
"-"         |
"*"         |
"^"         |
"+"         |
"!"         |
"="         |
"&"         |
","         |
":"         {return *yytext;}

           
"&&"        {return AND;}
"=="        {return EQ;}
">="        {return GEQ;}
"<="        {return LEQ;}
"!="        {return NEQ;}
"||"        {return OR;}


true        {yylval.u_base_int = 1; return V_BOOL;}
false       {yylval.u_base_int = 0; return V_BOOL;}

"'"[\40-\176]"'"   { int x = yytext[1];
                     yylval.u_base_int=x;
                     return V_CHAR; }

{DECIMAL} {yylval.u_base_int = (int)strtol(yytext, 0, 10); //Convert to Integer
           return V_INTEGER;}

{HEX} {yylval.u_base_int = (int)strtol(yytext, 0, 16); //Convert to Integer
           return V_INTEGER;}

{OCTAL} {yylval.u_base_int = (int)strtol(yytext, 0, 8); //Convert to Integer
           return V_INTEGER;}

{BINARY} {yylval.u_base_int = (int)strtol(yytext, 0, 2); //Convert to Integer
           return V_INTEGER;}

\"[^\"]*\"                       {  int len = strlen(yytext)-2;
                                    yytext = yytext + 1;
                                    yylval.u_base_charptr = strndup(yytext, len);
                                    return V_STRING;
                                 }

[a-zA-Z][a-zA-Z0-9_]*            {
                                 yylval.u_base_charptr = strdup(yytext);
                                 return V_IDENTIFIER;
                                /*Identifier denoted by v*/}

\/%                             {/*Delete comments*/
    //Credit to fish for brilliance
    int c;
    while(true){
        while((c = yyinput()) != '%' && c != EOF);
        if((c = yyinput()) == '/'){
            if(YYDEBUG) yyerror("End of Comment Found\n");
            break;
        }
        else if(c == EOF){
            yyerror("Unexpected EOF");
            exit(1);
        }
    } 
                                }

[ \t\n]                         ; /* skip whitespace */

.                   yyerror((char *) "invalid character");

%%

/** You shall not pass!
 *  You should not  have to do or edit anything past this.
 */

int yywrap(void) {
    return 1;
}
