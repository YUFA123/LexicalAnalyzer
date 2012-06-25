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
Lexer::Lexer( const char * fileName, LexerReserved * r, int rLen ) {

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
  reserved = r;
  reservedLen = rLen;

  // Assert all reserved word type ints are greater than 300
  for( int i = 0; i < reservedLen; i++ ) {
    if( reserved[i].type < MIN_TOK_VAL ) 
      lexerWError( MIN_TOK_VAL_ERROR );
  }

  sortReserved();
  punct = "+-/*()[]{}!@#$%^&<>=|:;?~.,'`";

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
bool Lexer::nextToken( LexerToken & t ) {

  int c;

  // Kill whitespace
  if( !readWhiteSpace() ) return false;

  c = *fileBuf; // First non-whitespace character

  // Check for numeric constant
  if( isNum( c ) ) 
    return readNumeric( t );
  
  // Identifiers or reserved words
  if( isAlpha( c ) ) 
    return readIdentifier( t );

  // All punctuation
  if( readPunctuation( t ) ) 
    return true;

  // Should never reach here
  lexerWError( UNKNOWN_INPUT, 0 );
  return false;
}

//==============================================================================
// match( int t )
// Determines if the next lexeme the token you provide
// If no match, return false, else return true
//==============================================================================
bool Lexer::match( int t ) {

  LexerToken tok;
  if( nextToken( tok ) ) {
    
    if( tok.type == t ) return true;
   
  }
  return false;

}

//==============================================================================
// Reads any punctuation
// Returns 1 if there was punctuation, otherwise return 0
//==============================================================================
int Lexer::readPunctuation( LexerToken & t ) {

  // Get first punctuation symbol
  for( unsigned int i = 0; i < strlen(punct); i++ ) {

    if( *fileBuf == punct[i] ) {
      t.type = punct[i];
      fileBuf++;

      // Handle multi-punctuation symbols here

      // <=
      if( t.type == '<' && *fileBuf == '=' ) {
	t.type = LEXER_LE;
	fileBuf++;
      }

      // >=
      if( t.type == '>' && *fileBuf == '=' ) {
	t.type = LEXER_GE;
	fileBuf++;
      }

      // ==
      if( t.type == '=' && *fileBuf == '=' ) {
	t.type = LEXER_EQEQ;
	fileBuf++;
      }

      // <<
      if( t.type == '<' && *fileBuf == '<' ) {
	t.type = LEXER_SL;
	fileBuf++;
      }

      // >>
      if( t.type == '>' && *fileBuf == '>' ) {
	t.type = LEXER_SR;
	fileBuf++;
      }
      
      // There was at least one punct mark
      return 1;
    }
  }

  // No punct marks
  return 0;
}

//==============================================================================
// Reads an identifier 
// Begins with a alpha character, From then on contains any 
//    alpha-numeric or underscore
//==============================================================================
int Lexer::readIdentifier( LexerToken & t ) {

  char id[MAX_ID_LEN];
  char * i = &id[0];
  LexerReserved * r = 0;

  // Scoop up the identifier into the token
  do {
    *i++ = *fileBuf++;
  } while( isAlphaNum( *fileBuf ) );
  
  *i = '\0';

  // Check reserved words
  for( int k = 0; k < reservedLen; k++ ) {
    if( ( r = isReserved( id ) ) ) {
      t.type = r->type;
      return 1;
    }
  }

  // Not a reserved word, an identifier
  strcpy( t.s, id );
  t.type = LEXER_ID;

  return 1;

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
  
  t.type = LEXER_REAL;
  t.f = atof( num );
  
  return 1;

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

  return strcmp( (char*)(((LexerReserved*)a)->str), 
		 (char*)(((LexerReserved*)b)->str) );

}

//==============================================================================
// bsearch comparison function
// Function is static, no pollution of namespace
//==============================================================================
static int bSearchCmp( const void * a, const void * b ) {

  return strcmp( (char*) a,                            // Search key
		 (char*)(((LexerReserved*)b)->str) );  // Element in array

}

//==============================================================================
// Sort the reserved words using stdlib qsort
//==============================================================================
void Lexer::sortReserved( void ) {
  
  qsort( reserved,                     // ptr to base of array
	 reservedLen,                  // number of elements in array
	 sizeof( LexerReserved ),     // byte size of a single element
	 qSortCmp );                   // ptr to comparison function

}

//==============================================================================
// Searches the sorted reserved words with stdlib bsearch
//==============================================================================
LexerReserved * Lexer::isReserved( char * s ) const {

  return (LexerReserved*) bsearch( s,                        // key we are looking for
				   reserved,                 // base of array
				   reservedLen,              // Length of array
				   sizeof( LexerReserved ), // Byte size of array elem
				   bSearchCmp );             // Comparator

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
