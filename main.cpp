#include <stdio.h>
#include "Lexer.h"

enum {
  _INT = 300,
  _FLOAT,
  _IF,
  _WHILE
};
  

LexerReserved res[] = {
  { "int", _INT },
  { "float", _FLOAT },
  { "if", _IF },
  { "while", _WHILE }
};

int main( int argc, 
	  char ** argv ) {

  LexerToken tok;
  Lexer lex( "testInput.txt", res, sizeof(res)/sizeof(LexerReserved) );
  
  while ( lex.nextToken( tok ) ) {
    
    switch( tok.type ) {
      
    case '+':
      printf( "_PLUS\n" );
      break;
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
      printf( "_WHILE\n");
      break;
    case _INT:
      printf( "_INT\n" );
      break;
    case _FLOAT:
      printf( "_FLOAT\n" );
      break;
    default: 
      printf( "Error in main switch\n" ); 
      break;

    }

  }


  return 0;
 

}

