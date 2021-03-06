%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "scanner.h"
    #include "tabla_simbolos.h"

    //#define YYSTYPE int

    void yyerror(const char *s);
%}

%defines "parser.h"
%output "parser.c"

%union {
    char *cadena;
    int numero;
}

%token FDT INICIO FIN
%token <cadena> ID
%token ASIGNACION PUNTOYCOMA PARENIZQUIERDO PARENDERECHO COMA
%token SUMA RESTA
%token LEER ESCRIBIR
%token <numero> CONSTANTE

%left SUMA RESTA
%right ASIGNACION

%type <numero> expresion primaria operadorAditivo

%start objetivo

%%

// <objetivo>  ->  <programa>  FDT
objetivo:
    programa FDT                                                            { imprimirTablaDeSimbolos(); exit(EXIT_SUCCESS);}
    ;

// <programa>  ->  INICIO  <listaSentencias>  FIN
programa:
    INICIO listaSentencias FIN
    ;

// <listaSentencias>  ->  <sentencia>  {<sentencia>}
listaSentencias:
    sentencia 
    | listaSentencias sentencia
    ;

// <sentencia>  ->  ID  ASIGNACIÓN  <expresión>  PUNTOYCOMA  |
//      LEER  PARENIZQUIERDO  <listaIdentificadores>  PARENDERECHO  PUNTOYCOMA  |
//      ESCRIBIR  PARENIZQUIERDO  <listaExpresiones>  PARENDERECHO  PUNTOYCOMA
sentencia:
    ID ASIGNACION expresion PUNTOYCOMA                                      { guardarValorEnTabla($1, $3); }
    | LEER PARENIZQUIERDO listaIdentificadores PARENDERECHO PUNTOYCOMA      
    | ESCRIBIR PARENIZQUIERDO listaExpresiones PARENDERECHO PUNTOYCOMA      
    ;

// <listaIdentificadores>  ->  ID  {COMA  ID}
listaIdentificadores:
    ID                                                                      { cargarEntradaEnTabla($1); }
    | listaIdentificadores COMA ID                                          { cargarEntradaEnTabla($3); }
    ;

// <listaExpresiones>  ->  <expresión>  {COMA  <expresión>}
listaExpresiones:
    expresion                                                               { printf("Resultado expresion: %d\n", $1); }
    | listaExpresiones COMA expresion                                       { printf("Resultado expresion: %d\n", $3); }
    ;

// <expresión>  ->  <primaria>  {<operadorAditivo>  <primaria>}
expresion:
    primaria                                                                { $$ = $1; }
    | expresion operadorAditivo primaria                                    { $$ = $2 == '+' ? $1 + $3 : $1 - $3; }

// <primaria>  ->  ID  |  CONSTANTE  | PARENIZQUIERDO  <expresión>  PARENDERECHO
primaria:
    ID                                                                      { $$ = leerValorSimbolo($1); }
    | CONSTANTE                                                             { $$ = $1; }
    | PARENIZQUIERDO expresion PARENDERECHO                                 { $$ = $2; }
    ;

// <operadorAditivo>  ->  SUMA | RESTA
operadorAditivo:
    SUMA                                                                    { $$ = '+'; }
    | RESTA                                                                 { $$ = '-'; }
    ;

%%


void yyerror(const char *msg) {
    printf("Error en la expresion. %s\n", msg);
}
