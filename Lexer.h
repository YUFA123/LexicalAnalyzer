/*
Copyright (C) 2012 Andrew Montgomery. 

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __LEXER_H__
#define __LEXER_H__

#define MAX_ID_LEN 255
#define MIN_TOK_VAL 300

// Token Values for generic constructs,
// All Reserved words must not conflict with these
enum {
  LEXER_ID       = 256,  // Identifier / variable
  LEXER_REAL     = 257,  // Constant
  LEXER_LE       = 258,  // <=
  LEXER_GE       = 259,  // >=
  LEXER_EQEQ     = 260,  // ==
  LEXER_SL       = 261,  // <<
  LEXER_SR       = 262,  // >>
  LEXER_NE       = 263   // !=
};

//==============================================================================
// Token
//==============================================================================
struct LexerToken {
  int i;
  float f;
  char s[MAX_ID_LEN];
};

//==============================================================================
// Reserved Words Struct
//==============================================================================
struct LexerKeyword {
  char * str;
  int type;
};

//==============================================================================
// Lexer
//==============================================================================
class Lexer {
public:

  Lexer( const char * fileName, LexerKeyword * r, int rLen );
  ~Lexer();

  int nextToken( LexerToken & t );
  bool match( int t );

private:

  // Members
  LexerKeyword *  keywords;        // Keywords
  unsigned int    keywordsLen;
  char *          punct;           // Punctuation array, defined in constructor
  unsigned int    punctLen;
  char *          fileBuf;         // The only pointer we work with
  char *          fileBufBegin;    // Points to beginning of fileBuf
  unsigned int    lineNo;

  // Methods
  int readWhiteSpace( void );
  int readNumeric( LexerToken & t );
  int readIdentifier( LexerToken & t );
  int readPunctuation( void );

  void sortKeywords( void );
  LexerKeyword * isKeyword( char * s ) const;

  bool isNum( const int c ) const;
  bool isAlpha( const int c ) const;
  bool isAlphaNum( const int c ) const;
  bool isWhiteSpace( const int c ) const;

  enum LEX_WERROR {
    WARNING_MESSAGE,
    ERROR_MESSAGE,
    IDENT_NAME_TOO_LONG,
    CONSTANT_TOO_LONG,
    UNKNOWN_INPUT,
    MIN_TOK_VAL_ERROR
  };

  // Warning/Error routine
  void lexerWError( const LEX_WERROR type, const char * msg = 0, ... ); 

};


//==============================================================================
// Overloading <ctype> functionality
//==============================================================================
inline bool Lexer::isNum( const int c ) const {
  return c >= '0' && c <= '9';
}
inline bool Lexer::isAlpha( const int c ) const {
  return ( ( c >= 'a' && c <= 'z' ) ||
    ( c >= 'A' && c <= 'Z' ) );
}
inline bool Lexer::isAlphaNum( const int c ) const {
  return ( isNum( c ) || isAlpha( c ) || c == '_' );
}
inline bool Lexer::isWhiteSpace( const int c ) const {
  return ( c == ' ' || c == '\t' ||
    c == '\n' || c == '\r' );
}

#endif
// Local Variables:
// mode: c++
// End:
