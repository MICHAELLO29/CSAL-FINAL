#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//Tokens lexer
typedef enum {
    Token_Digit, Token_Opt,
    Token_Main, Token_LPar, Token_RPar,
    Token_LeftB, Token_RightB, Token_Engage, Token_Equal, Token_Terminator,
    Token_Identity, Token_Denote, Token_Fuse, Token_Insert
} TokenType;

//Stuct of code
typedef struct {
    TokenType type;
    int value;         // For Numbers
    char name[256];     // For Identifiers (variable names)
    char str_value[256]; //for String literals
} Token;

// Variable structure for storing variables and their values
typedef struct {
    char name[256];
    int value; // For integers
    char str_value[256]; // Add this for string variables
} Variable;


Variable variables[256];
int variable_count = 0;

//Declare as a global variable
int parse_insert(Token *tokens, int *pos);


// Function to find or add a variable
int get_or_add_variable(const char *name) {
	int i;
    for (i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return i;
        }
    }
    strcpy(variables[variable_count].name, name);
    variables[variable_count].value = 0;
    return variable_count++;
}

//Tokenize Functions
Token *tokenize(const char *input) {
    Token *tokens = malloc(256 * sizeof(Token));
    int pos = 0;

    while (*input) {
        if (isdigit(*input)) {
            tokens[pos].type = Token_Digit;
            tokens[pos].value = strtol(input, (char **)&input, 10);
            pos++;
        } else if (*input == '+') {
            tokens[pos].type = Token_Opt;
    		tokens[pos++].value = '+';
    		input++;
        } else if (*input == '-') {
            tokens[pos].type = Token_Opt;
   			tokens[pos++].value = '-';
    		input++;
        } else if (*input == '*') {
            tokens[pos].type = Token_Opt;
    		tokens[pos++].value = '*';
    		input++;
        } else if (*input == '/') {
            tokens[pos].type = Token_Opt;
    		tokens[pos++].value = '/';
    		input++;
        } else if (strncmp(input, "Main", 4) == 0) {
            tokens[pos++].type = Token_Main;
            input += 4;
        } else if (strncmp(input, "=", 1) == 0) {
            tokens[pos++].type = Token_Equal;
            input += 1;
        } else if (strncmp(input, "Insert", 6) == 0) {
            tokens[pos++].type = Token_Insert;
            input += 6;
        } else if (*input == '(') {
            tokens[pos++].type = Token_LPar;
            input++;
        } else if (*input == ')') {
            tokens[pos++].type = Token_RPar;
            input++;
        } else if (*input == '{') {
            tokens[pos++].type = Token_LeftB;
            input++;
        } else if (*input == '}') {
            tokens[pos++].type = Token_RightB;
            input++;
        } else if (strncmp(input, "Engage", 6) == 0) {
            tokens[pos++].type = Token_Engage;
            input += 6;
        } else if (*input == ';') {
            tokens[pos++].type = Token_Terminator;
            input++;
        } else if (*input == ',') {
            tokens[pos++].type = Token_Fuse;
            input++;
        } else if (*input == '"') {
            tokens[pos].type = Token_Denote;
            input++; 
            int str_pos = 0;
            while (*input != '"' && *input != '\0') {
                tokens[pos].str_value[str_pos++] = *input++;
            }
            tokens[pos].str_value[str_pos] = '\0'; 
            if (*input == '"') {
                input++;
            }
            pos++;
		} else if (isalpha(*input)) { 
			tokens[pos].type = Token_Identity;
    		int i = 0;
    		while (isalpha(*input) || isdigit(*input) || *input == '_') {
        		tokens[pos].name[i++] = *input++;
    		}
    		tokens[pos].name[i] = '\0';
    		pos++;
    		
        } else {
            input++; // Skip unrecognized characters (e.g., spaces)
        }
	}
    tokens[pos].type = Token_Terminator;
    return tokens;
}

// Parser
// Parsing function that has multiplication and division
int parse_multidiv(Token *tokens, int *pos) {
    int result = 0;

    // Parse the first number or variable
    if (tokens[*pos].type == Token_Digit) {
        result = tokens[*pos].value;
        (*pos)++;
    } else if (tokens[*pos].type == Token_Identity) {
        int var_index = get_or_add_variable(tokens[*pos].name);
        result = variables[var_index].value;
        (*pos)++;
    } else {
        printf(" ");
        exit(1);
    }

    // Handle multiplication (*) and division (/)
    while (tokens[*pos].type == Token_Opt && 
          (tokens[*pos].value == '*' || tokens[*pos].value == '/')) {
        char op = tokens[*pos].value; // Store the operator
        (*pos)++;

        if (tokens[*pos].type != Token_Digit && tokens[*pos].type != Token_Identity) {
            printf("Error: An Identity or Digit should be included\n");
            exit(1);
        }

        int next_value = (tokens[*pos].type == Token_Digit)
                             ? tokens[*pos].value
                             : variables[get_or_add_variable(tokens[*pos].name)].value;
        (*pos)++;

        if (op == '*') {
            result *= next_value;
        } else if (op == '/') {
            if (next_value == 0) {
                printf("Error: Zero return\n");
                exit(1);
            }
            result /= next_value;
        }
    }

    return result;
}

// Handle addition and subtraction
int parse_addsub(Token *tokens, int *pos) {
    int result = parse_multidiv(tokens, pos);

    // Handle addition (+) and subtraction (-)
    while (tokens[*pos].type == Token_Opt && 
          (tokens[*pos].value == '+' || tokens[*pos].value == '-')) {
        char op = tokens[*pos].value; // Store the operator
        (*pos)++;

        int next_value = parse_multidiv(tokens, pos);

        if (op == '+') {
            result += next_value;
        } else if (op == '-') {
            result -= next_value;
        }
    }

    return result;
}


// Function to parse the equal statements
void parse_equal(Token *tokens, int *pos) {
    if (tokens[*pos].type != Token_Identity) {
        printf("Error: An Identity should be included before '='\n");
        exit(1);
    }

    // Retrieve Identity and its index
    char variable_name[32];
    strcpy(variable_name, tokens[*pos].name);
    int var_index = get_or_add_variable(variable_name);
    (*pos)++;

    if (tokens[*pos].type != Token_Equal) {
        printf("Error: An '=' need to be included to proceed");
        exit(1);
    }
    (*pos)++;

    // Handle assignment based on token type
    if (tokens[*pos].type == Token_Insert) {
        // Parse Insert statement
        int int_value = parse_insert(tokens, pos);
        if (tokens[*pos - 1].str_value[0] != '\0') {
            strcpy(variables[var_index].str_value, tokens[*pos - 1].str_value);
            variables[var_index].value = 0; // Reset integer value
        } else {
            variables[var_index].value = int_value;
            variables[var_index].str_value[0] = '\0'; // Reset string value
        }
    } else if (tokens[*pos].type == Token_Denote) {
        // Assign string literals directly to the string list
        strcpy(variables[var_index].str_value, tokens[*pos].str_value);
        variables[var_index].value = 0; // Reset integer value
        (*pos)++;
    } else {
        // Parse general expressions
        int value = parse_addsub(tokens, pos);
        variables[var_index].value = value;
        variables[var_index].str_value[0] = '\0'; // Reset string value
    }

    // Skip redundant semicolons
    while (tokens[*pos].type == Token_Terminator) {
        (*pos)++;
    }
}



//Printing using Engage
void parse_engage(Token *tokens, int *pos) {
    if (tokens[*pos].type != Token_Engage) {
        printf("Error: Expected 'Engage' to be included\n");
        exit(1);
    }
    (*pos)++;

    if (tokens[*pos].type != Token_LPar) {
        printf("Error: Expected '(' after 'Engage' to be included\n");
        exit(1);
    }
    (*pos)++;

    // Loop through the tokens inside Engage so that it can handle not only one engage statement
    while (tokens[*pos].type != Token_RPar) {
        if (tokens[*pos].type == Token_Denote) {
            // Print a string literal
            printf("%s", tokens[*pos].str_value);
            (*pos)++;
        } else if (tokens[*pos].type == Token_Identity) {
            // Handle variables (string or numeric)
            int var_index = get_or_add_variable(tokens[*pos].name);
            if (strlen(variables[var_index].str_value) > 0) {
                // Print string variable
                printf("%s", variables[var_index].str_value);
            } else {
                // Print numeric variable
                printf("%d", variables[var_index].value);
            }
            (*pos)++;
        } else if (tokens[*pos].type == Token_Fuse) {
            // Skip commas separating arguments
            (*pos)++;
        } else {
            // Handle unexpected token
            printf("Error: Unexpected token in 'Engage' statement\n");
            exit(1);
        }
    }

    if (tokens[*pos].type != Token_RPar) {
        printf("Error: Expected ')' after Engage statement\n");
        exit(1);
    }
    (*pos)++;

    if (tokens[*pos].type == Token_Terminator) {
        (*pos)++; // Skip the semicolon after 'Engage' statement
    } else {
        printf("Error: Expected ';' after 'Engage' statement\n");
        exit(1);
    }

    printf("\n"); // Add a newline after the Engage output so that the next print is in the next line
}

// Function to parse the Insert statement
int parse_insert(Token *tokens, int *pos) {
    if (tokens[*pos].type != Token_Insert) {
        printf("Error: Expected 'Insert'\n");
        exit(1);
    }
    (*pos)++;

    if (tokens[*pos].type != Token_LPar) {
        printf("Error: Expected '(' after 'Insert to be included'\n");
        exit(1);
    }
    (*pos)++;

    if (tokens[*pos].type != Token_Denote) {
        printf("Error: Expected a string prompt inside 'Insert'\n");
        exit(1);
    }
    char prompt[256];
    strcpy(prompt, tokens[*pos].str_value);
    printf("%s", prompt); // Print the prompt
    (*pos)++;

    if (tokens[*pos].type != Token_RPar) {
        printf("Error: Expected ')' after prompt\n");
        exit(1);
    }
    (*pos)++;

    if (tokens[*pos].type != Token_Terminator) {
        printf("Error: Expected ';' after 'Insert' statement\n");
        exit(1);
    }
    (*pos)++;

    // Read user input
    char input_str[256];
    if (!fgets(input_str, sizeof(input_str), stdin)) {
        printf("Error: Invalid input\n");
        exit(1);
    }

    // Remove newline character if present
    input_str[strcspn(input_str, "\n")] = '\0';

    // Attempt to parse the input as an integer
    char *end_ptr;
    int input_value = strtol(input_str, &end_ptr, 10);

    if (*end_ptr == '\0') {
        // Return if its a valid integer
        return input_value;
    } else {
        // Input is not a valid integer, treat it as a string
        strcpy(tokens[*pos - 1].str_value, input_str);
        return 0; // Return 0 for strings as a placeholder
    }
}

void parse_blitz(Token *tokens) {
    int pos = 0;

    if (tokens[pos].type != Token_Main) {
        printf("Error: Missing 'Main in the struct'\n");
        return;
    }
    pos++;

    if (tokens[pos].type != Token_LPar || tokens[pos + 1].type != Token_RPar) {
        printf("Error: Missing '() in the struct'\n");
        return;
    }
    pos += 2;

    if (tokens[pos].type != Token_LeftB) {
        printf("Error: Missing '{' for body in struct\n");
        return;
    }
    pos++;

    while (tokens[pos].type != Token_RightB) {
        if (tokens[pos].type == Token_Identity && tokens[pos + 1].type == Token_Equal) {
            parse_equal(tokens, &pos); // Handle assignment using '='
        } else if (tokens[pos].type == Token_Engage) {
            parse_engage(tokens, &pos); // Handle Engage
        } else if (tokens[pos].type == Token_Insert) {
            parse_insert(tokens, &pos); // Handle Insert statement
        } else {
            printf("Error: Unexpected token in body\n");
            exit(1);
        }
    }
    pos++;
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]); // Choose filename to read
        return 1;
    }

    FILE *file = fopen(argv[1], "r"); // Read the file
    if (!file) {
        printf("Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    char input[256];
    size_t len = fread(input, 1, sizeof(input) - 1, file);
    input[len] = '\0';  // Null-terminate the string

    fclose(file);

    Token *tokens = tokenize(input);

    parse_blitz(tokens);

    free(tokens);
    return 0;
}
