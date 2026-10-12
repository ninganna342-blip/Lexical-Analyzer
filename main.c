/* Projec Title : Lexical Analyzer
   Description  : This project reads the source file character by character, identifies and classifies lexemes, 
                  handles single-line and multi-line comments, tracks line numbers, detects basic lexical errors, 
                  and stores the generated tokens using a structured token table.*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"


int main(int argc, char *argv[])
{
    // Check whether the source file name is provided
    if(argc != 2)
    {
        printf("Usage: ./lexer <source_file>\n");
        return 1;
    }

    // Open the source file in read mode
    FILE *fp = fopen(argv[1], "r");

    // Check whether the file was opened successfully
    if(fp == NULL)
    {
        perror("fopen");
        return 1;
    }

    // Temporary token used while identifying each lexeme
    Token token;

    // Array used to store all identified tokens
    Token tokens[1000];

    // Keep track of the number of tokens stored
    int token_count = 0;

    // Keep track of the current line number
    int line_number = 1;

    int ch;

    // Read the source file character by character
    while((ch = fgetc(fp)) != EOF)
    {
        // Increment line number when a newline is encountered
        if(ch == '\n')
        {
            line_number++;
            continue;
        }

        // Ignore spaces, tabs and other whitespace characters
        if(isspace(ch))
        {
            continue;
        }


        // Check whether the character starts an identifier or keyword
        if(isalpha(ch) || ch == '_')
        {
            char buffer[100];
            int index = 0;

            // Store the first character of the lexeme
            buffer[index++] = ch;

            // Continue reading the identifier/keyword
            while((ch = fgetc(fp)) != EOF)
            {
                // Identifier can contain letters, digits and underscore
                if(isalnum(ch) || ch == '_')
                {
                    buffer[index++] = ch;
                }
                else
                {
                    // Put back the character that does not belong to the lexeme
                    if(ch != EOF)
                    {
                        ungetc(ch, fp);
                    }

                    break;
                }
            }

            // Add null character to terminate the string
            buffer[index] = '\0';


            // Check whether the lexeme is a C keyword
            if(is_keyword(buffer) == 0)
            {
                create_token(&token, buffer, TOKEN_KEYWORD, line_number);
            }
            else
            {
                // If it is not a keyword, classify it as an identifier
                create_token(&token, buffer, TOKEN_IDENTIFIER, line_number);
            }

            // Store the generated token in the token array
            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Check whether the character starts a numeric constant
        else if(isdigit(ch))
        {
            char buffer[100];

            int index = 0;
            int has_dot = 0;
            int invalid = 0;

            // Store the first digit
            buffer[index++] = ch;

            // Continue reading the numeric constant
            while((ch = fgetc(fp)) != EOF)
            {
                // Continue reading digits
                if(isdigit(ch))
                {
                    buffer[index++] = ch;
                }

                // Allow one decimal point
                else if(ch == '.' && has_dot == 0)
                {
                    has_dot = 1;
                    buffer[index++] = ch;
                }

                // Detect an alphabetic character immediately after a number
                // Example: 123abc
                else if(isalpha(ch) || ch == '_')
                {
                    invalid = 1;
                    buffer[index++] = ch;

                    // Consume the remaining identifier characters
                    while((ch = fgetc(fp)) != EOF)
                    {
                        if(isalnum(ch) || ch == '_')
                        {
                            buffer[index++] = ch;
                        }
                        else
                        {
                            // Put back the character that belongs to the next token
                            if(ch != EOF)
                            {
                                ungetc(ch, fp);
                            }

                            break;
                        }
                    }

                    break;
                }

                else
                {
                    // Put back the character that belongs to the next token
                    if(ch != EOF)
                    {
                        ungetc(ch, fp);
                    }

                    break;
                }
            }

            // Terminate the numeric lexeme
            buffer[index] = '\0';

            // Report an error for invalid numeric constants such as 123abc
            if(invalid)
            {
                printf("Lexical Error: Invalid numeric constant '%s' at line %d\n",
                       buffer, line_number);
                continue;
            }

            // Create a constant token
            create_token(&token, buffer, TOKEN_CONSTANT, line_number);

            // Store the constant token
            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Handle +, ++ and += operators
        else if(ch == '+')
        {
            int next = fgetc(fp);

            // Increment operator
            if(next == '+')
            {
                create_token(&token, "++", TOKEN_OPERATOR, line_number);
            }

            // Addition assignment operator
            else if(next == '=')
            {
                create_token(&token, "+=", TOKEN_OPERATOR, line_number);
            }

            else
            {
                // Put back the character if it belongs to another token
                if(next != EOF)
                {
                    ungetc(next, fp);
                }

                // Addition operator
                create_token(&token, "+", TOKEN_OPERATOR, line_number);
            }

            // Store the operator token
            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Handle -, -- and -= operators
        else if(ch == '-')
        {
            int next = fgetc(fp);

            // Decrement operator
            if(next == '-')
            {
                create_token(&token, "--", TOKEN_OPERATOR, line_number);
            }

            // Subtraction assignment operator
            else if(next == '=')
            {
                create_token(&token, "-=", TOKEN_OPERATOR, line_number);
            }

            else
            {
                if(next != EOF)
                {
                    ungetc(next, fp);
                }

                // Subtraction operator
                create_token(&token, "-", TOKEN_OPERATOR, line_number);
            }

            // Store the operator token
            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Handle = and == operators
        else if(ch == '=')
        {
            int next = fgetc(fp);

            // Equality operator
            if(next == '=')
            {
                create_token(&token, "==", TOKEN_OPERATOR, line_number);
            }

            else
            {
                if(next != EOF)
                {
                    ungetc(next, fp);
                }

                // Assignment operator
                create_token(&token, "=", TOKEN_OPERATOR, line_number);
            }

            // Store the operator token
            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Handle < and <= operators
        else if(ch == '<')
        {
            int next = fgetc(fp);

            if(next == '=')
            {
                create_token(&token, "<=", TOKEN_OPERATOR, line_number);
            }

            else
            {
                if(next != EOF)
                {
                    ungetc(next, fp);
                }

                create_token(&token, "<", TOKEN_OPERATOR, line_number);
            }

            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Handle > and >= operators
        else if(ch == '>')
        {
            int next = fgetc(fp);

            if(next == '=')
            {
                create_token(&token, ">=", TOKEN_OPERATOR, line_number);
            }

            else
            {
                if(next != EOF)
                {
                    ungetc(next, fp);
                }

                create_token(&token, ">", TOKEN_OPERATOR, line_number);
            }

            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Handle ! and != operators
        else if(ch == '!')
        {
            int next = fgetc(fp);

            if(next == '=')
            {
                create_token(&token, "!=", TOKEN_OPERATOR, line_number);
            }

            else
            {
                if(next != EOF)
                {
                    ungetc(next, fp);
                }

                create_token(&token, "!", TOKEN_OPERATOR, line_number);
            }

            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Handle & and && operators
        else if(ch == '&')
        {
            int next = fgetc(fp);

            if(next == '&')
            {
                create_token(&token, "&&", TOKEN_OPERATOR, line_number);
            }

            else
            {
                if(next != EOF)
                {
                    ungetc(next, fp);
                }

                create_token(&token, "&", TOKEN_OPERATOR, line_number);
            }

            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Handle | and || operators
        else if(ch == '|')
        {
            int next = fgetc(fp);

            if(next == '|')
            {
                create_token(&token, "||", TOKEN_OPERATOR, line_number);
            }

            else
            {
                if(next != EOF)
                {
                    ungetc(next, fp);
                }

                create_token(&token, "|", TOKEN_OPERATOR, line_number);
            }

            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Handle multiplication and modulus operators
        else if(ch == '*' || ch == '%')
        {
            char buffer[2];

            buffer[0] = ch;
            buffer[1] = '\0';

            create_token(&token, buffer, TOKEN_OPERATOR, line_number);

            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Handle brackets, braces, parentheses and other special symbols
        else if(ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == '[' || ch == ']' || ch == ';' || ch == ',' || ch == ':' || ch == '.')
        {
            char buffer[2];

            buffer[0] = ch;
            buffer[1] = '\0';

            create_token(&token, buffer, TOKEN_SPECIAL_SYMBOL, line_number);

            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Handle division operator and comments
        else if(ch == '/')
        {
            int next = fgetc(fp);

            // Handle single-line comment
            if(next == '/')
            {
                handle_single_line_comment(fp, &line_number);
            }

            // Handle multi-line comment
            else if(next == '*')
            {
                if(handle_multiline_comment(fp, &line_number) == 1)
                {
                    printf("Error: Unterminated comment\n");
                    fclose(fp);
                    return 1;
                }
            }

            // Handle division assignment operator
            else if(next == '=')
            {
                create_token(&token, "/=", TOKEN_OPERATOR, line_number);

                if(add_token(tokens, &token_count, token) == 1)
                {
                    printf("Error: Maximum token limit reached\n");
                    fclose(fp);
                    return 1;
                }
            }

            else
            {
                // Put back the next character if it belongs to another token
                if(next != EOF)
                {
                    ungetc(next, fp);
                }

                // Division operator
                create_token(&token, "/", TOKEN_OPERATOR, line_number);

                if(add_token(tokens, &token_count, token) == 1)
                {
                    printf("Error: Maximum token limit reached\n");
                    fclose(fp);
                    return 1;
                }
            }
        }


        // Handle string literals
        else if(ch == '"')
        {
            if(handle_string(fp, &token, line_number) == 1)
            {
                printf("Error: Unterminated string literal\n");
                fclose(fp);
                return 1;
            }

            // Store the string literal token
            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Handle character constants
        else if(ch == '\'')
        {
            if(handle_character(fp, &token, line_number) == 1)
            {
                printf("Error: Invalid character constant\n");
                fclose(fp);
                return 1;
            }

            // Store the character constant token
            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Handle preprocessor directives
        else if(ch == '#')
        {
            if(handle_preprocessor(fp, &token, &line_number) == 1)
            {
                printf("Error processing preprocessor directive\n");
                fclose(fp);
                return 1;
            }

            // Store the preprocessor token
            if(add_token(tokens, &token_count, token) == 1)
            {
                printf("Error: Maximum token limit reached\n");
                fclose(fp);
                return 1;
            }
        }


        // Report any character that is not recognized by the lexer
        else
        {
            printf("Lexical Error: Invalid character '%c' at line %d\n",
                   ch, line_number);
        }
    }


    // Close the source file after lexical analysis
    fclose(fp);


    // Display the lexical analyzer heading
    printf("\n=================================================================\n");
    printf("                         LEXICAL ANALYZER\n");
    printf("=================================================================\n\n\n");


    // Display the token table header
    printf("%-25s %-30s %5s\n",
           "Lexeme",
           "Token Type",
           "Line");

    printf("----------------------------------------------------------------\n");


    // Print all tokens stored in the token array
    for(int i = 0; i < token_count; i++)
    {
        print_token(tokens[i]);
    }

    return 0;
}