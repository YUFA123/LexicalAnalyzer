class Lexan

NOTABLES:

-semi-dynamic lexical analysis: reserved words provided at class construction
	this is useful if you are parsing multiple file types in the execution of one program
	for me, parsing SDL's, object model files, polygon lists, etc.

-generic parsing scheme and parsing order
	1) numerical constants
	2) identifiers (words/variable/reserved words)
	3) punctuation

-parsing types functioned off for simple modification
	readWhiteSpace()  // eats # comments too
	readPunctuation() // single and multiple character punctuation possible and easily added
	etc...

-entire file is read into memory, ideal for most applications, prevents many small system 
	calls, and also prevents unnecessary complexity of circular buffers, RAM is cheap
	if you're parsing files > GB's, i'm sorry

-defines own type definitions for more control
	isNum()
	isAlpha()
	isAlphaNum()
	isWhiteSpace()

-simple interface	

	bool nextToken( LexerToken & t )
		returns true if token information was stored in 't' otherwise eof

	bool match( int t )
		useful for "asserting" the next lexeme in the stream is equal to 't'
		typical case
			match( '(' ) 
			returns true if the next token is '(', false otherwise

-stupid simple error system
	in-class warning/error enum and method
	pass in type of warning and message and handle accordingly
	strings/messages passed similar to printf()
	pros: easy, expandable, great for debugging
	cons: links against cstdio,cstdarg, may not have a console to print to

	if used in a more mature/commercial setting, a simple warning/error "state"
	would be more appropriate, the program using Lexer would check the state and 
	react accordingly

USAGE:

class Lexan lex( char * fileName, LexanReserved * words, int listLength );
	fileName = duh
	words = struct of words and their lexical "value", example provided shows
		a simple enum solves this. 
		Minimum lexical value = 300, to prevent conflicts, again look at example
	listLength = number of reserved words you are providing

Two public methods

1) bool nextToken( LexerToken & t )
	returns true if lexeme found, stores results in t
	return false if eof
2) bool match( int t )
	return true if next lexeme value matches t



email: ajmontgomery5@gmail.com