<h1 align=center>Chapter 02: Designing ChibiLisp</h1>

### Overview:

In the last chapter we learned how to use the EBNF notation to describe context-free grammars. Now comes the fun part. Using that knowledge to start designing our own language: ChibiLisp.

We will walk through the process of actually designing this in a way that you can hopefully repeat to do this yourself when you want to go off and build your own language later on. This is because, in many ways, designing a language is a craft and in order to teach you how to do it effectively I need to not just show you the tools (in this case EBNF notation) but also walk you through the process of how you actually use them.

### Syntax:

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
As we can see the outer list represents an object (not in the OOP sense). In this case, it is a function definition. We see that the first keyword define is used to signify the kind of object which is a definition and the second signifies that it is a procedure definition (the choice of this keyword rather than, say, func is something we will return to in chapter 9). The final element of the outer list is the function itself. We can see that a function is a list itself that comprises 3 elements, the name of the function, the type of the function (itself a nested list containing the i32 return type and an empty list of parameters) and the body of the function which comprises the word body followed by a list of statements, in this case a single return statement.

Obviously there is a lot more syntax we want to add and we will be doing so throughout the vast majority of the book but this syntax gives us a syntactic core to test our first program and build on going forward.

### Designing Our Tokens:

If we think back to our pipeline in chapter 0 the first stage of that is tokenisation. We can think about what sort of information we might want to be able to extract from the text of this basic function. Obviously we want our opening and closing parens to be individual tokens. We also want the number 69 to be it's own token that stores the literal value 69, and I'm going to suggest we have 4 types of literals:
  *  Integer Literals (whole numbers)
  *  Decimal Literals (decimal numbers)
  *  Boolean Literals (true or false)
  *  String Literals (text enclosed in quotes)

I'm also going to suggest that we treat everything else here as a keyword. This means we'll have a separate token for each word instead of storing the word itself as a value within the token like we store the number inside our integer literal tokens. I'm also going to suggest we create keywords now for all our other base types that we can see we will obviously want later. We do, however, need a token type for regular words, which we will call 'Identifiers', as well because we need to store variable and function names.

This gives us a list of the kinds of tokens we want to produce:
  *  Identifiers that contain the string of the identifier
  *  Left Parens
  *  Right Parens
  *  A Define Keyword
  *  A Proc Keyword
  *  A Main Keyword
  *  A Body Keyword
  *  A Return Keyword
  *  An i8 Keyword
  *  An i16 Keyword
  *  An i32 keyword
  *  An i64 Keyword
  *  A u8 Keyword
  *  A u16 Keyword
  *  A u32 Keyword
  *  A u64 Keyword
  *  An f32 Keyword
  *  An f64 Keyword
  *  An f80 Keyword
  *  A Bool Keyword
  *  A String Keyword
  *  Integer Literals which contain the value of that integer
  *  Decimal Literals which contain the value of that decimal
  *  Boolean Literals which contain the value of that boolean
  *  String Literals which contain the value of that string

Using these tokens, our example from before would look like this:
```
[l_paren] [keyword_define] [keyword_proc]
    [l_paren] [identifier: main] [l_paren] [keyword_i32] [l_paren] [r_paren] [r_paren]
        [l_paren] [keyword_body] 
            [l_paren] [keyword_return] [l_paren] [keyword_i32] [integer_literal: 69] [r_paren] [r_paren]
        [r_paren]
    [r_paren]
[r_paren]
```

### Writing Our Token Grammar:

We can express our tokens in terms of a grammar like we learned last chapter. Admittedly, this is perhaps more of a formality than would be the grammar describing our program at a higher level but it's both useful practice for writing the rest of our grammar and still genuinely helpful for our design.

The parens and keyword tokens are the easiest to express that they are effectively just terminals although we create non-terminal symbols for them to represent that this is the output of our tokeniser.
```EBNF
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
```
For our identifiers and literals we will want a way of representing letters and digits as a way of composing them:
```EBNF
<letter> ::= "A" | "B" | "C" | "D" | "E" | "F" | "G" |
             "H" | "I" | "J" | "K" | "L" | "M" | "N" | "O" | 
             "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" |
             "X" | "Y" | "Z" | "a" | "b" | "c" | "d" | "e" |
             "f" | "g" | "h" | "i" | "j" | "k" | "l" | "m" |
             "n" | "o" | "p" | "q" | "r" | "s" | "t" | "u" |
             "v" | "w" | "x" | "y" | "z";
<digit> ::= "0" | "1" | "2" | "3" | "4" |
            "5" | "6" | "7" | "8" | "9";
```
which then allows us to define identifiers and our 4 literal types:
```EBNF
<identifier> ::= <letter> (<letter> | <digit> | "_")*;

<integer_literal> ::= <digit>+;
<decimal_literal> ::= <digit>+ "." <digit>+;
<bool_literal> ::= "true" | "false";
<string_literal> ::= "\"" "{any non-quote terminal}"* "\"";
```
Which completes our set of tokens.

We will want to add one more symbol though, as our tokeniser needs to be able to consume the whole of the plaintext input and that is whitespace. Since we don't use whitespace semantically we can merely define this symbol as something that the tokeniser consumes somewhat greedily and just discards afterwards:
```EBNF
<whitespace> ::= (" " | "\t" | "\n" | "\r")*;
```

We now have a complete grammar that can represent the core set of tokens we need for ChibiLisp.

### Finishing Our Grammar:

The last thing we need is a grammar for the rest of our syntax. We want something quite specific out of our grammar and that is because we want the grammar to act as a template for our parser (remember chapter 0) to transform the tokens from before into that AST tree structure we described.

We will design our grammar from the top down starting with a single symbol that represents our entire program and work down defining our symbols. Another concern, of course, is that our example from earlier was a very minimal example so we want to keep our grammar extensible to the features we will be adding later. This means we'll have quite a few symbols for the moment that are just defined as equivalent to some other symbol but that will, over time, accumulate more alternatives.

We know we want out program to consist of multiple top-level objects, and that one of the things those objects can be is a definition so we can start there with our grammar:
```EBNF
<program> ::= <object>*;

<object> ::= <definition>;
```
In the case of our example from above that definition was, specifically, a proc definition which was comprised of the syntax "define proc" and then some other internal things such as the name, type, and procedure body all surrounded by parens. We can therefore define those symbols with something like this:
```EBNF
<proc_definition> ::= <l_paren> <keyword_define> <keyword_proc>
                 <proc_definition_inner> <r_paren>;
```
And, in fact, we just covered what is in proc_definition_inner as well:
```EBNF
<proc_definition_inner> ::= <l_paren> <identifier> <proc_type>
                      <proc_body> <r_paren>;
```
With the identifier being the name and the proc_type and proc_body being symbols we have yet to define.

For our body, we saw that we wanted a syntax that had the word body and then some statements (for now we only have return but obviously there are plenty more we will want later), again, surrounded by brackets:
```EBNF
<proc_body> ::= <l_paren> <keyword_body> <statement>*
                <r_paren>;
```
And for the procedure type we want the return type then, in brackets, a list of argument declarations (which we will simply name declarations in order to re-use the symbol for variable declarations) that contain a name and a type (of course, for the main function in the example above this is an empty list so we don't strictly need to define these yet but it is obvious that for other procedures we will want to):
```EBNF
<proc_type> ::= <l_paren> <type> <l_paren> <declaration>*
                <r_paren> <r_paren>;
```
If we go back to the proc_body, we can also define our statement symbol. For now, this is easy as there is only a return statement which itself contains only the word return and some expression to be returned all in parens:
```EBNF
<statement> ::= <return_statement>

<return_statement> ::= <l_paren> <keyword_return> <expression>
                <r_paren>;
```
For the return type of the proc_type we can obviously use one of our base types we discussed for the tokeniser grammar but since we might eventually want to have procedures in our type system (for, say, returning function pointers) and, indeed, other sorts of types such as pointers and arrays we will define our top-level type symbol in terms of our two lower level symbols of either a proc_type or a base_type:
```EBNF
<proc_type> ::= <l_paren> <type> <l_paren> <declaration>*
                <r_paren> <r_paren>;

<base_type> ::= <keyword_bool> | <keyword_i8> | <keyword_i16> |
                <keyword_i32> | <keyword_i64> | <keyword_u8> |
                <keyword_u16> | <keyword_u32> | <keyword_u64> |
                <keyword_f32> | <keyword_f64> | <keyword_f80> |
                <keyword_string>;
```
Which leaves now only the declaration and the expression left. For declarations we need only the type (in this case we can only declare base types as arguments for now) and an identifier for the name of the parameter all wrapped in parens:
```EBNF
<declaration> ::= <l_paren> <base_type> <identifier> <r_paren>;
```
And for an expression we currently only have literal values. However, as you saw our token literals don't match up with the types we defined (for example, having a single integer literal but many integer types from u8 through i64). The example sample therefore shows that our literal must have the type of the literal as well as the literal itself enclosed in brackets (int this case '(i32 69)') so our expression and value literal symbols would look something like this@
```EBNF
<expression> ::= <value_literal>;

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
```
Note that we don't simply use value_literal directly as this would cause problems once we want to start passing things like additions such as 5+2 into function arguments rather than literals directly.

That is actually all of the grammar we need so far to parse the simple program from above:
```lisp
(define proc (main (i32 ()) (body (return (i32 69)))))
```
And observe how, through careful thought about what the semantic structure of our syntax might be we were able to define the syntax for this using an EBNF formal grammar in a way that corresponds nicely with the structure of the AST we will later be building. (If you don't quite follow that point now, don't worry, once we look at parsers in a bit more detail in chapters 5 and 6 it will make a lot more sense what I'm talking about here.)

### Stepping Back:

One important property of the grammar for the parser is that we don't use any terminals, we only use the non-terminals produced by the tokeniser. This is because of the fact that the tokeniser must run entirely first and produce all of its output tokens before we can treat them as the input to our parser grammar. In the references for this chapter you will see the entire grammar unified in a single file but there will be a line across the middle with the symbols below the line able to consume terminals and representing the tokens produced by the tokeniser and the symbols above the line representing the AST nodes produced by the parser and therefore using no terminals. In a later chapter, we will even re-write the parser grammar to represent the token symbols as literals themselves since from the perspective of the parser which receives a string of tokens and not a string of characters, that's exactly what they actually are.

Note as well how this separation gave us clear benefits in terms of being able to clean up our grammar. Since we could specify that we were not emitting any whitespace from the tokeniser, we could simply ignore it when creating the grammar for our parser. Had this not been the case we would have had to put a whitespace symbol in between every symbol of our parser's grammar. This not only clutters up the grammar, but because the grammar directly mirrors the structure of the AST our parser will construct, and therefore the structure of the code we will have to write _for_ our parser, it also clutters up our code making it harder to read and less maintainable.

This isn't the only advantage of tokenisation. Tokenisation can also play a role in making our grammar as close to context-free as possible as well. Remember how I said that [Python](https://www.python.org/)'s indentation-based scoping isn't strictly context free? Well we can make it context-free using the tokeniser by, instead of emitting a token for every level of indentation at the start of the line, only emitting a token every time the indentation increases or decreases. In fact, such a system would make those tokens identical to the ones representing, say, curly braces in C-like languages. It also allows us to make decisions between keywords and mere identifiers which would otherwise be ambiguous, or to evaluate numeric and decimal literals so the parser can see their value as a number rather than simply a string of digits (something we obviously explored earlier without pointing out that it was a specific advantage of tokenisation). So you can see how as the language gets more complex it becomes really powerful to have basic tokens representing small bits of syntactic meaning that are generated from the plaintext and can effectively act as the terminals for our more complex parser.

Before finishing up, and I promise we're about too, I would also like to bring up a point I made last chapter. Remember I said our example grammar was likely not how you would go about designing the grammar for a system of equations. Hopefully you can start to see why looking at the grammar we defined for our syntax here and the relationship we tried to build between it and the AST. When we covered an example of a mathematical AST in chapter 0 we saw how the nodes represented the structure of our BIDMAS rules so we would likely do the same thing in our grammar. Instead of simply defining our expression as a list of unaries with ops in between them we would likely define a set of different symbols to represent exponentiation, multiplication and division, addition and subtraction, unaries, then individual numbers and bracketed expressions. Hopefully you can see the analogy between that and the way we are defining our grammar here to mirror the semantics of our language as it is and is likely to become. If not, don't worry, seeing it in practice over the next few chapters will build that understanding. 

In the next chapter we will be focussing on writing our tokeniser to turn a plaintext input file into a stream of tokens that we can hand to the parser.
