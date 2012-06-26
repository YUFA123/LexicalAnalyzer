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

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include "Lexer.h"

//==============================================================================
// Constructor
// Allocate and read in file contents, set buffer pointers
// Assign reserved word value, sort reserved words
//==============================================================================
Lexer::Lexer( const char * fileName, LexerKeyword * r, int rLen ) {

  FILE * f;
  size_t fileSize = 0;

  // Open file
  f = fopen( fileName, "r" );
  if ( f == 0 ) {
    lexerWError( ERROR_MESSAGE, "File Unable to open %s", fileName );
  }

  // Get file size
  fseek( f, 0L, SEEK_END );
  fileSize = ftell( f );
  rewind( f );

  // Allocate file buffer
  if( ( fileBuf = new char[fileSize+1] ) == 0 ) {
    lexerWError( ERROR_MESSAGE, 
      "Unable to allocate memory in Lexer contructor" );
  }
  fileBufBegin = fileBuf;

  // Read in entire file
  if ( ( fread( fileBuf, 1, fileSize, f ) != fileSize ) ) { // Didn't get the whole file
    lexerWError( ERROR_MESSAGE, "Error Reading in file %s", fileName );
  }

  // Null Terminate
  fileBuf[fileSize] = 0;

  fclose( f );

  // Assign reserved word list
  lineNo = 1;
  keywords = r;
  keywordsLen = rLen;

  // Assert all reserved word type ints are greater than 300
  for( unsigned int i = 0; i < keywordsLen; i++ ) {
    if( keywords[i].type < MIN_TOK_VAL ) 
      lexerWError( MIN_TOK_VAL_ERROR );
  }

  sortKeywords();
  punct = "+-/*()[]{}!@#$%^&<>=|:;?~.,'`";
  punctLen = strlen( punct );

}

//==============================================================================
// Deconstructor
//==============================================================================
Lexer::~Lexer() {

  if ( fileBufBegin ) delete [] fileBufBegin;

}

//==============================================================================
// Main Function
// Return true if token retrieved, false if not
//==============================================================================
int Lexer::nextToken( LexerToken & t ) {

  int c;  // peek ahead one char
  int r;  // result of readPunctuation

  // Kill whitespace
  if( !readWhiteSpace() ) return false;

  c = *fileBuf; // First non-whitespace character

  // Check for numeric constant
  if( isNum( c ) ) return readNumeric( t );

  // Identifiers or reserved words
  if( isAlpha( c ) ) return readIdentifier( t );

  // All punctuation
  if( r = readPunctuation( ) ) return r;

  // Should never reach here
  lexerWError( UNKNOWN_INPUT, 0 );
  return 0;
}

//==============================================================================
// match( int t )
// Determines if the next lexeme the token you provide
// If no match, return false, else return true
//==============================================================================
bool Lexer::match( int t ) {

  LexerToken tok;

  if( nextToken( tok ) == t ) return true;

  return false;

}

//==============================================================================
// Reads any punctuation
// Returns 1 if there was punctuation, otherwise return 0
//==============================================================================
int Lexer::readPunctuation( void ) {

  int p = 0; // First punctuation found

  // Get first punctuation symbol
  for( unsigned int i = 0; i < punctLen; i++ ) {

    if( *fileBuf == punct[i] ) {
      p = punct[i];
      fileBuf++;
      break;
    }

  }

  // Handle multi-punctuation symbols here
  // <=
  if( p == '<' && *fileBuf == '=' ) {
    fileBuf++;
    return LEXER_LE;
  }

  // >=
  if( p == '>' && *fileBuf == '=' ) {
    fileBuf++;
    return LEXER_GE;
  }

  // ==
  if( p == '=' && *fileBuf == '=' ) {
    fileBuf++;
    return LEXER_EQEQ;
  }

  // !=
  if( p == '!' && *fileBuf == '=' ) {
    fileBuf++;
    return LEXER_NE;
  }
  // <<
  if( p == '<' && *fileBuf == '<' ) {
    fileBuf++;
    return LEXER_SL;
  }

  // >>
  if( p == '>' && *fileBuf == '>' ) {
    fileBuf++;
    return LEXER_SR;
  }

  // There was at least one punct mark
  return p;

}

//==============================================================================
// Reads an identifier 
// Begins with a alpha character, From then on contains any 
//    alpha-numeric or underscore
//==============================================================================
int Lexer::readIdentifier( LexerToken & t ) {

  char id[MAX_ID_LEN];
  char * i = &id[0];
  LexerKeyword * r = 0;

  // Scoop up the identifier into the token
  do {
    *i++ = *fileBuf++;
  } while( isAlphaNum( *fileBuf ) );

  *i = '\0';

  // Check reserved words
  for( unsigned int k = 0; k < keywordsLen; k++ ) {
    if( ( r = isKeyword( id ) ) ) {
      return r->type;
    }
  }

  // Not a reserved word, an identifier
  strcpy( t.s, id );
  return LEXER_ID;

}

//==============================================================================
// Reads a constant
// Stores value in token returns 1 if successful, otherwise error
//==============================================================================
int Lexer::readNumeric( LexerToken & t ) {

  char num[MAX_ID_LEN];
  char * i = &num[0];

  // Before decimal
  while( *fileBuf >= '0' && *fileBuf <= '9' )
    *i++ = *fileBuf++;

  // Decimal, get it and all numerics afterwards
  if( *fileBuf == '.' ) {
    do {
      *i++ = *fileBuf++;
    } while ( *fileBuf >= '0' && *fileBuf <= '9' );
  }

  *i = '\0';

  t.f = (float)atof( num );
  return LEXER_REAL;

}

//==============================================================================
// Reads whitespace, increase lineNo
// Return 1 if some value follows or 0 if eof
// '#' == comment until newline
//==============================================================================
int Lexer::readWhiteSpace( ) {

  int c = *fileBuf;

  // Until first non-whitespace char
  while ( isWhiteSpace( c ) ) {
    if( c == '\n' ) ++lineNo;
    c = *++fileBuf;
    if( c == 0 ) return 0; // eof
  }

  // First non-whitespace is # denoting comment until new-line
  // Read until new line or eof, Then we must call readWhiteSpace again
  if( c == '#' ) {
    while( ( c = *++fileBuf ) ) {
      if( c == 0 ) return 0;
      if( c == '\n' ) break;
    } 
    ++lineNo;
    return readWhiteSpace();
  }

  return 1; // Still something left

}

//==============================================================================
// qsort comparison function
// Function is static, no pollution of namespace
//==============================================================================
static int qSortCmp( const void * a, const void * b ) {

  return strcmp( (char*)(((LexerKeyword*)a)->str), 
    (char*)(((LexerKeyword*)b)->str) );

}

//==============================================================================
// bsearch comparison function
// Function is static, no pollution of namespace
//==============================================================================
static int bSearchCmp( const void * a, const void * b ) {

  return strcmp( (char*) a,                            // Search key
    (char*)(((LexerKeyword*)b)->str) );  // Element in array

}

//==============================================================================
// Sort the reserved words using stdlib qsort
//==============================================================================
void Lexer::sortKeywords( void ) {

  qsort( keywords,                     // ptr to base of array
    keywordsLen,                       // number of elements in array
    sizeof( LexerKeyword ),           // byte size of a single element
    qSortCmp );                        // ptr to comparison function

}

//==============================================================================
// Searches the sorted reserved words with stdlib bsearch
//==============================================================================
LexerKeyword * Lexer::isKeyword( char * s ) const {

  return (LexerKeyword*) bsearch( s,               // key we are looking for
    keywords,                                       // base of array
    keywordsLen,                                    // Length of array
    sizeof( LexerKeyword ),                        // Byte size of array elem
    bSearchCmp );                                   // Comparator

}

//==============================================================================
// Warning/Error handling routine
// Handle each message based on type
//==============================================================================
void Lexer::lexerWError( const LEX_WERROR type, const char * msg, ... ) {

  char text[256];
  va_list ap;

  // Parse input arguments if there is one
  if( msg ) {
    va_start( ap, msg );
    vsprintf( text, msg, ap );
    va_end( ap );
  }

  // Return if just a warning, break to exit program
  switch( type ) {

  case WARNING_MESSAGE:
    fprintf( stderr, "Lexer Warning: %s\n", text );
    return;
  case ERROR_MESSAGE:
    fprintf( stderr, "Lexer Error: %s\n", text );
    break;
  case UNKNOWN_INPUT:
    fprintf( stderr, "Lexer Error: Unknown Input on line %d\n", lineNo );
    break;
  case MIN_TOK_VAL_ERROR:
    fprintf( stderr, "Lexer Error: Minimum reserved word type value should be"
      " at minimum 300\n" );
    break;
  default:
    fprintf( stderr, "Lexer Error: Unknown Error Type\n" );
  }
  exit(0);

}
