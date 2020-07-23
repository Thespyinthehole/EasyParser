#include "EasyParser.h"
#include <stack>
#include <stdarg.h>

void EasyParser::add_new_token(Tokens token, std::string regex)
{
    //Add this to the lexer
    lexer.add_new_token((int)token, regex);
}

void EasyParser::add_ignored_token(Tokens token)
{
    //Add the token to ignored list
    ignored_tokens.insert((int)token);
}

void EasyParser::add_comment_rule(Tokens start, Tokens end, bool consume)
{
    //Set up the new comment rule
    Comment new_rule{(int)end, consume};
    //Add the new rule
    comment_rules.insert(std::pair<int, Comment>((int)start, new_rule));
}

void EasyParser::parse(std::string read_string, Syntax syntax)
{
    //Get the tokens - dumped to list in class
    extract(read_string);

    //Begin parsing
    syntax.evaluate(*this);
}

void EasyParser::set_end_of_field_token(Tokens end_of_field)
{
    lexer.end_of_field_token = end_of_field;
}

Token EasyParser::next_token()
{
    if (current_token_position >= extracted_tokens.size())
        //Return invalid token
        return Token();
    return extracted_tokens[current_token_position];
}

Token EasyParser::token_after(int offset)
{
    if (current_token_position + offset >= extracted_tokens.size() || current_token_position + offset < 0)
        //Return invalid token
        return Token();
    return extracted_tokens[current_token_position + offset];
}

void EasyParser::extract(std::string read_string)
{
    //Set up the comment stack
    std::stack<Comment> comment_stack;

    //Extract the tokens from the string
    std::list<Token> tokens = lexer.parse(read_string);

    //Loop over tokens
    std::list<Token>::iterator iter;
    for (iter = tokens.begin(); iter != tokens.end(); iter++)
    {
        //Get the token
        Token next_token = *iter;

        //If we are in a comment
        if (comment_stack.size() != 0)
        {
            //Does the token we just found end the current comment
            if (next_token.token == comment_stack.top().end_token)
            {
                //Should we keep the last token
                if (!comment_stack.top().consume_last_token)
                    if (ignored_tokens.find(next_token.token) == ignored_tokens.end())
                        extracted_tokens.push_back(next_token);

                //Exit the current comment
                comment_stack.pop();
            }
            else
            { //Check to see if a new comment token has been found
                std::map<int, Comment>::iterator iter = comment_rules.find(next_token.token);

                //If so add a new comment layer
                if (iter != comment_rules.end())
                    comment_stack.push(iter->second);
            }
        }
        else
        {
            //Check to see if a new comment token has been found
            std::map<int, Comment>::iterator iter = comment_rules.find(next_token.token);
            if (iter != comment_rules.end())
                //Add the comment layer
                comment_stack.push(iter->second);
            //Should we ignore the token
            else if (ignored_tokens.find(next_token.token) == ignored_tokens.end())
                //Keep the token
                extracted_tokens.push_back(next_token);
        }
    }
}

Syntax::Syntax() {}

Syntax::Syntax(SyntaxTypes type, bool (*evaluate)(Syntax &, EasyParser &))
{
    this->type = type;
    on_evaluate = evaluate;
}

Syntax::Syntax(SyntaxTypes type, bool (*evaluate)(Syntax &, EasyParser &), Tokens token_type)
{
    this->type = type;
    on_evaluate = evaluate;
    read_token_data = {nullptr, token_type};
}

Syntax::Syntax(SyntaxTypes type, bool (*evaluate)(Syntax &, EasyParser &), Token *output, Tokens token_type)
{
    this->type = type;
    on_evaluate = evaluate;
    read_token_data = {output, token_type};
}

Syntax::Syntax(SyntaxTypes type, bool (*evaluate)(Syntax &, EasyParser &), void (*on_complete)(), Tokens token_type)
{
    this->type = type;
    on_evaluate = evaluate;
    this->on_complete = on_complete;
    read_token_data = {nullptr, token_type};
}

Syntax &Syntax::operator>>(void (*complete)())
{
    on_complete = complete;
    return *this;
}

bool on_token_evaluate(Syntax &current_syntax, EasyParser &parser)
{
    ReadTokenData data = current_syntax.read_token_data;
    Token current_token = parser.next_token();
    if (data.token_type != current_token.token)
        return false;
    if (data.token_location != nullptr)
        *data.token_location = current_token;
    parser.current_token_position++;
    return true;
}

Syntax::Syntax(Tokens token)
{
    this->type = read_token;
    on_evaluate = on_token_evaluate;
    read_token_data = {nullptr, token};
}

Syntax &Syntax::operator+=(Tokens adding_token)
{
    if (type != sequence)
    {
        Syntax toAdd = Syntax(*this);

        type = sequence;
        syntax_sequence.push_back(toAdd);
    }
    syntax_sequence.push_back(Syntax(read_token, on_token_evaluate, adding_token));
    return *this;
}

Syntax &Syntax::operator+=(Syntax adding_syntax)
{
    if (type != sequence)
    {
        Syntax toAdd = Syntax(*this);

        type = sequence;
        syntax_sequence.push_back(toAdd);
    }
    syntax_sequence.push_back(adding_syntax);
    return *this;
}
Syntax &Syntax::operator|=(Tokens adding_token)
{
    if (type != or_syntax)
    {
        Syntax toAdd = Syntax(*this);

        type = sequence;
        syntax_sequence.push_back(toAdd);
    }
    syntax_sequence.push_back(Syntax(read_token, on_token_evaluate, adding_token));
    return *this;
}

Syntax &Syntax::operator|=(Syntax adding_syntax)
{
    if (type != or_syntax)
    {
        Syntax toAdd = Syntax(*this);

        type = sequence;
        syntax_sequence.push_back(toAdd);
    }
    syntax_sequence.push_back(adding_syntax);
    return *this;
}

bool on_sequence_evaluate(Syntax &current_syntax, EasyParser &parser)
{
    if (current_syntax.syntax_sequence.size() == 0)
        return true;
    int start_location = parser.current_token_position;
    for (int offset = 0; offset < current_syntax.syntax_sequence.size(); offset++)
    {
        if (!current_syntax.syntax_sequence[offset].evaluate(parser))
        {
            parser.current_token_position = start_location;
            return false;
        }
    }
    if (current_syntax.on_complete != nullptr)
        current_syntax.on_complete();
    return true;
}

Syntax operator<<(Token &location, Tokens token)
{
    return Syntax(read_token, on_token_evaluate, &location, token);
}

Syntax operator>>(Tokens token, void (*on_complete)())
{
    return Syntax(read_token, on_token_evaluate, on_complete, token);
}

Syntax operator+(Tokens left_token, Tokens right_token)
{
    Syntax sequence_syntax = Syntax(sequence, on_sequence_evaluate);
    sequence_syntax += left_token;
    sequence_syntax += right_token;
    return sequence_syntax;
}

Syntax operator+(Syntax left_syntax, Tokens right_token)
{
    if (left_syntax.type == sequence)
    {
        left_syntax += right_token;
        return left_syntax;
    }

    Syntax sequence_syntax = Syntax(sequence, on_sequence_evaluate);
    sequence_syntax += left_syntax;
    sequence_syntax += right_token;
    return sequence_syntax;
}

Syntax operator+(Tokens left_token, Syntax right_syntax)
{
    if (right_syntax.type == sequence)
    {
        right_syntax.syntax_sequence.insert(right_syntax.syntax_sequence.begin(), left_token);
        return right_syntax;
    }
    Syntax sequence_syntax = Syntax(sequence, on_sequence_evaluate);
    sequence_syntax += left_token;
    sequence_syntax += right_syntax;
    return sequence_syntax;
}

Syntax operator+(Syntax left_syntax, Syntax right_syntax)
{
    if (left_syntax.type == sequence)
    {
        left_syntax += right_syntax;
        return left_syntax;
    }
    Syntax sequence_syntax = Syntax(sequence, on_sequence_evaluate);
    sequence_syntax += left_syntax;
    sequence_syntax += right_syntax;
    return sequence_syntax;
}

bool on_or_evaluate(Syntax &current_syntax, EasyParser &parser)
{
    if (current_syntax.syntax_sequence.size() == 0)
        return true;
    int start_location = parser.current_token_position;
    for (int offset = 0; offset < current_syntax.syntax_sequence.size(); offset++)
    {
       if (current_syntax.syntax_sequence[offset].evaluate(parser))
        {
            if (current_syntax.on_complete != nullptr)
                current_syntax.on_complete();
            return true;
        }
    }
    parser.current_token_position = start_location;
    return false;
}

Syntax operator|(Tokens left_token, Tokens right_token)
{
    Syntax new_or_syntax = Syntax(or_syntax, on_or_evaluate);
    new_or_syntax |= left_token;
    new_or_syntax |= right_token;
    return new_or_syntax;
}

Syntax operator|(Syntax left_syntax, Tokens right_token)
{
    if (left_syntax.type == or_syntax)
    {
        left_syntax |= right_token;
        return left_syntax;
    }

    Syntax new_or_syntax = Syntax(or_syntax, on_or_evaluate);
    new_or_syntax |= left_syntax;
    new_or_syntax |= right_token;
    return new_or_syntax;
}

Syntax operator|(Tokens left_token, Syntax right_syntax)
{
    if (right_syntax.type == or_syntax)
    {
        right_syntax.syntax_sequence.insert(right_syntax.syntax_sequence.begin(), right_syntax);
        return right_syntax;
    }
    Syntax new_or_syntax = Syntax(or_syntax, on_or_evaluate);
    new_or_syntax |= left_token;
    new_or_syntax |= right_syntax;
    return new_or_syntax;
}

Syntax operator|(Syntax left_syntax, Syntax right_syntax)
{
    if (left_syntax.type == or_syntax)
    {
        left_syntax |= right_syntax;
        return left_syntax;
    }
    Syntax new_or_syntax = Syntax(or_syntax, on_or_evaluate);
    new_or_syntax |= left_syntax;
    new_or_syntax |= right_syntax;
    return new_or_syntax;
}

bool on_any_evaluate(Syntax &current_syntax, EasyParser &parser)
{
    if (current_syntax.syntax_sequence.size() != 0)
    {
        int start_location = parser.current_token_position;
        if (on_sequence_evaluate(current_syntax, parser))
            on_any_evaluate(current_syntax, parser);
        else
            parser.current_token_position = start_location;
    }
    return true;
}

Syntax operator~(Tokens token)
{
    Syntax new_any_syntax = Syntax(any_syntax, on_any_evaluate);
    new_any_syntax.syntax_sequence.push_back(token);
    return new_any_syntax;
}

Syntax operator~(Syntax syntax)
{
    if (syntax.type == any_syntax)
        return syntax;
    Syntax new_any_syntax = Syntax(any_syntax, on_any_evaluate);
    new_any_syntax.syntax_sequence.push_back(syntax);
    return new_any_syntax;
}

bool on_maybe_evaluate(Syntax &current_syntax, EasyParser &parser)
{
    on_sequence_evaluate(current_syntax, parser);
    return true;
}

Syntax operator!(Tokens token)
{
    Syntax new_maybe_syntax = Syntax(maybe_syntax, on_maybe_evaluate);
    new_maybe_syntax.syntax_sequence.push_back(token);
    return new_maybe_syntax;
}

Syntax operator!(Syntax syntax)
{
    if (syntax.type == maybe_syntax)
        return syntax;
    Syntax new_maybe_syntax = Syntax(maybe_syntax, on_maybe_evaluate);
    new_maybe_syntax.syntax_sequence.push_back(syntax);
    return new_maybe_syntax;
}

bool Syntax::evaluate(EasyParser &parser)
{
    if (on_evaluate != nullptr)
        if (!on_evaluate(*this, parser))
            return false;

    return true;
}