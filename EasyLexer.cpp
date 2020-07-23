#include "EasyLexer.h"

Token::Token()
{
    //This token is the end of the processing
    hasNext = false;
}

Token::Token(int _token)
{
    //This token still has stuff to process
    hasNext = true;
    //Set the token type for this token
    token = _token;
}

Token::Token(int _token, std::string value)
{
    //This token still has stuff to process
    hasNext = true;
    //Set the token type for this token
    token = _token;
    //Set the value
    this->value = value;
}

void EasyLexer::add_new_token(int token, std::string regex)
{
    //Add a new valid token to the map, turning the regex string into a regex object
    tokens.insert(std::pair<int, std::regex>(token, std::regex(regex)));
}

std::list<Token> EasyLexer::parse(std::string read_string)
{
    //Set the new string
    string_to_analysis = read_string;

    //Reset the offset
    current_char_location = 0;

    std::list<Token> tokens;

    Token token;
    while ((token = next_token()).hasNext)
        tokens.push_back(token);

    tokens.push_back(Token(end_of_field_token));
    return tokens;
}

Token EasyLexer::next_token()
{
    //If there is a blank string do not process
    if (string_to_analysis.empty())
        return Token();

    //If we have reached the end do not process
    if (current_char_location > string_to_analysis.size())
        return Token();

    //Get the iterator for the map of valid tokens
    std::map<int, std::regex>::iterator iter;
    //Loop over each valid token
    for (iter = tokens.begin(); iter != tokens.end(); iter++)
    {
        //How many characters should we read
        int offset = 1;
        //Has this token matched any characters yet
        bool found = false;
        //The current search string
        std::string current_analysis = string_to_analysis.substr(current_char_location, offset);

        //Loop over the rest of the characters
        while (current_char_location + offset <= string_to_analysis.size())
        {
            //If we have a match already, keep searching until it doesnt match
            if (found)
            {
                if (!std::regex_match(current_analysis, iter->second))
                {
                    current_analysis = string_to_analysis.substr(current_char_location, --offset);
                    break;
                }
            }
            //If we just found a match note it
            else if (std::regex_match(current_analysis, iter->second))
                found = true;

            //Update the current search string
            current_analysis = string_to_analysis.substr(current_char_location, ++offset);
        }

        //If a match has been found return a token object with the correct type and value.
        if (found)
        {
            //If you reached the second to last character it kinda messes up, but this solves it
            if ((current_char_location + offset) == string_to_analysis.size())
                current_analysis = string_to_analysis.substr(current_char_location, --offset);

            current_char_location += offset;
            Token token = Token(iter->first, current_analysis);
            return token;
        }
    }
    return Token();
}