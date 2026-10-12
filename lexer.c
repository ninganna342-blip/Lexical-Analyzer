#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"


// List of all C keywords supported by the lexical analyzer
char *keywords[] = {"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern",
                    "float", "for", "goto", "if", "inline", "int", "long", "register", "return", "short", "signed", "sizeof",
                    "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"};


// Check whether the given word is a C keyword
int is_keyword(char *word)
{
    // Calculate the number of keywords in the array
    int count = sizeof(keywords) / sizeof(keywords[0]);

    // Compare the given word with every keyword
    for(int i = 0; i < count; i++)
    {
        // Return 0 when the word matches a keyword
        if(strcmp(word, keywords[i]) == 0)
        {
            return 0;
        }
    }

    // Return 1 when the word is not a keyword
    return 1;
}


// Skip characters until the end of a single-line comment
void handle_single_line_comment(FILE *fp, int *line_number)
{
    int ch;

    // Read characters until newline or end of file
    while((ch = fgetc(fp)) != '\n' && ch != EOF)
    {

    }

    // Increment line number when the newline is consumed
    if(ch == '\n')
    {
        (*line_number)++;
    }
}


// Skip characters until the closing */ of a multi-line comment
int handle_multiline_comment(FILE *fp, int *line_number)
{
    int ch;
    int next;

    // Read characters until the end of the file
    while((ch = fgetc(fp)) != EOF)
    {
        // Count new lines occurring inside the comment
        if(ch == '\n')
        {
            (*line_number)++;
        }

        // Check whether * may be the beginning of */
        if(ch == '*')
        {
            next = fgetc(fp);

            // Found the closing comment sequence */
            if(next == '/')
            {
                return 0;
            }

            // Put back the character if it was not /
            if(next != EOF)
            {
                // Count the newline if it was consumed as next
                if(next == '\n')
                {
                    (*line_number)++;
                }

                ungetc(next, fp);
            }
        }
    }

    // Return 1 when the comment was not terminated
    return 1;
}


// Read and create a string literal token
int handle_string(FILE *fp, Token *token, int line_number)
{
    char buffer[100];
    int index = 0;
    int ch;

    // Store the opening double quote
    buffer[index++] = '"';

    // Read characters until closing quote or EOF
    while((ch = fgetc(fp)) != EOF)
    {
        // Check for the closing double quote
        if(ch == '"')
        {
            buffer[index++] = '"';
            buffer[index] = '\0';

            // Create a string literal token
            create_token(token, buffer, TOKEN_STRING, line_number);

            return 0;
        }

        // Handle escape sequences such as \n and \"
        if(ch == '\\')
        {
            buffer[index++] = ch;

            // Read the character following the backslash
            ch = fgetc(fp);

            // EOF or newline means the string is unterminated
            if(ch == EOF || ch == '\n')
            {
                return 1;
            }

            buffer[index++] = ch;
        }

        else
        {
            // A newline before closing quote means invalid string
            if(ch == '\n')
            {
                return 1;
            }

            buffer[index++] = ch;
        }
    }

    // EOF was reached before closing quote
    return 1;
}


// Read and create a character constant token
int handle_character(FILE *fp, Token *token, int line_number)
{
    char buffer[100];
    int index = 0;

    int ch;
    int next;

    // Store the opening single quote
    buffer[index++] = '\'';

    // Read the character inside the character constant
    ch = fgetc(fp);

    // EOF or newline means invalid character constant
    if(ch == EOF || ch == '\n')
    {
        return 1;
    }

    // Handle escape characters such as '\n'
    if(ch == '\\')
    {
        buffer[index++] = ch;

        // Read the character following the backslash
        next = fgetc(fp);

        // EOF or newline means invalid character constant
        if(next == EOF || next == '\n')
        {
            return 1;
        }

        buffer[index++] = next;
    }

    else
    {
        // Store the normal character
        buffer[index++] = ch;
    }

    // Read the closing single quote
    next = fgetc(fp);

    // The character constant must end with a single quote
    if(next != '\'')
    {
        return 1;
    }

    // Store closing quote and terminate the string
    buffer[index++] = '\'';
    buffer[index] = '\0';

    // Create the character constant token
    create_token(token, buffer, TOKEN_CHARACTER, line_number);

    return 0;
}


// Create and initialize a token structure
void create_token(Token *token, char *lexeme, TokenType type, int line_number)
{
    // Copy the lexeme into the token
    strcpy(token->lexeme, lexeme);

    // Store the token type
    token->type = type;

    // Store the line number
    token->line = line_number;
}


// Print a token in formatted table form
void print_token(Token token)
{
    printf("%-25s %-30s %5d\n",
           token.lexeme,
           token_type_to_string(token.type),
           token.line);
}


// Convert TokenType enum value into a readable string
char *token_type_to_string(TokenType type)
{
    switch(type)
    {
        case TOKEN_KEYWORD:
            return "KEYWORD";

        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";

        case TOKEN_CONSTANT:
            return "CONSTANT";

        case TOKEN_OPERATOR:
            return "OPERATOR";

        case TOKEN_SPECIAL_SYMBOL:
            return "SPECIAL SYMBOL";

        case TOKEN_STRING:
            return "STRING LITERAL";

        case TOKEN_CHARACTER:
            return "CHARACTER CONSTANT";

        case TOKEN_PREPROCESSOR:
            return "PREPROCESSOR";

        default:
            return "UNKNOWN";
    }
}


// Add a newly created token to the token array
int add_token(Token tokens[], int *token_count, Token token)
{
    // Check whether the token array is full
    if(*token_count >= MAX_TOKENS)
    {
        return 1;
    }

    // Store the token at the current index
    tokens[*token_count] = token;

    // Increase the token count
    (*token_count)++;

    return 0;
}


// Read and create a preprocessor directive token
int handle_preprocessor(FILE *fp, Token *token, int *line_number)
{
    char buffer[200];
    int index = 0;
    int ch;

    // Store the # character
    buffer[index++] = '#';

    // Read the rest of the preprocessor directive
    while((ch = fgetc(fp)) != EOF && ch != '\n')
    {
        // Ignore carriage return characters
        if(ch == '\r')
        {
            continue;
        }

        // Prevent buffer overflow
        if(index >= 99)
        {
            printf("Error: Preprocessor directive too long\n");
            return 1;
        }

        // Store the character in the buffer
        buffer[index++] = ch;
    }

    // Remove any remaining carriage return
    if(index > 0 && buffer[index - 1] == '\r')
    {
        index--;
    }

    // Terminate the preprocessor directive string
    buffer[index] = '\0';

    // Create the preprocessor token
    create_token(token, buffer, TOKEN_PREPROCESSOR, *line_number);

    // Update line number after consuming the newline
    if(ch == '\n')
    {
        (*line_number)++;
    }

    return 0;
}