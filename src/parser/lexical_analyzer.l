%option noyywrap
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int lines;
int pos_start;
int pos_end;

void pass_node(char *text){
     yylval.node = new_syntax_tree_node(text);
}

/*****************声明和选项设置  end*****************/

%}


%%
 /* to do for students */
 /* two cases for you, pass_node will send flex's token to bison */
 /****请在此补全所有flex的模式与动作  end******/
\+ {pos_start = pos_end; pos_end += 1; pass_node(yytext); return ADD;}
\- {pos_start = pos_end; pos_end += 1; pass_node(yytext); return SUB;}
\* {pos_start = pos_end; pos_end += 1; pass_node(yytext); return MUL;}
\% {pos_start = pos_end; pos_end += 1; pass_node(yytext); return MOD;}
\/ {pos_start = pos_end; pos_end += 1; pass_node(yytext); return DIV;}
\< {pos_start = pos_end; pos_end += 1; pass_node(yytext); return LT;}
\<= {pos_start = pos_end; pos_end += 2; pass_node(yytext); return LE;}
\> {pos_start = pos_end; pos_end += 1; pass_node(yytext); return GT;}
\>= {pos_start = pos_end; pos_end += 2; pass_node(yytext); return GE;}
\== {pos_start = pos_end; pos_end += 2; pass_node(yytext); return EQ;}
\!= {pos_start = pos_end; pos_end += 2; pass_node(yytext); return NEQ;}
\= {pos_start = pos_end; pos_end += 1; pass_node(yytext); return ASSIGN;}

\; {pos_start = pos_end; pos_end += 1; pass_node(yytext); return SEMICOLON;}
\, {pos_start = pos_end; pos_end += 1; pass_node(yytext); return COMMA;}
\( {pos_start = pos_end; pos_end += 1; pass_node(yytext); return LPARENTHESIS;}
\) {pos_start = pos_end; pos_end += 1; pass_node(yytext); return RPARENTHESIS;}
\[ {pos_start = pos_end; pos_end += 1; pass_node(yytext); return LBRACKET;}
\] {pos_start = pos_end; pos_end += 1; pass_node(yytext); return RBRACKET;}
\{ {pos_start = pos_end; pos_end += 1; pass_node(yytext); return LBRACE;}
\} {pos_start = pos_end; pos_end += 1; pass_node(yytext); return RBRACE;}
int {pos_start = pos_end; pos_end += 3; pass_node(yytext); return INT;}
float {pos_start = pos_end; pos_end += 5; pass_node(yytext); return FLOAT;}
return {pos_start = pos_end; pos_end += 6; pass_node(yytext); return RETURN;}
void {pos_start = pos_end; pos_end += 4; pass_node(yytext); return VOID;}
while {pos_start = pos_end; pos_end += 5; pass_node(yytext); return WHILE;}
if {pos_start = pos_end; pos_end += 2; pass_node(yytext); return IF;}
else {pos_start = pos_end; pos_end += 4; pass_node(yytext); return ELSE;}
break {pos_start = pos_end; pos_end += 5; pass_node(yytext); return BREAK;}
const {pos_start = pos_end; pos_end += 5; pass_node(yytext); return CONST;}
continue {pos_start = pos_end; pos_end += 8; pass_node(yytext); return CONTINUE;}
&& {pos_start = pos_end; pos_end += 2; pass_node(yytext); return AND;}
|| {pos_start = pos_end; pos_end += 2; pass_node(yytext); return OR;}
! {pos_start = pos_end; pos_end += 1; pass_node(yytext); return NOT;}

[_a-zA-Z][_a-zA-Z0-9]* {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return ID;}
([0-9]+\.[0-9]*|[0-9]*\.[0-9]+)(e|E[-+]?[0-9]+)? {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return FLOATPOINT_10;}
(0x|0X)([0-9a-fA-F]*\.[0-9a-fA-F]+|[0-9a-fA-F]+\.)((p|P)[-+]?[0-9a-fA-F]+)? {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return FLOATPOINT_16;}

0|[1-9][0-9]* {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return INTEGER_10;}
0[0-7]* {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return INTEGER_8;}
(0x|0X)[0-9a-fA-F]+ {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return INTEGER_16;}

(\r)?\n {pos_start = 0; pos_end = 1; lines += 1;}
[ \t\r] {pos_end += 1;}

 "//"[^\n\r]*[\n\r] {pos_start = 0; pos_end = 1; lines += 1;}

\/\*([^*]|(\*+[^*/]))*\*+\/ {
     for(int i = 0; i < strlen(yytext); ++i){
          if(yytext[i] == '\n'){
               lines ++;
               pos_start = 0;
               pos_end = 1;
          }else{
               pos_end += 1;
          }
     }
}

. { pos_start = pos_end; pos_end++; return ERROR; }

%%
