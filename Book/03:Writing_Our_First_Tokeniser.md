<h1 align=center>Chapter 03: Writing our First Tokeniser</h1>

Last chapter we explored the grammar of our language, ChibiLisp, in its most minimal form. We explored how this grammar related to the two stages of tokenisation and parsing, and we looked at what we want our tokens to output.

Now we get to do the fun part of writing our tokeniser. I will provide code snippets here and I encourage you strongly to copy along and actually type it out yourself however I will also be providing, in the references section of this repository, the full code of the tokeniser we will be generating in this chapter.

In line with the philosophy of this entire book which is streamlined simplicity and ease of development, we will not be setting up a full build system. It is for this reason that we will be writing our modules in the form of an 'stb-style' header-only 'library'. You may be familiar with the typical C workflow of writing various C files defining your code and, separately, corresponding header files declaring which functions are available publicly. With this style we instead provide only a single header file that contains two sections: the first a normal header file, and the second the implementations that would normally live in their own C file. A preprocessor directive is then used to ensure the implementation is only included in one compilation unit. This means our tokeniser's file will look as follows:
```c
/************************
* File: tokeniser.h
************************/

#ifndef TOKENISER_H
#define TOKENISER_H

//header goes here

#endif

#ifdef TOKENISER_C
#undef TOKENISER_C

//implementation goes here

#endif
```
This way when we get to writing our main.c file that runs the entire compiler we can do the following:
```c
/************************
* File: main.c
************************/
#include tokeniser.h

//main file goes here

#define TOKENISER_C
#include tokeniser.h
```
We will also have a helper module that uses exactly the same header-only structure as tokeniser: 
```c
/************************
* File: helpers.h
************************/

#ifndef HELPERS_H
#define HELPERS_H

//header goes here

#endif

#ifdef HELPERS_C
#undef HELPERS_C

//implementation goes here

#endif
```


Now that we've established our structure, we can _finally_ get to the fun part. Actually writing code.

Let's start by reminding ourselves of our token grammar (the grammar below the divider in chapter 2):
```ebnf
<l_paren> ::= "(";
<r_paren> ::= ")";

<keyword_define> ::= "define";
<keyword_proc> ::= "proc";
<keyword_body> ::= "body";

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

So from this we can start to define our tokens. You're first instinct might be to define a token using an enum. After all, an enum is the perfect data type for defining a set of discrete categories of thing. But remember, some of our tokens (such as literals) also need to store semantic meaning. We will still use an enum but we that will define the kind and be used to form a **tagged union**. In your header section of tokeniser.h write the following:

```c
typedef enum {
    TOK_STRING_LITERAL,
    TOK_BOOL_LITERAL,
    TOK_INTEGER_LITERAL,
    TOK_DECIMAL_LITERAL,
    TOK_IDENTIFIER,
    TOK_KEYWORD_STRING,
    TOK_KEYWORD_F80,
    TOK_KEYWORD_F64,
    TOK_KEYWORD_F32,
    TOK_KEYWORD_U64,
    TOK_KEYWORD_U32,
    TOK_KEYWORD_U16,
    TOK_KEYWORD_U8,
    TOK_KEYWORD_I64,
    TOK_KEYWORD_I32,
    TOK_KEYWORD_I16,
    TOK_KEYWORD_I8,
    TOK_KEYWORD_BOOL,
    TOK_KEYWORD_BODY,
    TOK_KEYWORD_PROC,
    TOK_KEYWORD_DEFINE,
    TOK_L_PAREN,
    TOK_R_PAREN,
} TokenKind;

typedef struct {
    TokenKind kind;
    union {
        char* string_literal;
        bool bool_literal;
        long integer_literal;
        long double decimal_literal;
        char* identifier;
    } data;
} Token;
```

As you can see our token contains two elements: the kind of token, and a union containing whatever semantic data the token may have. This is what will allow the parser to check which kind of token is being read in then correctly store the meaning of that token. Note that there is no whitespace token kind (or, for that matter, any token kind not used by the parser in the top half of the grammar from chapter 2).

Now we need a function that can take in a string of chars and return a string of these tokens. For this we will use a data structure that I am calling a Seq but that you might know as a Vector from C++.
```c
#define Seq(T) struct {T* data; long len; long cap}
#define append(T, SEQ, VAL) {\
Seq(T)* seq = &SEQ;\
if(seq->len >= seq->cap) {\
    if(seq->cap == 0) seq->cap = 1;\
    T* old_data = seq->data;\
    seq->cap *= 2;\
    seq->data = calloc(seq->cap, sizeof(T));\
    if (old_data != NULL) {\
        memcpy(seq->data, old_data, seq->len*sizeof(T));\
        free(old_data);\
    }\
}\
seq->data[seq->len] = VAL;\
seq->len += 1;\
}
```
This may look a touch weird because we're using C preprocessor macros rather than functions or standard typedefs to define the Seq and the append operation. This is so that these macros work for any type of Seq without having to re-implement Seqs for every type we might possibly want. I won't go into too much detail about how it works here since the point of this book isn't to explain Seqs or C macros but I encourage you to try to understand what the code is doing under the hood. The main takeaway is that Seqs are an array of a dynamic length that automatically grow when we try to add an element without having enough space (just like Vectors in C++).

A final note before moving on. We free the old_data when growing the Seq. This makes sense because for a large Seq it's likely that we will produce many smaller data allocations that we absolutely don't need later so we may as well clean them up however, libchibi is designed to allocate with calloc (like malloc but zero-initialises the allocated memory) and basically never free. This is because, for a program like a compiler that runs once, executes, then quits and hands all its system resources back to the operating system rather than running continuously in a loop, it's fine to leak some memory and rely on the OS to clean it up. So you wont be seeing a lot of frees throughout this book. This obviously looks a bit weird but it massively simplifies our code since we never have to think about who owns a piece of memory since the answer is generally no-one.

We're going to need one final helper function. This time though it's just a regular function; no more macros.
```c
int string_in_array(char* string, char* array[], int array_len) {
    for (int i = 0; i < array_len; i++) {
        if (strcmp(string, array[i]) == 0) return i;
    }
    return -1;
}
```
Of course, don't forget to copy the signature into the header section of helpers.h. This will help us later to decide if a token is an identifier or a keyword.

We can now declare our function in the header section of tokeniser.h (remembering to place it below both the definition of Token and the import line for helpers.h):
```c
Seq(Token) tokenise(char* plaintext);
```
That's it. The entire signature. All of the complexity in our tokeniser lives inside the implementation. Finally, after all this setup, we can start to write our actual tokeniser. The tokeniser will run in a loop iterating through the string and emitting tokens as soon as they are ready. It is, after all, the role of the tokeniser to determine when a token is complete and ready to emit. We need a few things as setup though. Specifically our tokeniser will want an index to track how far through the string it is, a Seq of tokens to add new tokens to (and ultimately to return), and a dictionary of keywords to compare against. So let's build that.
```c
Seq(Token) tokenise(char* plaintext) {
    int index = 0;
    Seq(Token) tokens = {0};
    char* keyword_strings[] = {"string", "f80", "f64",
        "f32", "u64", "u32", "u16", "u8", "i64", "i32",
        "i16", "i8", "bool", "body", "proc", "define"};
    TokenKind keyword_kinds[] = {TOK_KEYWORD_STRING,
        TOK_KEYWORD_F80, TOK_KEYWORD_F64,
        TOK_KEYWORD_F32, TOK_KEYWORD_U64,
        TOK_KEYWORD_U32, TOK_KEYWORD_U16,
        TOK_KEYWORD_U8, TOK_KEYWORD_I64,
        TOK_KEYWORD_I32, TOK_KEYWORD_I16,
        TOK_KEYWORD_I8, TOK_KEYWORD_BOOL,
        TOK_KEYWORD_BODY, TOK_KEYWORD_PROC,
        TOK_KEYWORD_DEFINE};
    
    //tokenisation loop here...

    return tokens;
}
```
Note that the order of the strings corresponds exactly with the order of the token kinds. This is important because we will be using the index returned by string_in_array from the array to strings to index into the array of kinds.

We can then start to write our main loop. The loop will run until the character at index **index** of the plaintext string is '\0' (indicating the end of the string) and we can branch within the loop based on if the character is alphabetical, a digit, whitespace, or symbolic. We can use the standard C functions isalpha, isdigit, and isspace for this.
```c
Seq(Token) tokenise(char* plaintext) {
    int index = 0;
    Seq(Token) tokens = {0};
    char* keyword_strings[] = {"string", "f80", "f64",
        "f32", "u64", "u32", "u16", "u8", "i64", "i32",
        "i16", "i8", "bool", "body", "proc", "define"};
    TokenKind keyword_kinds[] = {TOK_KEYWORD_STRING,
        TOK_KEYWORD_F80, TOK_KEYWORD_F64,
        TOK_KEYWORD_F32, TOK_KEYWORD_U64,
        TOK_KEYWORD_U32, TOK_KEYWORD_U16,
        TOK_KEYWORD_U8, TOK_KEYWORD_I64,
        TOK_KEYWORD_I32, TOK_KEYWORD_I16,
        TOK_KEYWORD_I8, TOK_KEYWORD_BOOL,
        TOK_KEYWORD_BODY, TOK_KEYWORD_PROC,
        TOK_KEYWORD_DEFINE};
    
    while (plaintext[index] != '\0') {
        if (isalpha(plaintext[index])) {
            //consume a keyword or identifier token
        } else if (isdigit(plaintext[index])) {
            //consume an integer_literal or decimal_literal token
        } else if (isspace(plaintext[index])) {
            //consume the whitespace character and continue
        } else if (plaintext[index] == '(') {
            //consume an l_paren token
        } else if (plaintext[index] == ')') {
            //emit an r_paren token
        } else if (plaintext[index] == '"') {
            //consume a string token
        } else {
            fprintf(stderr, "Error: leading token character not recognised\n");
            exit(1);
        }
    }

    return tokens;
}
```
We can fill in the cases for whitespace and parens very easily:
```c
//whitespace
else if (isspace(plaintext[index])) {
    index++;
}
```
```c
//l_paren
else if (plaintext[index] == '(') {
    index++;
    Token new_token;
    new_token.kind = TOK_L_PAREN;
    append(Token, tokens, new_token);
}
```
```c
//r_paren
else if (plaintext[index] == ')') {
    index++;
    Token new_token;
    new_token.kind = TOK_R_PAREN;
    append(Token, tokens, new_token);
}
```
As you can see we simply increment the index by one to consume the character then, in the case of parens: create a new token, set its type appropriately, and append it to the list of tokens to be returned.

The other cases are a bit more complex. Lets look at if the character is alphabetical first. Clearly the first thing we have to do is build a string containing all the alphanumerical (or '_') characters that appear in an uninterrupted row. Then we can compare that string with the keyword dictionary to decide if we should append an identifier token or a keyword token. In fact, we can use a Seq for this again since a Seq of chars contains an array of chars in it, we just need to remember to null terminate it after the alphanumeric characters end.

So let's see what that looks like:
```c
//alphabetical
if (isalpha(plaintext[index])) {
    Seq(char) token_string = {0};
    while (isalpha(plaintext[index]) || isdigit(plaintext[index]) || plaintext[index] == '_') {
        append(char, token_string, plaintext[index++]);
    }
    append(char, token_string, '\0');
    int keyword_index = string_in_array(token_string.data, keyword_strings, sizeof(keyword_strings)/sizeof(char*));
    if (strcmp(token_string.data, "true") == 0) {
        Token new_token;
        new_token.kind = TOK_BOOL_LITERAL;
        new_token.data.bool_literal = true;
        append(Token, tokens, new_token);
    } else if (strcmp(token_string.data, "false") == 0) { 
        Token new_token;
        new_token.kind = TOK_BOOL_LITERAL;
        new_token.data.bool_literal = false;
        append(Token, tokens, new_token);
    }else if (keyword_index == -1) {
        Token new_token;
        new_token.kind = TOK_IDENTIFIER;
        new_token.data.identifier = token_string.data;
        append(Token, tokens, new_token);
    } else {
        Token new_token;
        new_token.kind = keyword_kinds[keyword_index];
        append(Token, tokens, new_token);
    }
}
```
Take a second; look it over. Reassure yourself that you see what we're doing here. Notice, as well, how we can add new keywords really easily; all we have to do is create a new TokenKind and add it, as well as the corresponding string, to the end of their respective parts of the keyword dictionary (although it is admittedly slightly more tedious to add more identifier literals such as true or false).

It is relatively trivial, now, to see how we would do the same thing for the string case:
```c
//string_literal
else if (plaintext[index] == '"') {
    index++; //consume the opening quote
    Seq(char) token_string = {0};
    while (plaintext[index] != '"') {
        append(char, token_string, plaintext[index++]);
    }
    append(char, token_string, '\0');
    index++; // consume the closing quote
    Token new_token;
    new_token.kind = TOK_STRING_LITERAL;
    new_token.data.string_literal = token_string.data;
    append(Token, tokens, new_token);
}
```
All of a sudden our tokeniser is almost done. We can even add the ability to escape quotes by checking if the last character in our string is a backslash like this:
```c
//string_literal
else if (plaintext[index] == '"') {
    index++; //consume the opening quote
    Seq(char) token_string = {0};
    while ((token_string.len > 0 && token_string.data[token_string.len-1] == '\\') || plaintext[index] != '"')  {
        append(char, token_string, plaintext[index++]);
    }
    append(char, token_string, '\0');
    index++; // consume the closing quote
    Token new_token;
    new_token.kind = TOK_STRING_LITERAL;
    new_token.data.string_literal = token_string.data;
    append(Token, tokens, new_token);
}
```
Now the only remaining case is when we find a leading digit.

If the token is an integer_literal it's simply a case of doing the same thing as before except instead of appending to a Seq of chars we multiply a running total by 10 then add the value of the digit (which we can get by taking the [ASCII](https://en.wikipedia.org/wiki/ASCII) encoding of the digit in our char, casting to a long, and subtracting [48](https://en.wikipedia.org/wiki/ASCII#Printable_character_table)). We can then check if the next character is a '.'. If not, we emit the token as an integer literal. Otherwise, we have to do the same thing again to build a second integer then cast both integers to long floats, divide the second one by 10 until it is less than 1 (to shift it into the decimal part) and add them together.

And this is the code to do that:
```c
//numerical
else if (isdigit(plaintext[index])) {
    long token_integer_1 = 0;
    while (isdigit(plaintext[index])) {
        token_integer_1 *= 10;
        token_integer_1 += (long)plaintext[index++]-48;
    }
    if (plaintext[index] != '.') {
        Token new_token;
        new_token.kind = TOK_INTEGER_LITERAL;
        new_token.data.integer_literal = token_integer_1;
        append(Token, tokens, new_token);
    } else {
        index++;

        long token_integer_2 = 0;
        while (isdigit(plaintext[index])) {
            token_integer_2 *= 10;
            token_integer_2 += (long)plaintext[index++]-48;
        }

        long double token_float = (long double)token_integer_2;
        while(token_float >= 1.0) token_float /= 10;
        token_float += (long double)token_integer_1;
        
        Token new_token;
        new_token.kind = TOK_DECIMAL_LITERAL;
        new_token.data.decimal_literal = token_float;
        append(Token, tokens, new_token);
    }
}
```
And just like that we have a complete tokeniser. Obviously when we add new features and change the grammar we might have to adjust the tokeniser but it really is that simple.

At this point we can now convert a plaintext piece of code like our example from chapter 2:
```lisp
(define proc (main (i32 ()) (body (return (i32 69)))))
```
into the equivalent sequence of tokens:
```
[l_paren] [keyword_define] [keyword_proc]
    [l_paren] [identifier: main] [l_paren] [keyword_i32] [l_paren] [r_paren] [r_paren]
        [l_paren] [keyword_body] 
            [l_paren] [keyword_return] [l_paren] [keyword_i32] [integer_literal: 69] [r_paren] [r_paren]
        [r_paren]
    [r_paren]
[r_paren]
```

Don't forget as well to import the correct headers at the start of the header section of tokens.h:
  * \<stdlib.h>
  * \<string.h>
  * \<stdio.h>
  * \<ctype.h>
  * \<stdbool.h>
  * and, of course, "helpers.h"

Before we finish though, you will find that when writing the parse it's helpful to have a token denoting the end of the file. Adding this is very easy. Simply add an element TOK_EOF to the TokenKind union and then, before returning from the tokenise function make sure to add an eof token:
```c
//return
Token eof_token;
eof_token.kind = TOK_EOF;
append(Token, tokens, eof_token);
return tokens;
```

Next chapter we will look at different parsing techniques as well as the specific format expected by Libchibi in preparation for taking the output of this tokeniser and actually parsing it to compile our very first program.

