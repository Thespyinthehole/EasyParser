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
    token_assign,
    token_integer = 4,
    token_open_bracket,
    token_close_bracket
};

ParseTree operation()
{
    return token_add | token_divide | token_multiply | token_subtract;
}

ParseTree bracket_expression();
ParseTree full_expression();
ParseTree extra();

ParseTree expression()
{
    return (token_integer + extra);
}

ParseTree bracket_expression()
{
    return token_open_bracket + full_expression + token_close_bracket;
}

ParseTree full_expression()
{
    return (bracket_expression | ParseTree(expression)) + extra;
}

ParseTree declaration()
{
    return token_let + token_variable_name + token_assign + full_expression;
}

ParseTree extra()
{
    return (operation + ParseTree(full_expression)) | epsilon;
}

ParseTree assignment()
{
    return token_variable_name + token_assign + full_expression;
}

ParseTree statement()
{
    return declaration + token_line_end;
}

ParseTree statements()
{
    return (ParseTree(statement) + statements) | epsilon;
}

ParseTree program()
{
    return statements + token_end_of_field;
}

int main()
{
    //Define a program string
    std::string program_string = "let x = (1*3) + (4); let b = 5;";

    //Make a new parser
    EasyParser parser;

    //Add the token regex definitions
    parser.add_new_token(token_white_space, "\\s");
    parser.add_new_token(token_variable_name, "[a-zA-Z_][a-zA-Z0-9_]*");
    parser.add_new_token(token_let, "let");
    parser.add_new_token(token_add, "\\+");
    parser.add_new_token(token_subtract, "-");
    parser.add_new_token(token_multiply, "\\*");
    parser.add_new_token(token_divide, "/");
    parser.add_new_token(token_assign, "=");
    parser.add_new_token(token_integer, "-?[0-9]+");
    parser.add_new_token(token_line_end, ";");
    parser.add_new_token(token_open_bracket, "\\(");
    parser.add_new_token(token_close_bracket, "\\)");

    parser.end_of_field_token = token_end_of_field;

    //This is needed to parse
    parser.register_tree(program, "program");
    parser.register_tree(statement, "statement");
    // parser.register_tree(assignment,"assignment");
    parser.register_tree(declaration, "declaration");
    parser.register_tree(full_expression, "full_expression");
    parser.register_tree(bracket_expression, "bracket_expression");
    parser.register_tree(expression, "expression");
    parser.register_tree(operation, "operation");
    parser.register_tree(statements, "statements");
    parser.register_tree(extra, "extra");

    //This is needed for pretty output
    parser.register_token_name(token_line_end, "token_line_end");
    parser.register_token_name(token_variable_name, "token_variable_name");
    parser.register_token_name(token_add, "token_add");
    parser.register_token_name(token_subtract, "token_subtract");
    parser.register_token_name(token_multiply, "token_multiply");
    parser.register_token_name(token_divide, "token_divide");
    parser.register_token_name(token_let, "token_let");
    parser.register_token_name(token_assign, "token_assign");
    parser.register_token_name(token_integer, "token_integer");
    parser.register_token_name(token_open_bracket, "token_open_bracket");
    parser.register_token_name(token_close_bracket, "token_close_bracket");
    parser.register_token_name(token_end_of_field, "EOF");

    parser.add_ignored_token(token_white_space);

    //This needs to be set
    parser.program = program;

    try
    {
        parser.parse(program_string);
    }
    catch (LexicalException e)
    {
        printf("%s", e.what());
    }
    catch (SyntaxException e)
    {
        printf("Syntax error on line %d at character %d, error: %s", e.error_token.line_number, e.error_token.start_character, e.error_token.value.c_str());
    }
}