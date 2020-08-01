#include "EasyParser.h"

SyntaxException::SyntaxException(Token error)
{
    error_token = error;
}

std::string generate_name(int value)
{
    if (value < 26)
        return std::string(1, 'a' + value);
    return generate_name((value / 26) - 1) + generate_name(value % 26);
}

std::string EasyParser::generate_syntax(ParseTree tree, bool deep = false)
{
    if (tree.type == nothing)
        return "ε";
    if (tree.type == terminal_token)
        return "<" + (token_names.find(tree.token_type) != token_names.end() ? token_names[tree.token_type] : std::to_string(tree.token_type)) + ">";
    if (tree.type == recursive)
        return "<" + function_names[tree.recursive_parsing] + ">";
    std::string value;
    if (deep)
        value += "(";
    for (int i = 0; i < tree.other_trees.size(); i++)
    {
        if (i != 0 && tree.type == or_tokens)
            value += "|";

        value += generate_syntax(tree.other_trees[i], true);
    }
    if (deep)
        value += ")";
    return value;
}

std::set<int> EasyParser::first_set(ParseTree tree)
{
    std::set<int> first;
    if (tree.type == terminal_token)
        first.insert(tree.token_type);
    if (tree.type == or_tokens)
    {
        for (int i = 0; i < tree.other_trees.size(); i++)
        {
            std::set<int> deep_set = first_set(tree.other_trees[i]);
            first.insert(deep_set.begin(), deep_set.end());
        }
    }

    if (tree.type == sequence)
    {
        std::set<int> deep_set = first_set(tree.other_trees[0]);
        first.insert(deep_set.begin(), deep_set.end());
    }

    if (tree.type == recursive)
        first.insert(first_sets[tree.recursive_parsing].begin(), first_sets[tree.recursive_parsing].end());

    if (tree.type == nothing)
    {
        first.insert(INT_MIN);
    }
    return first;
}

void EasyParser::follow_set(ParseTree (*tree_function)(), std::map<ParseTree (*)(), std::set<int>> &replacement)
{
    ParseTree tree = tree_function();
    if (tree.type == nothing || tree.type == terminal_token)
        return;
    for (int i = 0; i < tree.other_trees.size() - 1; i++)
    {
        ParseTree other = tree.other_trees[i];
        std::set<int> deep_set = tree.type == or_tokens ? follow_sets[tree_function] : first_set(tree.other_trees[i + 1]);

        if (deep_set.find(INT_MIN) != deep_set.end())
            deep_set.erase(deep_set.find(INT_MIN));

        if (other.type != recursive)
        {
            follow_set(other, deep_set, replacement);
            continue;
        }

        replacement[other.recursive_parsing].insert(deep_set.begin(), deep_set.end());
    }
    if (tree.other_trees.size() != 0)
    {
        ParseTree other = tree.other_trees[tree.other_trees.size() - 1];
        std::set<int> deep_set = follow_sets[tree_function];
        if (other.type == recursive)
        {
            replacement[other.recursive_parsing].insert(deep_set.begin(), deep_set.end());
        }
        else
        {
            follow_set(other, deep_set, replacement);
        }
    }
}

void EasyParser::follow_set(ParseTree tree, std::set<int> parent_follow, std::map<ParseTree (*)(), std::set<int>> &replacement)
{
    if (tree.type == nothing || tree.type == terminal_token)
        return;
    for (int i = 0; i < tree.other_trees.size() - 1; i++)
    {
        ParseTree other = tree.other_trees[i];
        std::set<int> deep_set = tree.type == or_tokens ? parent_follow : first_set(tree.other_trees[i + 1]);

        if (deep_set.find(INT_MIN) != deep_set.end())
            deep_set.erase(deep_set.find(INT_MIN));

        if (other.type != recursive)
        {
            follow_set(other, deep_set, replacement);
            continue;
        }
        replacement[other.recursive_parsing].insert(deep_set.begin(), deep_set.end());
    }
    if (tree.other_trees.size() != 0)
    {
        ParseTree other = tree.other_trees[tree.other_trees.size() - 1];
        std::set<int> deep_set = parent_follow;
        if (other.type == recursive)
            replacement[other.recursive_parsing].insert(deep_set.begin(), deep_set.end());
        else
            follow_set(other, deep_set, replacement);
    }
}

void EasyParser::generate_first_sets()
{
    first_sets.empty();
    std::map<ParseTree (*)(), std::string>::iterator names_iter = function_names.begin();
    std::map<ParseTree (*)(), std::string>::iterator names_end = function_names.end();

    for (; names_iter != names_end; names_iter++)
        first_sets.insert(std::pair<ParseTree (*)(), std::set<int>>(names_iter->first, std::set<int>()));
    std::map<ParseTree (*)(), std::set<int>> replacement_set;
    do
    {
        replacement_set = first_sets;

        std::map<ParseTree (*)(), std::set<int>>::iterator iter = replacement_set.begin();
        std::map<ParseTree (*)(), std::set<int>>::iterator end = replacement_set.end();

        for (; iter != end; iter++)
        {
            std::set<int> new_set = first_set(iter->first());
            iter->second.insert(new_set.begin(), new_set.end());
        }

        if (replacement_set == first_sets)
            break;
        first_sets = replacement_set;
    } while (true);
}

void EasyParser::generate_follow_sets()
{
    follow_sets.empty();
    std::map<ParseTree (*)(), std::string>::iterator names_iter = function_names.begin();
    std::map<ParseTree (*)(), std::string>::iterator names_end = function_names.end();
    for (; names_iter != names_end; names_iter++)
        follow_sets.insert(std::pair<ParseTree (*)(), std::set<int>>(names_iter->first, std::set<int>()));

    follow_sets[program].insert(INT_MIN);
    std::map<ParseTree (*)(), std::set<int>> replacement_set;
    do
    {
        replacement_set = follow_sets;

        std::map<ParseTree (*)(), std::set<int>>::iterator iter = replacement_set.begin();
        std::map<ParseTree (*)(), std::set<int>>::iterator end = replacement_set.end();

        for (; iter != end; iter++)
            follow_set(iter->first, replacement_set);

        if (replacement_set == follow_sets)
            break;
        follow_sets = replacement_set;
    } while (true);
}

void EasyParser::register_tree(ParseTree (*function)(), std::string name)
{
    function_names.insert(std::pair<ParseTree (*)(), std::string>(function, name));
}

void EasyParser::register_token_name(Tokens token, std::string name)
{
    token_names.insert(std::pair<Tokens, std::string>(token, name));
}

void EasyParser::parse(std::string program_string)
{
    std::vector<Token> tokens = EasyLexer::parse(program_string);
    remove_ignored_tokens(tokens);
    if (tokens.size() == 0)
        return;
    generate_first_sets();
    generate_follow_sets();
    if (first_sets[program].find(tokens[0].token) == first_sets[program].end())
        throw SyntaxException(tokens[0]);
    program().evaluate(tokens, 0, true, *this);
    program().evaluate(tokens, 0, false, *this);
}

void EasyParser::remove_ignored_tokens(std::vector<Token> &tokens)
{
    for (int i = tokens.size() - 1; i >= 0; i--)
        if (std::count(ignored_tokens.begin(), ignored_tokens.end(), tokens[i].token))
            tokens.erase(tokens.begin() + i);
}

void EasyParser::add_ignored_token(Tokens token)
{
    ignored_tokens.push_back(token);
}

ParseTree::ParseTree()
{
    type = nothing;
}

ParseTree::ParseTree(Tokens token)
{
    type = terminal_token;
    token_type = token;
}

ParseTree::ParseTree(ParseTree (*recursive_parse_function)())
{
    type = recursive;
    recursive_parsing = recursive_parse_function;
}

ParseTree::ParseTree(ParseTree first, ParseTree second)
{
    type = sequence;
    other_trees.push_back(first);
    other_trees.push_back(second);
}

ParseTree operator+(Tokens lhs, Tokens rhs)
{
    return ParseTree(lhs, rhs);
}

ParseTree operator+(Tokens lhs, ParseTree (*rhs)())
{
    return ParseTree(lhs, rhs);
}

ParseTree operator+(ParseTree (*lhs)(), Tokens rhs)
{
    return ParseTree(lhs, rhs);
}

ParseTree operator+(ParseTree lhs, ParseTree rhs)
{
    if (lhs.type == sequence)
    {
        lhs.other_trees.push_back(rhs);
        return lhs;
    }
    if (rhs.type == sequence)
    {
        rhs.other_trees.insert(rhs.other_trees.begin(), lhs);
        return rhs;
    }
    return ParseTree(lhs, rhs);
}

ParseTree operator|(Tokens lhs, Tokens rhs)
{
    ParseTree or_tree = ParseTree(lhs, rhs);
    or_tree.type = or_tokens;
    return or_tree;
}

ParseTree ParseTree::operator>>(void (*complete)())
{
    on_complete = complete;
    return *this;
}

ParseTree operator>>(Tokens token, void (*complete)())
{
    ParseTree tree = token;
    tree = tree >> complete;
    return tree;
}

ParseTree operator<<(Token &location, Tokens token)
{
    ParseTree tree = token;
    tree.output_location = &location;
    return tree;
}
ParseTree operator|(ParseTree lhs, ParseTree rhs)
{
    if (lhs.type == or_tokens)
    {
        lhs.other_trees.push_back(rhs);
        return lhs;
    }
    if (rhs.type == or_tokens)
    {
        rhs.other_trees.insert(rhs.other_trees.begin(), lhs);
        return rhs;
    }
    ParseTree or_tree = ParseTree(lhs, rhs);
    or_tree.type = or_tokens;
    return or_tree;
}

int ParseTree::evaluate(std::vector<Token> tokens, int offset, bool testing, EasyParser parser)
{
    if (offset >= tokens.size())
        return offset;
    switch (type)
    {
    case terminal_token:
    {
        Token next_token = tokens[offset++];
        if (testing && output_location != nullptr)
            *output_location = next_token;
    }
    break;
    case sequence:
    {
        for (int i = 0; i < other_trees.size(); i++)
        {
            Token next_token = tokens[offset];
            std::set<int> first = parser.first_set(other_trees[i]);
            if (first.find(next_token.token) != first.end())
                offset = other_trees[i].evaluate(tokens, offset, testing, parser);
            else if (first.find(INT_MIN) == first.end())
                throw SyntaxException(next_token);
        }
    }
    break;
    case or_tokens:
    {
        bool called = false;
        Token next_token = tokens[offset];
        for (int i = 0; i < other_trees.size() && !called; i++)
        {
            std::set<int> first = parser.first_set(other_trees[i]);
            if (first.find(next_token.token) != first.end())
            {
                offset = other_trees[i].evaluate(tokens, offset, testing, parser);
                called = true;
            }
            else if (first.find(INT_MIN) != first.end())
                called = true;
        }
        if (!called)
            throw SyntaxException(next_token);
    }
    break;

    case recursive:
        offset = recursive_parsing().evaluate(tokens, offset, testing, parser);
    }

    if (testing && on_complete != nullptr)
        on_complete();
    return offset;
}

/**
 std::map<ParseTree (*)(), std::set<int>>::iterator iter = first_sets.begin();
    std::map<ParseTree (*)(), std::set<int>>::iterator end = first_sets.end();

    printf("First sets:\n");
    for (; iter != end; iter++)
    {
        printf("%s:{", function_names[iter->first].c_str());
        int i = 0;
        for (int token : iter->second)
        {
            printf("%s", token == INT_MIN ? "ε" : token_names[(Tokens)token].c_str());
            if (i++ != iter->second.size() - 1)
                printf(",");
        }
        printf("}\n");
    }

    std::map<ParseTree (*)(), std::set<int>>::iterator follow_iter = follow_sets.begin();
    std::map<ParseTree (*)(), std::set<int>>::iterator follow_end = follow_sets.end();

    printf("\nFollow sets:\n");
    for (; follow_iter != follow_end; follow_iter++)
    {
        printf("%s:{", function_names[follow_iter->first].c_str());
        int i = 0;
        for (int token : follow_iter->second)
        {
            printf("%s", token == INT_MIN ? "$" : token_names[(Tokens)token].c_str());
            if (i++ != follow_iter->second.size() - 1)
                printf(",");
        }
        printf("}\n");
    }
    std::map<std::string, std::string> parse_rules;

    printf("\nGrammar:\n");
    std::map<ParseTree (*)(), std::string>::iterator names_iter = function_names.begin();
    std::map<ParseTree (*)(), std::string>::iterator names_end = function_names.end();

    for (; names_iter != names_end; names_iter++)
        parse_rules.insert(std::pair<std::string, std::string>(names_iter->second, generate_syntax(names_iter->first())));

    std::map<std::string, std::string>::iterator _iter = parse_rules.begin();
    std::map<std::string, std::string>::iterator _end = parse_rules.end();
    for (; _iter != _end; _iter++)
        printf("<%s> ::= %s\n", _iter->first.c_str(), _iter->second.c_str());

    printf("\n\nProgram:\n");
    for (int i = 0; i < tokens.size(); i++)
    {
        if (i != 0)
            printf(", ");
        printf("%s", token_names[tokens[i].token].c_str());
    }
    printf("\n\n");
 */