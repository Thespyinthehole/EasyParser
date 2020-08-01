# EasyParser

## Requirements
This requires [EasyLexer](https://github.com/Thespyinthehole/EasyLexer) to work. You will need to change the location of your easy lexer files in `EasyParser.h` include


## Introduction
This project is a generic parser library for LL(1) grammars. It is made using JavaCC as inspiration and was orginally developed in order to assist the generation of LLVM. The parser is a 3 pass parser, 1 for lexical analysis, 1 for syntax checking and finally 1 for running over the input string.

## How To Use
To use the parser it needs to know a few things beforehand, tokens, grammar and the data to parse.

### Defining Tokens
A token is a sequence of characters which is defined by regular expressions. These are extracted the the lexical analysis phase using the [EasyLexer](https://github.com/Thespyinthehole/EasyLexer) library. The method to add tokens to the EasyParser object is the same as adding to the EasyLexer object. First, you need to define a list of tokens which is made by having an enum named `Tokens` of the `int` type. You can then add your token types to this enum, the `int` value of the token determines the priority of extraction during lexical analysis, the only integer you should not use for a token is `INT_MIN`.

Example:
```c
enum Tokens : int {
    token_add = 1,
    token_subtract = 2,
    token_multiply = 3,
    token_divide = 4,
    token_variable_name = -1,
};
```

Now we need to bind the tokens to a regular expression that will be able to extract this. This is done by calling the `add_new_token` on a parser object which takes a token from the enum and a regular expression string.

Examples:
```c
parser.add_new_token(token_variable_name, "[a-zA-Z_]+");
parser.add_new_token(token_multiply, "\\*");
```

Notice how the `*` character has to be escaped twice as it is a regular expression function character and therefore requires it to be escaped. The priority of the token is used in reference to the regular expressions, try to have broader regular expressions at a lower priority. For example, token_variable_name will match any length of alphabetical characters, so if you want to have a token which is alphabetical characters, it will need a higher priority.
 
For more information, check out [EasyLexer](https://github.com/Thespyinthehole/EasyLexer) as this step is just to set up the lexer.

## Limitations
- The parser will only report the first error that it finds with the given input.  
- The parser no longer handles comment tokens.
