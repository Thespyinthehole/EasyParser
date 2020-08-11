#ifndef EasyParser_H
#define EasyParser_H
#include "EasyLexer/EasyLexer.h"
#include <algorithm>
#include <set>

enum ParseTreeType
{
    terminal_token,
    sequence,
    or_tokens,
    recursive,
    nothing
};

class SyntaxException : public std::exception
{
public:
    Token error_token;
    SyntaxException(Token);
};

class EasyParser;

class ParseTree
{
private:
    void (*on_complete)() = nullptr;

public:
    Tokens token_type;
    Token *output_location = nullptr;
    ParseTreeType type;
    ParseTree (*recursive_parsing)() = nullptr;
    std::vector<ParseTree> other_trees;
    int evaluate(std::vector<Token>&, int, bool, EasyParser&);
    ParseTree();
    ParseTree(Tokens);
    ParseTree(ParseTree (*)());
    ParseTree(ParseTree, ParseTree);
    ParseTree operator>>(void (*)());
};

ParseTree operator+(ParseTree, ParseTree);
ParseTree operator+(Tokens, Tokens);
ParseTree operator+(Tokens, ParseTree (*)());
ParseTree operator+(ParseTree (*)(), Tokens);
ParseTree operator|(ParseTree, ParseTree);
ParseTree operator|(Tokens, Tokens);
ParseTree operator>>(Tokens, void (*)());
ParseTree operator<<(Token &, Tokens);

class EasyParser : public EasyLexer
{
private:
    std::map<ParseTree (*)(), std::string> function_names;
    std::string generate_syntax(ParseTree, bool);
    std::map<ParseTree (*)(), std::set<int>> first_sets;
    std::map<ParseTree (*)(), std::set<int>> follow_sets;
    void generate_first_sets();
    void generate_follow_sets();
    void follow_set(ParseTree (*)(), std::map<ParseTree (*)(), std::set<int>> &);
    void follow_set(ParseTree, std::set<int>, std::map<ParseTree (*)(), std::set<int>> &);

public:
    std::map<Tokens, std::string> token_names;
    std::map<ParseTree (*)(), ParseTree> recursive_trees;
    std::set<int> first_set(ParseTree);
    ParseTree (*program)();
    std::string get_grammar();
    void parse(std::string);
    void register_tree(ParseTree (*)(), std::string);
    void register_token_name(Tokens, std::string);
};

#define epsilon ParseTree()
#endif
