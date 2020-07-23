#ifndef EasyParser_H
#define EasyParser_H
#include "EasyLexer.h"
#include <set>

enum Tokens : int;

class Syntax;

enum SyntaxTypes
{
    not_set,
    read_token,
    sequence,
    or_syntax,
    any_syntax,
    maybe_syntax
};

struct Comment
{
    int end_token;
    bool consume_last_token;
};

struct ReadTokenData
{
    Token *token_location;
    Tokens token_type;
};

class EasyParser
{
private:
    //Stores a lexer in the parser to extract tokens from the string
    EasyLexer lexer;

    //Stores the list of tokens extracted from the string
    std::vector<Token> extracted_tokens;

    //Stores the tokens that should be ignored
    std::set<int> ignored_tokens;

    //Stores the comment rules
    std::map<int, Comment> comment_rules;

    //Extracts the tokens from the string, storing them in the queue
    void extract(std::string read_string);

public:
    int current_token_position = 0;
    //Add a new valid token to the set of valid tokens. token- the token type, regex - the regex that defines the token
    void add_new_token(Tokens token, std::string regex);

    //Add a token that will be ignored by the parser. token - the token to ignore
    void add_ignored_token(Tokens token);

    //Add a rule that will ignore any tokens in between 2 tokens. start - the token at the start of the comment, end - the token at the end of a comment, consume - should the end token be used
    void add_comment_rule(Tokens start, Tokens end, bool consume);

    //Set what token type declares end of field
    void set_end_of_field_token(Tokens end_of_field);

    //Set up a new string to be processed. read_string - the string to be processed
    void parse(std::string read_string, Syntax syntax);

    //Gets the next token to be analysis
    Token next_token();

    //Gets the next token from current token position shifted by the offset
    Token token_after(int offset);
};

class Syntax
{
private:
    bool (*on_evaluate)(Syntax &, EasyParser &) = nullptr;

public:
    ReadTokenData read_token_data;
    // std::pair<Syntax,Syntax> or_pair;
    std::vector<Syntax> syntax_sequence;
    void (*on_complete)() = nullptr;
    SyntaxTypes type = not_set;
    Syntax();
    Syntax(Tokens token);
    Syntax(SyntaxTypes type) { this->type = type; };
    Syntax(SyntaxTypes type, bool (*evaluate)(Syntax &, EasyParser &));
    Syntax(SyntaxTypes type, bool (*evaluate)(Syntax &, EasyParser &), Tokens token_type);
    Syntax(SyntaxTypes type, bool (*evaluate)(Syntax &, EasyParser &), Token *output, Tokens token_type);
    Syntax(SyntaxTypes type, bool (*evaluate)(Syntax &, EasyParser &), void (*on_complete)(), Tokens token_type);
    Syntax &operator>>(void (*on_complete)());
    Syntax &operator+=(Tokens adding_token);
    Syntax &operator+=(Syntax adding_syntax);
    Syntax &operator|=(Tokens adding_token);
    Syntax &operator|=(Syntax adding_syntax);

    bool evaluate(EasyParser &parser);
};

Syntax operator<<(Token &token_location, Tokens token_type);
Syntax operator>>(Tokens token_type, void (*on_complete)());
Syntax operator+(Tokens left_token, Tokens right_token);
Syntax operator+(Syntax left_syntax, Tokens right_token);
Syntax operator+(Tokens left_token, Syntax right_syntax);
Syntax operator+(Syntax left_syntax, Syntax right_syntax);
Syntax operator|(Tokens left_token, Tokens right_token);
Syntax operator|(Syntax left_syntax, Tokens right_token);
Syntax operator|(Tokens left_token, Syntax right_syntax);
Syntax operator|(Syntax left_syntax, Syntax right_syntax);
Syntax operator~(Syntax syntax);
Syntax operator~(Tokens token);
Syntax operator!(Syntax syntax);
Syntax operator!(Tokens token);
#endif
