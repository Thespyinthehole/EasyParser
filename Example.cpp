#include "EasyParser.h"

enum Tokens : int
{
    token_white_space = 0,
    token_line_end = 10,
    token_variable_name = -20,
    token_end_of_field = -30,
    token_add,
    token_subtract,
    token_multiply,
    token_divide,
    token_let = 2,
    token_print = 100,
    token_assign,
    token_integer = 4,
    token_string_type
};

Token assignment_var_name;
Token assignment_var_value;
Token operator_token;
Token print_token;

int value;
std::map<std::string, int> variables;

void on_success()
{
    
}

void on_new_assignment()
{
    assignment_var_name = Token();
    assignment_var_value = Token();
    value = 0;
}

void on_successful_assignment()
{
    variables.insert(std::pair<std::string, int>(assignment_var_name.value, value));
}

void on_successful_replacement()
{
    variables[assignment_var_name.value] = value;
}

void on_pre_expression()
{
    if (assignment_var_value.token == token_integer)
        value = stoi(assignment_var_value.value);
    else
        value = variables[assignment_var_value.value];
}

void on_post_expression()
{
    int other_value = 0;
    if (assignment_var_value.token == token_integer)
        other_value = stoi(assignment_var_value.value);
    else
        other_value = variables[assignment_var_value.value];

    switch (operator_token.token)
    {
    case token_add:
        value += other_value;
        break;
    case token_subtract:
        value -= other_value;
        break;
    case token_divide:
        value /= other_value;
        break;
    case token_multiply:
        value *= other_value;
        break;
    }
}

void on_print()
{
    if (print_token.token == token_variable_name)
        printf("%d", variables[print_token.value]);
    else
        printf("%s", print_token.value.substr(1, print_token.value.size() - 2).c_str());
}

int main()
{
    //Define a program string
    std::string program_string = "let x = 10 + 1 ; let y = 5; let z = x + y; y = y * 2 ; print x; print y; print \"hello\";";

    //Make a new parser
    EasyParser parser;

    //Add the token regex definitions
    parser.add_new_token(token_white_space, "\\s+");
    parser.add_new_token(token_variable_name, "[a-zA-Z_][a-zA-Z0-9_]*");
    parser.add_new_token(token_let, "let");
    parser.add_new_token(token_print, "print");
    parser.add_new_token(token_line_end, ";");
    parser.add_new_token(token_add, "\\+");
    parser.add_new_token(token_subtract, "-");
    parser.add_new_token(token_multiply, "\\*");
    parser.add_new_token(token_divide, "/");
    parser.add_new_token(token_assign, "=");
    parser.add_new_token(token_integer, "-?[0-9]+");
    parser.add_new_token(token_string_type, "\"([^\"])*\"");

    //Set end of field
    parser.set_end_of_field_token(token_end_of_field);

    //Add ignored tokens
    parser.add_ignored_token(token_white_space);

    //Add single comment line, do not consume the new line

    //Add comment block, comsuming the end of the block comment token

    //Make the parse tree
    Syntax operation = (operator_token << token_add) | (operator_token << token_divide) | (operator_token << token_multiply) | (operator_token << token_subtract);

    Syntax expression =
        ((
             (
                 (assignment_var_value << token_integer) |
                 (assignment_var_value << token_variable_name)) >>
             on_pre_expression) +
         (!(
              operation +
              ((assignment_var_value << token_integer) |
               (assignment_var_value << token_variable_name))) >>
          on_post_expression));

    Syntax assignment =
        ((
             token_let >> on_new_assignment) +
         (assignment_var_name << token_variable_name) +
         !(
             token_assign +
             expression)) >>
        on_successful_assignment;

    Syntax replacement =
        ((assignment_var_name << token_variable_name) +
         !(
             token_assign +
             expression)) >>
        on_successful_replacement;

    Syntax print =
        (token_print +
         ((print_token << token_string_type) |
          (print_token << token_variable_name))) >>
        on_print;

    Syntax statement = assignment | replacement | print;
    Syntax program_syntax = ((~(statement + token_line_end)) + token_end_of_field) >> on_success;

    //Run the parser
    parser.parse(program_string, program_syntax);
}