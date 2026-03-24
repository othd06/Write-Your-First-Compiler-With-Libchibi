<h1 align=center>Chapter 00: Introduction</h1>

This ebook is a tutorial on building a language parser and compiler for the first time using the Libchibi backend library developed by myself specifically to facilitate making this process as easy and beginner friendly as possible.

Specifically, we will be building a language lexer and parser (you will learn what this means shortly) and largely relying upon Libchibi to act as a backend to turn this code into a compiled object file which can be turned into an executable.

Does this mean that the skills and techniques taught here are useless for general compiler development? No. In fact pretty much every modern compiler (and indeed interpreter!) goes through broadly the same initial stages taught here before typically undertaking some additional steps which I absolutely encourage you to learn about on your own after following along with this.

The aim of this book is to make it as easy as possible to follow along using only a C compiler (I recommend gcc as it is installed by default on most Linux distributions) and the [Libchibi](https://github.com/othd06/libchibi) library (for which I will have hopefully provided reasonable build instructions by the time you read this!) however it is necessary that you be running Linux on an x86-64 hardware platform in order to follow along. If you are on Windows I recommend using [WSL](https://learn.microsoft.com/en-gb/windows/wsl/) however I will not be testing that everything does, in fact, work so I cannot guarantee perfect compatibility. If you are using MacOS then you are likely also using arm64 rather than x86-64 meaning the best I can recommend is to set up a virtual machine however since I have never used MacOS I can't give you any more specific advice on how to do that.

I would also recommend that you be at least somewhat familiar with C. You needn't be some sort of C god, indeed it isn't even my own preferred language, that title belongs to [Nim](https://nim-lang.org/) (although typically configured to be semantically very C-like), but compilers, even when they're made easy, certainly aren't a style of project that are well suited to people who have never touched a line of systems-level code before.

So, what will this process involve? Well, there are a few stages to writing a compiler. The typical process tends to be as follows (don't worry, I will explain them all in detail just below):
```
plaintext -> tokenisation -> parsing to an AST -> lowering to IR -> generating assembly
```

Oftentimes the final stage will be handled by some other backend such as [LLVM](https://releases.llvm.org/), [Cranelift](https://cranelift.dev/), or [QBE](https://c9x.me/compile/) which will typically perform certain optimisations to improve the speed of your compiled code as well as providing a generic way to compile for multiple different architectures and operating systems. I highly recommend looking into them if you want to get more of an insight into how 'real' compilers operate in the wild. I particularly recommend looking into QBE as a sort of intermediate stepping stone of complexity between what we will do in this book and the much greater complexity of industrial-grade backends like Cranelift and, especially, LLVM. These systems will generally define an 'intermediate representation' (IR) that must be generated and handed to them in order to perform optimisations and generate valid assembly and which typically resembles assembly quite closely while smoothing over architecture differences and making adjustments for optimisation (look up [SSA form](https://en.wikipedia.org/wiki/Static_single-assignment_form) if you're interested about how they work specifically).

When using Libchibi, however, we skip not only the step of generating assembly but also of generating any form of intermediate representation. Instead Libchibi provides us with the tools to generate an AST and directly compile that to assembly (or, for simplicity, a compiled object file).

So what is an AST and why does Libchibi allow us to pass in one directly while other backends require an IR? Well, taking some code in plaintext form and converting it into correct assembly (or IR) is quite a daunting task that seems tricky at best and near-impossible at worst. Indeed, this was how many of the very earliest C compilers worked and therefore what C was designed for (in case you ever wondered why you need to declare a function **before** calling it in C this is why). You can still see an example of this with the [TinyCC](https://bellard.org/tcc/) compiler. In general though this is an incredibly tricky task that really only makes sense to do in very memory-constrained environments like those of the very earliest C compilers in the 80's and, as you may have guessed from the way that it limits C so much, not for languages that want to support many 'modern' features.

Instead it is usually preferred to first transform a piece of code into a tree-like structure that captures the semantic meaning of a program then generate the assembly from that. The simplest of these is the so called **Abstract Syntax Tree**. There are others such as a Concrete Syntax Tree which more closely mirrors the literal content of some plaintext code rather than the meaning but we will be generating only an AST. To get to grips with exactly what an AST is, lets look below at how we might generate a tree for something like the expression "x = 12*y-3*(4+z)/5": 
```
    assign
    /     \
   x      sub
         /   \
       mul   mul
      /  |   |  \ 
    12   y  mul  5
           /   \
          3    add
              /   \
             4     z
```
We can see here that the rules of BIDMAS (and the brackets) instead of being implicit rules about how maths works are directly encoded in the **structure** of the tree (if it takes you a second don't worry. Just stare at the code and the tree for a second and you'll get it) which is what allows us to calculate those rules (and any of the rules of a formal coding language) using only the very basic instructions of assembly which have no idea about things like BIDMAS.

So if we always need to construct an AST, why do other backends have a separate IR? The first reason is optimisation. It's generally more difficult to do many forms of optimisation using an AST representation. There is, however, a more fundamental reason and that is because the AST still represents the meaning of the language rather than the behaviour of the computer when executing it. Specifically, Libchibi was designed to use the AST produced in the [chibicc](https://github.com/rui314/chibicc) C compiler so the specific nodes available are tied to the kind of constructs and meaning available in C. Indeed, later in the book we will see how a feature common to many languages but not present in C, local functions, is actually kind of tricky using Libchibi's standard AST nodes and requires us to do our own transformations to get it to work. Indeed, for many compilers using Libchibi it will make more sense to define their own AST nodes and first parse into that before transforming the result into the sort of C-style AST expected by Libchibi (still a much simpler process than lowering fully to an IR like LLVM IR for languages that share some semantic similarity with C, which is almost all languages).

So does that mean that we're limiting what we can do by using Libchibi? A bit, yes. But not nearly as much as one might assume. In fact, C itself is used as a sort of intermediate representation by many languages with the C compiler acting as the backend. I mentioned the language [Nim](https://nim-lang.org/) before which notably does this but even languages that are radically different than C can do this such as [Haskell](https://www.haskell.org/) whose primary compiler, [ghc](https://www.haskell.org/ghc/) has the ability to compile to C for use on platforms that it doesn't support directly but that do have a C compiler. So using an AST representation designed for C certainly isn't a death sentence for flexibility, especially since if you really need it you can directly use inline assembly in C to fulfil more advanced features that can't be easily directly handled by C (such as the custom allocators of [Zig](https://ziglang.org/), [Odin](https://odin-lang.org/), and [C3](https://c3-lang.org/)). In fact, using an AST and backend designed for C can be a great benefit in very similar ways to the benefits of using C as a backend for Nim which is that it makes our code integrate very neatly and well with existing C code (or code that exposes a C API) which constitutes the vast majority of useful, in particular, library code that exists in the wild.

And now that we've explained what parsing to an AST is there remains only one stage of compilation left to explain and that is the mysterious _tokenisation_ step (often referred to as lexing or, occasionally, scanning). In reality, we don't parse into an AST directly from plaintext because a string itself is actually quite hard to do anything useful with. If the parser is what works out the structure of what a program means, the tokeniser is what works out the structure of what a program _literally says_. That is to say, if we had the entire piece of code in just a string then the entire thing would just be individual characters with no understanding of where the words, numbers, and punctuation begin and end or what is contained within them. This is exactly the role of the tokeniser. To split the text into **tokens** that each represent a bit of meaning tied to the text, typically either:
* A Number
* A Keyword
* A String (in quotes)
* Punctuation or
* Some Other Identifier (such as a function or variable name)

For instance we might tokenise the c code "printf("Hello World! %d", 5);" as follows:
```
[IDENTIFIER printf], [SYMBOL LPAREN], [STRING "Hello World! %d"], [SYMBOL COMMA], [NUMBER 5], [SYMBOL RPAREN], [SYMBOL SEMICOLON]
```
with 'printf' being an identifier because it is a function name defined in libc. A word like 'return' might be tokenised as a keyword instead since that has meaning in the actual language rather than the standard library.


The final question to ask is what language we will be compiling. This is something that we will iteratively define throughout the book, and I'll be able to give you a better answer after the next chapter where we talk a bit about how we define the grammar of a language but for now I can give you a high level overview. The language we will be compiling is called ChibiLisp. As you can probably tell from the name this is in the lisp family of languages which is a family that goes back as far as any other with the original lisp being the second oldest compiled language in 1958, only preceded by FORTRAN. A major reason for this is that it's incredibly easy to parse into an AST because it structurally mirrors an AST making it the perfect family of languages for a first ever compiler (beyond toy languages such as [Brainfuck](https://simple.wikipedia.org/wiki/Brainfuck)). As you can tell by the fact that we're writing a compiler ChibiLisp will be statically compiled and, in contrast with many lisps, it will be statically typed. It will also be multi-paradigm between procedural and functional although the functional features will be implemented later in the book as they're more advanced to implement in a C-like AST.

In the next chapter we will cover how we write and define the grammar of a formal language using notation in preparation for starting to write out our compiler.
