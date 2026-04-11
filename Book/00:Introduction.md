<h1 align=center>Chapter 00: Introduction</h1>

### Overview:

This ebook is a tutorial on building a language parser and compiler for the first time using the Libchibi backend library developed by myself specifically to facilitate making this process as easy and beginner friendly as possible. It also covers the process of designing a language iteratively throughout the book with the hopes of guiding you through using the tools and processes to design your own languages afterwards as well.

Specifically, we will be building a language tokeniser (often called a lexer) and parser (you will learn what this means shortly) and largely relying upon Libchibi to act as a backend to turn this code into a compiled object file which can be linked into an executable.

Does this mean that the skills and techniques taught here are useless for general compiler development? No. In fact pretty much every modern compiler (and indeed interpreter!) goes through broadly the same initial stages taught here before typically undertaking some additional steps which I absolutely encourage you to learn about on your own after following along with this.

The target audience for this book is people who are used to systems programming in C but don't have experience with programming language development. We will be taking quite a pragmatic approach but we will explore the process of designing a language, including the use of formal grammar to design and define our syntax.

The aim of this book is to make it as easy as possible to follow along using only a C compiler (I recommend gcc as it is installed by default on most Linux distributions) and the [Libchibi](https://github.com/othd06/libchibi) library (for which I will have hopefully provided reasonable build instructions by the time you read this!) however it is necessary that you be running Linux on an x86-64 hardware platform in order to follow along. If you are on Windows I recommend using [WSL](https://learn.microsoft.com/en-gb/windows/wsl/) however I will not be testing that everything does, in fact, work so I cannot guarantee perfect compatibility. If you are using MacOS then you are likely also using arm64 rather than x86-64 meaning the best I can recommend is to set up a virtual machine however since I have never used MacOS I can't give you any more specific advice on how to do that.

I would also recommend that you be at least fairly familiar with C. You needn't be some sort of C god, indeed it isn't even my own preferred language, that title belongs to [Nim](https://nim-lang.org/) (although typically configured to be semantically very C-like), but compilers, even when they're made easy, as I intend to do here, certainly aren't a style of project that are well suited to people who have never touched a line of systems-level code before. In particular, I encourage you to be familiar and comfortable with pointers as well as able to at least read and understand C macros. It is not necessary to have any experience writing programming languages to follow along, indeed, I've aimed the book very much at those who **don't** have that experience.

Additionally, I recommend looking over the [include header](https://github.com/othd06/libchibi/blob/main/libchibi.h) for Libchibi. I don't expect you to understand it all right now but once you get a few chapters in it will start to become a resource you'll begin to follow and _want_ to refer back to so you can begin to write code using it yourself.

### Roadmap:

Before we begin, it's worth getting an overview of what a compiler looks like. This is generally a pretty standardised pipeline of transformations regardless of the compiler. The typical process tends to be as follows (don't worry, I will explain them all in detail just below):
```
plaintext -> tokenisation -> parsing to an AST -> lowering to IR -> generating assembly
```

Oftentimes the final stage will be handled by some other backend such as [LLVM](https://releases.llvm.org/), [Cranelift](https://cranelift.dev/), or [QBE](https://c9x.me/compile/) which will typically perform certain optimisations to improve the speed of your compiled code as well as providing a generic way to compile for multiple different architectures and operating systems. I highly recommend looking into them if you want to get more of an insight into how 'real' compilers operate in the wild. I particularly recommend looking into QBE as a sort of intermediate stepping stone of complexity between what we will do in this book and the much greater complexity of industrial-grade backends like Cranelift and, especially, LLVM.

These systems will generally define an 'intermediate representation' (IR) that must be generated and handed to them in order to perform optimisations and generate valid assembly and which typically resembles assembly quite closely while smoothing over architecture differences and making adjustments for optimisation (look up [SSA form](https://en.wikipedia.org/wiki/Static_single-assignment_form) if you're interested about how they work specifically). IR is also used to make a compiler portable. This is because the IR is low level and assembly-like but generally just high level enough to smooth over platform differences so it can be lowered as easily into x86_64 Windows, as it can aarch_64 MacOS, as it can RISCV_128 Linux or even things we wouldn't traditionally interpret as 'platforms' such as lowering to JavaScript. (Something we have, notably, lost out on because Libchibi, itself, also skips this step internally and goes straight to linux-specific assembly).

When using Libchibi, however, we skip not only the step of generating assembly but also of generating any form of intermediate representation. Instead Libchibi provides us with the tools to generate an AST and directly compile that to assembly (or, for simplicity, a compiled object file).

### ASTs:

Working backwards, as we have, from the final output, the next stage of the pipeline is the **AST**. Since most of our time will be spent constructing and modifying one of these it's worth understanding why they exist. It largely comes down to the fact that taking some code in plaintext form and converting it into correct assembly (or IR) is quite a daunting task that seems tricky at best and near-impossible at worst. In fact, this was how many of the very earliest C compilers worked and therefore what C was designed for (in case you ever wondered why you need to declare a function **before** calling it in C this is why). You can still see an example of this with the [TinyCC](https://bellard.org/tcc/) compiler. This approach, though, really only makes sense to do in very memory-constrained environments like those of the very earliest C compilers in the 80's and, as you may have guessed from the way that it limits C so much, not for languages that want to support many 'modern' features.

Instead it is usually preferred to first transform a piece of code into a tree-like structure that captures the semantic meaning of a program then generate the IR from that. The simplest of these is the so called **Abstract Syntax Tree**. There are others such as a Concrete Syntax Tree which more closely mirrors the literal content of some plaintext code rather than the meaning but we will be generating only an AST. To get to grips with exactly what an AST is, lets look below at how we might generate a tree for something like the expression "x = 12*y-3*(4+z)/5": 
```
    assign
    /     \
   x      sub
         /   \
       mul   div
      /  |   |  \ 
    12   y  mul  5
           /   \
          3    add
              /   \
             4     z
```
We can see here that the rules of BIDMAS (and the brackets) instead of being implicit rules about how maths works are directly encoded in the **structure** of the tree. If it takes you a second don't worry; Just stare at the code and the tree for a second and you'll get it. This is what allows us to calculate the output of those rules (and any of the rules of a formal coding language) using only the very basic instructions of assembly which have no idea about things like BIDMAS.

### Why Industrial Compilers Go Beyond The AST

So if we always need to construct an AST, why do other backends have a separate IR? The first reason is optimisation. It's generally more difficult to do many forms of optimisation using an AST representation. There is, however, a more fundamental reason and that is because the AST still represents the meaning of the language rather than the behaviour of the computer when executing it. Specifically, Libchibi was designed to use the AST produced in the [chibicc](https://github.com/rui314/chibicc) C compiler so the specific nodes available are tied to the kind of constructs and meaning available in C. Indeed, later in the book we will see how a feature common to many languages but not present in C, local functions, is actually kind of tricky using Libchibi's standard AST nodes and requires us to do our own transformations to get it to work. Indeed, for many compilers using Libchibi it will make more sense to define their own AST nodes and first parse into that before transforming the result into the sort of C-style AST expected by Libchibi (still a much simpler process than lowering fully to an IR like LLVM IR for languages that share some semantic similarity with C, which is almost all languages).

So does that mean that we're limiting what we can do by using Libchibi? A bit, yes. But not nearly as much as one might assume. In fact, C itself is used as a sort of intermediate representation by many languages with the C compiler acting as the backend. I mentioned the language [Nim](https://nim-lang.org/) before which notably does this but even languages that are radically different than C can do this such as [Haskell](https://www.haskell.org/) whose primary compiler, [ghc](https://www.haskell.org/ghc/) has the ability to compile to C for use on platforms that it doesn't support directly but that do have a C compiler. So using an AST representation designed for C certainly isn't a death sentence for flexibility, especially since if you really need it you can directly use inline assembly in C to fulfil more advanced features that can't be easily directly handled by C (such as the custom allocators of [Zig](https://ziglang.org/), [Odin](https://odin-lang.org/), and [C3](https://c3-lang.org/)). In fact, using an AST and backend designed for C can be a great benefit in that it makes our code integrate very neatly and well with existing C code (or code that exposes a C API) which constitutes the vast majority of useful, in particular, library code that exists in the wild.

### Tokenisation:

The last step left to explain in our pipeline is so-called _tokenisation_ (often referred to as lexing or, occasionally, scanning). In reality, we don't parse into an AST directly from plaintext because a string itself is actually quite hard to do anything useful with. If the parser is what works out the structure of what a program means, the tokeniser is what works out the structure of what a program _literally says_. That is to say, if we had the entire piece of code in just a string then the entire thing would just be individual characters with no understanding of where the words, numbers, and punctuation begin and end or what is contained within them. This is exactly the role of the tokeniser. To split the text into **tokens** that each represent a bit of meaning tied to the text, typically either:
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

### Our Language:

Now that we know the shape of compilers in general and which parts we will write ourselves, the final question to ask is what language we will be compiling. This is something that we will iteratively define throughout the book, and I'll be able to give you a better answer after the next chapter where we talk a bit about how we define the grammar of a language but for now I can give you a high level overview. The language we will be compiling is called ChibiLisp. As you can probably tell from the name this is in the lisp family of languages which is a family that goes back as far as any other with the original lisp being the second oldest compiled language in 1958, only preceded by FORTRAN. A major reason for this is that it's very easy to parse into an AST because it structurally mirrors an AST making it the perfect family of languages for a first ever compiler (beyond toy languages such as [Brainfuck](https://simple.wikipedia.org/wiki/Brainfuck)). Note that ChibiLisp is not a [Scheme](https://www.scheme.org/) family language and will, in fact, be quite a maximal lisp in terms of the syntax we will directly parse so the experience of writing a parser for it will be a lot like writing a parser for any other language just smoothed over in places (rather than glossed over completely).

As you can tell by the fact that we're writing a compiler ChibiLisp will be statically compiled and, in contrast with many lisps, it will be statically typed. It will also be multi-paradigm between procedural and functional although the functional features will be implemented later in the book as they're more advanced to implement in a C-like AST meaning ChibiLisp will start out looking semantically a lot like a subset of C before later growing out into something more recognisably 'lisp'.

I should add, many books like this try to build out a 'toy' language that simply demonstrate the principles of compiling. In a sense this will be similar since we wouldn't really want to accept the tradeoffs of Libchibi for optimisation and portability for a true industrial-grade language. But in terms of the actual content of the input language the intention is to build a language that will go beyond being simply a toy language and become genuinely useful beyond merely the text of this book.

In the next chapter we will cover how we write and define the grammar of a formal language using notation in preparation for starting to write out our compiler.
