#include <stdio.h>
#include "Lexer.h"

enum {
  _INT = 300,
  _FLOAT,
  _IF,
  _WHILE
};


LexerKeyword res[] = {
  { "int", _INT },
  { "float", _FLOAT },
  { "if", _IF },
  { "while", _WHILE }
};

int main( int argc, char ** argv ) {

  int t;
  LexerToken tok;
  Lexer lex( "testInput.txt", res, sizeof(res)/sizeof(LexerKeyword) );

  while ( t = lex.nextToken( tok ) ) {

    switch( t ) {

    case LEXER_REAL: 
      printf( "%f\n", tok.f ); 
      break;
    case LEXER_ID:
      printf( "%s\n", tok.s );
      break;
    case _IF:
      printf( "_IF\n" );
      if( lex.match('(') ) printf("(\n");
      if( lex.match(')') ) printf(")\n");
      break;
    case _WHILE:
      printf( "while\n");
      if( lex.match('(') ) printf("(\n");
      if( lex.match(')') ) printf(")\n");
      if( lex.match('{') ) printf("{\n");
      if( lex.match('}') ) printf("}\n");
      break;
    case _INT:
      printf( "int\n" );
      break;
    case '=':
      printf( "=\n" );
      break;
    case ';':
      printf( ";\n" );
      break;


    default: 
      printf( "Error in main switch\n" ); 
      break;

    }

  }

  char c;
  scanf("%c", &c );

  return 0;


}

