<h1 align=center>Chapter 01: Understanding Grammar</h1>

### Overview:

In the previous chapter we outlined the stages of compilation and what we will be writing throughout this book. We also touched upon the language we will be writing: ChibiLisp. In this chapter we will explore how we can go about defining what exactly ChibiLisp looks like by learning how to write a grammar that describes our syntax.

You'll often see this discussed in the context of certain design tools that can automate parts of writing your compiler. Even though we're not doing that, it is _still_ very worthwhile to learn this now as a key tool for designing and implementing our language. This is because it allows us to directly define and iterate on the syntax of the language without needing to introduce, then modify, the code of our parser. On top of that, it provides a really useful framework to structure that code when we do start writing it as well. Think of it like static vs dynamic typing; it introduces a bit of friction at first but the benefit of being able to plan things out in a structured way eventually pays off as your complexity increases.

### Introducing BNF:

Perhaps the simplest notation we can use is something called [BNF](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_form) (or Backus-Naur Form) notation. BNF is a system designed in the late 50s for the development of a language named ALGOL 60 (an early predecessor of languages like C, PASCAL, and their descendants). The way that it works is by defining a set of so-called 'non-terminal' symbols in terms of rules that allow them to be substituted for other terminal or non-terminal symbols with each non-terminal being enclosed in angle brackets \<like this> and each terminal being enclosed in quotes "like this" (Note: you will sometimes see **bold** and _italics_ used in place of angle brackets and quotes however, in the interests of making it as easy for you to do this yourself in plain text I have chosen this convention). Each possible substitution rule is written as a non-terminal followed by the symbol '::=' followed by a set of terminal or non-terminal symbols that could replace the non-terminal on the left. For example we might write a grammar for mathematical equations that looks like this:
```bnf
<equation> ::= <expression> "=" <expression>
<expression> ::= <unary> <op> <unary>
<expression> ::= <unary> <op> <expression>
<op> ::= "+"
<op> ::= "-"
<op> ::= "*"
<op> ::= "/"
<op> ::= "^"
<unary> ::= <number>
<unary> ::= "-" <number>
<unary> ::= <number> "!"
<number> ::= <digit string>
<number> ::= <digit string> "." <digit string>
<digit string> ::= <digit>
<digit string> ::= <digit> <digit string>
<digit> ::= "0"
<digit> ::= "1"
<digit> ::= "2"
<digit> ::= "3"
<digit> ::= "4"
<digit> ::= "5"
<digit> ::= "6"
<digit> ::= "7"
<digit> ::= "8"
<digit> ::= "9"
```
Try it out for yourself. Try picking a replacement rule for each of the non-terminal symbols in the equation then picking replacement rules for each of the non-terminal symbols in what you get until you have just a string of terminals. Do it a few times to get a feel for how these rules work. (Note: if we were actually trying to write a grammar for mathematical equations it is very unlikely we would actually structure it this way since we typically want the format of the grammar to reflect the format of the tree we will use it to parse).

This notation is, in fact, all we need to formalise a vast array of language using what is called a context-free grammar. In reality most languages can't be fully defined unambiguously using only context-free grammar. As a result, the parser (whose job it is to basically go backwards through this process) will generally need to use more than just the written grammar to decide what was meant. Indeed there are some language features, such as [Python](https://www.python.org/)'s indentation, that can't be fully expressed as a context-free grammar at all, and need some pre-processing to turn them into something of that form. In practice, however, languages tend not to diverge too greatly from the structure of a context-free grammar so this is a widely used tool for designing languages.

So in practice this kind of notation provides a powerful tool to design and define the syntax of our (or any) language. You'll see later how each non-terminal also naturally corresponds with a function in our parser, creating the template for our programming that I mentioned earlier.

### Expanding Our BNF:

Our example earlier was, however, very verbose. It would be nice to be able to express it in a more compact form. This is where so-called Extended BNFs (EBNFs) come in. There are many forms of EBNF however we will be defining a variant that uses many well accepted and understood conventions that will allow you to write grammars that should be widely understandable.

#### Alternation:

The first thing to note is that having alternative rules on separate lines takes up a lot of space and it would be nice if we had a more compact way to represent different substitution rules for the same non-terminal. For this we use the '|' symbol to separate different rules. This, however, can make our rules quite long, so we might want them to overflow to a new line. Therefore we will put a semicolon at the end of every rule to signify where one rule finishes and another begins. Doing that our previous example looks like this:
```ebnf
<equation> ::= <expression> "=" <expression>;
<expression> ::= <unary> <op> <unary> | <unary> <op> <expression>;
<op> ::= "+" | "-" | "*" | "/" | "^";
<unary> ::= <number> | "-" <number> | <number> "!";
<number> ::= <digit string> | <digit string> "." <digit string>;
<digit string> ::= <digit> | <digit> <digit string>;
<digit> ::= "0" | "1" | "2" | "3" | "4" |  "5" | "6" | "7" | "8" | "9"; 
```
Take a second to reassure yourself that this is, indeed, the exact same grammar.

#### Repetition:

Another thing to notice is that we still have a few rules defined in terms of themselves. That isn't a problem and it's actually a big part of what makes this notation so powerful but in many cases we're just doing it to represent that we want something to be able to repeat. It would be nice if we had a way to represent that directly rather than relying on this sort of recursive definition that takes some time to understand. For that we'll introduce two new symbols: '+' and '*'. These symbols go at the end of a symbol and represent that a symbol repeats 1 or more, or 0 or more times respectively. Using these, our grammar becomes this:
```ebnf
<equation> ::= <expression> "=" <expression>;
<expression> ::= <unary> <op> <unary> | <unary> <op> <expression>;
<op> ::= "+" | "-" | "*" | "/" | "^";
<unary> ::= <number> | "-" <number> | <number> "!";
<number> ::= <digit>+ | <digit>+ "." <digit>+;
<digit> ::= "0" | "1" | "2" | "3" | "4" |  "5" | "6" | "7" | "8" | "9"; 
```
Again, take a second to reassure yourself that this is the same grammar as what we had before.

#### Optionality & Grouping:

Now that we have a way to show that a symbol may repeat multiple times it would be nice if we had a way to show that a symbol could appear either 1 or 0 times for parts of a rule that are optional. Indeed, there is a way to do this too and that's using square brackets around a section of rule that we want to be optional. Taking the earlier example again, we can now re-write it like this:
```ebnf
<equation> ::= <expression> "=" <expression>;
<expression> ::= <unary> <op> <unary> | <unary> <op> <expression>;
<op> ::= "+" | "-" | "*" | "/" | "^";
<unary> ::= ["-"] <number> | <number> "!";
<number> ::= <digit>+ ["." <digit>+];
<digit> ::= "0" | "1" | "2" | "3" | "4" |  "5" | "6" | "7" | "8" | "9"; 
```
As I'm sure you're used to by now, take a second; reassure yourself that this _really_ is the same grammar we had at the start. We only have one more rule to go. You may have noticed that for square brackets we had a way to make a string of multiple symbols appear 0 or 1 times but with '+' and '*' we could only make a single symbol appear multiple times. It would be nice if we had a way to show that these prefixes applied to multiple symbols. We can do that by grouping symbols within parentheses. Like this:
```ebnf
<equation> ::= <expression> "=" <expression>;
<expression> ::= (<unary> <op>)+ <unary>;
<op> ::= "+" | "-" | "*" | "/" | "^";
<unary> ::= ["-"] <number> | <number> "!";
<number> ::= <digit>+ ["." <digit>+];
<digit> ::= "0" | "1" | "2" | "3" | "4" |  "5" | "6" | "7" | "8" | "9"; 
```
That's it. That's all the rules. Compare this example to the first version in plain BNF. We can see that with a few extra rules we can create a grammar notation that is both more succinct and easier to follow.

### Final Notes:

There is, however, one more thing you can do with the brackets that's worth pointing out because it is useful which is that you can put a '|' inside of a bracket to show that there are multiple options **_for that bracketed section_**. For example if we wanted to write a grammar rule for the word colour that allowed us to use either the US or UK spelling we could do this:
```ebnf
<colour> = "col" ("ou" | "o") "r";
```
I admit, that's a slightly contrived example but it's worth noting. As is the fact that we don't actually _need_ any of these rules; everything we can write with them can be done with straightforward BNF so don't feel like you need to use them when coming up with your own grammars but they're handy shortcuts that we will be using in this book.

That's it! Now we're _really_ 100% done. I promise. If you can understand everything I showed you in this chapter then you're ready for the next chapter where we start designing the very first grammar for ChibiLisp.
