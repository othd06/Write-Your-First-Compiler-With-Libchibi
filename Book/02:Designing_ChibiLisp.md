<h1 align=center>Chapter 02: Designing ChibiLisp</h1>

In the last chapter we learned how to use the EBNF notation to describe context-free grammars. Now comes the fun part. Using that knowledge to start designing our own language: ChibiLisp. 

Just a note, this chapter is going to be a bit more dense than the last two. Don't worry; you've got it. I believe in you. Just take it slowly, and don't be afraid to read something again if it doesn't click the first time.

As you may know already the heart of any lisp-family language is the S-expression, a type of nestable list enclosed in parenthesis. This will be no different for our language either. Let's take a look at the most basic program in our language, a program that exits with a custom error code (in this case 69), so we can start to think about how we might define it.
```lisp
(define proc (main (i32 ()) (body (return (i32 69)))))
```
This is the form that would be most idiomatic to write in however, since I imagine that most readers will likely not have read or used lisp before (and markdown doesn't support bracket colouring) let's lay it out on different lines to show how it works in a more C-like fashion:
```lisp
(define proc 
    (main (i32 ()) 
        (body 
            (return (i32 69))
        )
    )
)
```
As we can see the outer list represents an object (not in the OOP sense). In this case, it is a function definition. We see that the first keyword define is used to signify the kind of object which is a definition and the second signifies that it is a procedure definition (since the lisp family is so closely associated with functional programming we will reserve the name "function" for so-called pure functions which guarantee they will not change anything outside of the function's own scope, a feature we will introduce later in the book). The final element of the outer list is the function itself. We can see that a function is a list itself that comprises 3 elements, the name of the function, the type of the function (itself a nested list containing the i32 return type and an empty list of parameters) and the body of the function which comprises the word body followed by a list of statements, in this case a single return statement.

We can start to think about how we might define this particular statement in EBNF. I would propose we do something like the following:
```ebnf
<program> ::= (<object> <whitespace>)*;

<object> ::= <definition>;

<definition> ::= <proc_definition>;

<proc_definition> ::= <l_paren> <whitespace> <keyword_define>
                <whitespace> <keyword_proc> <whitespace>
                <proc_definition_inner> <r_paren>;

<proc_definition_inner> ::= <l_paren> <whitespace> <identifier>
                      <whitespace> <proc_type> <whitespace>
                      <proc_body> <whitespace> <r_paren>;

<proc_body> ::= <l_paren> <whitespace> <keyword_body>
                <whitespace> (<statement> <whitespace>)*
                <r_paren>;

<statement> ::= <return_statement>

<return_statement> ::= <l_paren> <whitespace> <keyword_return>
                <whitespace> <value_literal> <whitespace>
                <r_paren>;

<type> ::= <proc_type> | <base_type>;

<proc_type> ::= <l_paren> <whitespace> <type> <whitespace>
                <l_paren> <whitespace> (<declaration> <whitespace>)*
                <r_paren> <whitespace> <r_paren>;

<base_type> ::= <keyword_bool> | <keyword_i8> | <keyword_i16> |
                <keyword_i32> | <keyword_i64> | <keyword_u8> |
                <keyword_u16> | <keyword_u32> | <keyword_u64> |
                <keyword_f32> | <keyword_f64> | <keyword_f80> |
                <keyword_string>;

<declaration> ::= <l_paren> <whitespace> (<keyword_i8> | <keyword_i16>
                  | <keyword_i32> | <keyword_i64> | <keyword_u8> |
                  <keyword_u16> | <keyword_u32> | <keyword_u64>)
                  (<whitespace> <identifier>) <whitespace> <r_paren>;
<declaration> ::= <l_paren> <whitespace>
                  (<keyword_f32> | <keyword_f64> | <keyword_f80>)
                  (<whitespace>  <identifier>) <whitespace>
                  <r_paren>;
<declaration> ::= <l_paren> <whitespace> <keyword_bool>
                  (<whitespace> <identifier>)
                  <whitespace> <r_paren>;
<declaration> ::= <l_paren> <whitespace> <keyword_string>
                  (<whitespace> <identifier>)
                  <whitespace> <r_paren>;

<value_literal> ::= <l_paren> <whitespace> (<keyword_i8> | <keyword_i16>
                    | <keyword_i32> | <keyword_i64> | <keyword_u8> |
                    <keyword_u16> | <keyword_u32> | <keyword_u64>)
                    (<whitespace> <integer_literal>) <whitespace> <r_paren>;
<value_literal> ::= <l_paren> <whitespace>
                    (<keyword_f32> | <keyword_f64> | <keyword_f80>)
                    (<whitespace>  (<integer_literal> | <decimal_literal>))
                    <whitespace> <r_paren>;
<value_literal> ::= <l_paren> <whitespace> <keyword_bool>
                    (<whitespace> <bool_literal>)
                    <whitespace> <r_paren>;
<value_literal> ::= <l_paren> <whitespace> <keyword_string>
                    (<whitespace> <string_literal>)
                    <whitespace> <r_paren>;

=====================================================================

<l_paren> ::= "(";
<r_paren> ::= ")";

<keyword_define> ::= "define";
<keyword_proc> ::= "proc";
<keyword_body> ::= "body";
<keyword_return> ::= "return";

<keyword_bool> ::= "bool";
<keyword_i8>  ::= "i8";
<keyword_i16> ::= "i16";
<keyword_i32> ::= "i32";
<keyword_i64> ::= "i64";
<keyword_u8>  ::= "u8";
<keyword_u16> ::= "u16";
<keyword_u32> ::= "u32";
<keyword_u64> ::= "u64";
<keyword_f32> ::= "f32";
<keyword_f64> ::= "f64";
<keyword_f80> ::= "f80";
<keyword_string> ::= "string";

<identifier> ::= <letter> (<letter> | <digit> | "_")*;

<integer_literal> ::= <digit>+;
<decimal_literal> ::= <digit>+ "." <digit>+;
<bool_literal> ::= "true" | "false";
<string_literal> ::= "\"" "{any non-quote terminal}"* "\"";

<letter> ::= "A" | "B" | "C" | "D" | "E" | "F" | "G" |
             "H" | "I" | "J" | "K" | "L" | "M" | "N" | "O" | 
             "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" |
             "X" | "Y" | "Z" | "a" | "b" | "c" | "d" | "e" |
             "f" | "g" | "h" | "i" | "j" | "k" | "l" | "m" |
             "n" | "o" | "p" | "q" | "r" | "s" | "t" | "u" |
             "v" | "w" | "x" | "y" | "z";
<digit> ::= "0" | "1" | "2" | "3" | "4" |
            "5" | "6" | "7" | "8" | "9";

<whitespace> ::= (" " | "\t" | "\n" | "\r")*;

```
Now, that's a lot. But you should be able to have a good go at understanding it after chapter 1. Take a minute; it's not easy, but I believe in you. If it feels overwhelming don't worry, try to get the gist of it and you'll find after we refer back to it in later chapters to concretely implement it that actually it will feel a lot easier. (Also, there is actually a slightly easier version just below if you read on). I should add that there is one informal shorthand in the rule for strings; this will, of course, be handled formally in the tokeniser.

Take note of line down the middle. Notice how I don't use any terminals above the line? That's because, if you think back to the stages of a compiler in chapter 0, the non-terminals under the line represent the tokens the tokeniser will produce, and the tokens above the line represent nodes of the AST the parser will produce. This means that before we try to parse this simple program we will first turn it into a series of tokens that might look something like this:
```
[l_paren] [keyword_define] [keyword_proc]
    [l_paren] [identifier: main] [l_paren] [keyword_i32] [l_paren] [r_paren] [r_paren]
        [l_paren] [keyword_body] 
            [l_paren] [keyword_return] [l_paren] [keyword_i32] [integer_literal: 69] [r_paren] [r_paren]
        [r_paren]
    [r_paren]
[r_paren]
```
Then we will parse these tokens to create the AST in the parser stage. At this point our tokens, the non-terminal symbols below the line, effectively become the terminals for the parser above the line with the output of the tokeniser _becoming_ the input string of the parser. In fact, since we do our parsing in two stages like this, we can simply not emit any whitespace tokens from the tokeniser meaning we can simplify the parser to something like this: 
```ebnf
<program> ::= <object>*;

<object> ::= <definition>;

<definition> ::= <proc_definition>;

<proc_definition> ::= <l_paren> <keyword_define> <keyword_proc>
                 <proc_definition_inner> <r_paren>;

<proc_definition_inner> ::= <l_paren> <identifier> <proc_type>
                      <proc_body> <r_paren>;

<proc_body> ::= <l_paren> <keyword_body> <statement>*
                <r_paren>;

<statement> ::= <return_statement>

<return_statement> ::= <l_paren> <keyword_return> <value_literal>
                <r_paren>;

<type> ::= <proc_type> | <base_type>;

<proc_type> ::= <l_paren> <type> <l_paren> <declaration>*
                <r_paren> <r_paren>;

<base_type> ::= <keyword_bool> | <keyword_i8> | <keyword_i16> |
                <keyword_i32> | <keyword_i64> | <keyword_u8> |
                <keyword_u16> | <keyword_u32> | <keyword_u64> |
                <keyword_f32> | <keyword_f64> | <keyword_f80> |
                <keyword_string>;

<declaration> ::= <l_paren> (<keyword_i8> | <keyword_i16>
                  | <keyword_i32> | <keyword_i64> | <keyword_u8> |
                  <keyword_u16> | <keyword_u32> | <keyword_u64>)
                  <identifier> <r_paren>;
<declaration> ::= <l_paren> (<keyword_f32> | <keyword_f64> |
                  <keyword_f80>) <identifier> <r_paren>;
<declaration> ::= <l_paren> <keyword_bool> <identifier>
                  <r_paren>;
<declaration> ::= <l_paren> <keyword_string> <identifier>
                  <r_paren>;

<value_literal> ::= <l_paren> (<keyword_i8> | <keyword_i16>
                  | <keyword_i32> | <keyword_i64> | <keyword_u8> |
                  <keyword_u16> | <keyword_u32> | <keyword_u64>)
                  <integer_literal> <r_paren>;
<value_literal> ::= <l_paren> (<keyword_f32> |
                    <keyword_f64> | <keyword_f80>)
                   (<integer_literal> | <decimal_literal>)
                   <r_paren>;
<value_literal> ::= <l_paren> <keyword_bool> <bool_literal>
                    <r_paren>;
<value_literal> ::= <l_paren> <keyword_string>
                    <string_literal> <r_paren>;

=====================================================================

<l_paren> ::= "(";
<r_paren> ::= ")";

<keyword_define> ::= "define";
<keyword_proc> ::= "proc";
<keyword_body> ::= "body";
<keyword_return> ::= "return";

<keyword_bool> ::= "bool";
<keyword_i8>  ::= "i8";
<keyword_i16> ::= "i16";
<keyword_i32> ::= "i32";
<keyword_i64> ::= "i64";
<keyword_u8>  ::= "u8";
<keyword_u16> ::= "u16";
<keyword_u32> ::= "u32";
<keyword_u64> ::= "u64";
<keyword_f32> ::= "f32";
<keyword_f64> ::= "f64";
<keyword_f80> ::= "f80";
<keyword_string> ::= "string";

<identifier> ::= <letter> (<letter> | <digit> | "_")*;

<integer_literal> ::= <digit>+;
<decimal_literal> ::= <digit>+ "." <digit>+;
<bool_literal> ::= "true" | "false";
<string_literal> ::= "\"" "{any non-quote terminal}"* "\"";

<letter> ::= "A" | "B" | "C" | "D" | "E" | "F" | "G" |
             "H" | "I" | "J" | "K" | "L" | "M" | "N" | "O" | 
             "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" |
             "X" | "Y" | "Z" | "a" | "b" | "c" | "d" | "e" |
             "f" | "g" | "h" | "i" | "j" | "k" | "l" | "m" |
             "n" | "o" | "p" | "q" | "r" | "s" | "t" | "u" |
             "v" | "w" | "x" | "y" | "z";
<digit> ::= "0" | "1" | "2" | "3" | "4" |
            "5" | "6" | "7" | "8" | "9";

<whitespace> ::= (" " | "\t" | "\n" | "\r")*;

```
That's a fair bit more manageable. Note, we obviously still consume whitespace in the tokeniser as if we were going to emit the whitespace symbols as tokens, we simply either emit no tokens or strip every whitespace token emitted before passing to the parser.

This isn't the only advantage of tokenisation. Tokenisation can also play a role in making our grammar as close to context-free as possible as well. Remember how I said that [Python](https://www.python.org/)'s indentation-based scoping isn't strictly context free? Well we can make it context-free using the tokeniser by, instead of emitting a token for every level of indentation at the start of the line, only emitting a token every time the indentation increases or decreases. In fact, such a system would make those tokens identical to the ones representing, say, curly braces in C-like languages. It also allows us to make decisions between keywords and mere identifiers which would otherwise be ambiguous, or to evaluate numeric and decimal literals so the parser can see their value as a number rather than simply a string of digits. So you can see how as the language gets more complex it becomes really powerful to have basic tokens representing small bits of syntactic meaning that are generated from the plaintext and can effectively act as the terminals for our more complex parser.

Also note how the grammar symbols in the top half of the grammar here are defined in quite a specific way. This is to make our syntactic grammar match as closely as possible to the semantics, our AST, of the language as possible. (If it's not obvious to you how this works now, again, don't worry; that's natural if you're not used to working with ASTs, especially since I haven't yet introduced the actual node types we will be using.)

You may remember from the example in chapter 1 how I said we wouldn't typically define the grammar of an equation the way I did there. This is exactly why, because the operators '^', '*' and '/', and '+' and '-' each have different precedence levels (remember BIDMAS) so instead of defining the grammar such that they're all just operators we would typically define it so that the types of operators match up with the precedence levels. Then we could define the expression as any number of exponentiations between any number of multiplications/divisions between any number of additions/subtractions between any number of unaries.

If you struggled to follow there don't worry; you'll get a more concrete understanding as we go through the process of parsing this grammar and you can see for yourself how the non-terminals we defined line up very nicely with the nodes of our AST-to-be. This is something you'll get somewhat of a feel for by doing it and, when it comes to designing your own languages, just trying stuff out.

In the next chapter, however, we will be focussing on writing our tokeniser to turn a plaintext input file into a stream of tokens that we can hand to the parser.
