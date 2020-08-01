# EasyParser

## Requirements
This requires [EasyLexer](https://github.com/Thespyinthehole/EasyLexer) to work. You will need to change the location of your easy lexer files in `EasyParser.h` include


## Introduction
This project is a generic parser library for LL(1) grammars. It is made using JavaCC as inspiration and was orginally developed in order to assist the generation of LLVM. The parser is a 3 pass parser, 1 for lexical analysis, 1 for syntax checking and finally 1 for running over the input string.

## How To Use
This will use [Example.cpp](https://github.com/Thespyinthehole/EasyParser/blob/master/Example.cpp) to show how to use this library. 

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
 
You will also need to define a token for the end of the program, `parser.end_of_field_token = token_end_of_field;`, you do not need to bind this token to regex as it is done automatically by the parser.

For more information, check out [EasyLexer](https://github.com/Thespyinthehole/EasyLexer) as this step is just to set up the lexer.

### Defining the grammar
The grammar is a collection of tokens which tells the parser the structure of the input you wish to process. The grammar is defined through the `ParseTree` class, however with the help of operators, the process has been streamlined in a way that you should not need to worry about making a `ParseTree` object yourself. To start making a grammer, make a function which takes no parameters and returns a `ParseTree`.

#### Sequences
A sequence is a list of tokens that need to appear in the same order as the sequence. By using the `+` operator, you can make a sequence of tokens.

For example:
```c
ParseTree program(){
    return token_variable_name + token_end_of_field;
}
```

This will mean that a variable name needs to be followed by a end of field.

You can also add `ParseTree` objects to the sequence in the same fashion. However, when using a function, do not call the function and instead just put the function name, this helps both with the parser being able to run but also allows the use of recursive grammar without causing an infinite loop.

For example:
```c
ParseTree variable(){
    return token_variable_name;
}

ParseTree program(){
    return variable + token_end_of_field;
}
```

Note:
If you are having compiler issues when you add 2 `ParseTree` functions together, this is because you cannot make an operator using both those types of variables. To fix this simply replace one of the function names with a `ParseTree` constructor with the function passed to it.

#### Or
If the grammar only allowed just sequences it would be a very limited grammar, therefore you can also use or operators in order to branch the grammar. This is used the same way as and but with the `|` operator instead. The or operator has left priority so it will try to run the furtherest left first.

Example:

```c
ParseTree operation(){
    return token_add | token_divide | token_multiply | token_subtract;
}
```

#### Epsilon

## Limitations
- The parser will only report the first error that it finds with the given input.  
- The parser no longer handles comment tokens.
