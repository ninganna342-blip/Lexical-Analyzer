#ifndef DECLARATIONS_H
#define DECLARATIONS_H

// Maximum number of tokens that can be stored
#define MAX_TOKENS 1000

#include <stdio.h>


// Different types of tokens recognized by the lexical analyzer
typedef enum
{
    TOKEN_KEYWORD,          // C keyword such as int, if, while
    TOKEN_IDENTIFIER,       // Variable or function name
    TOKEN_CONSTANT,         // Integer or floating-point constant
    TOKEN_OPERATOR,         // Operators such as +, -, =, ==
    TOKEN_SPECIAL_SYMBOL,   // Symbols such as (, ), {, }, ;, ,
    TOKEN_STRING,           // String literal such as "Hello"
    TOKEN_CHARACTER,        // Character constant such as 'A'
    TOKEN_PREPROCESSOR      // Preprocessor directive such as #include
} TokenType;


// Structure used to store information about a single token
typedef struct
{
    char lexeme[100];       // Actual token text
    TokenType type;         // Type of the token
    int line;               // Line number where the token occurs
} Token;


// Keyword array defined in lexer.c
extern char *keywords[];


// Check whether a given word is a C keyword
int is_keyword(char *word);


// Handle single-line comments beginning with //
void handle_single_line_comment(FILE *fp, int *line_number);


// Handle multi-line comments enclosed between /* and */
int handle_multiline_comment(FILE *fp, int *line_number);


// Handle string literals enclosed between double quotes
int handle_string(FILE *fp, Token *token, int line_number);


// Handle character constants enclosed between single quotes
int handle_character(FILE *fp, Token *token, int line_number);


// Create and initialize a token
void create_token(Token *token, char *lexeme, TokenType type, int line_number);


// Print the details of a token
void print_token(Token token);


// Convert token type enum into a readable string
char *token_type_to_string(TokenType type);


// Add a token to the token array
int add_token(Token tokens[], int *token_count, Token token);


// Handle preprocessor directives beginning with #
int handle_preprocessor(FILE *fp, Token *token, int *line_number);


#endif