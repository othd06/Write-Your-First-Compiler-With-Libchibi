<h1 align=center>Chapter 06: Compiling Our First Program</h1>

This is it. The moment we've been waiting for. And it only took 6 (well, 7 including this one) chapters. We're finally about to be able to execute a program to quit with a custom error code. I know, I know, don't get too excited. Before you know it we might even be able to print hello world.

In all seriousness though this is a big step. We learned the stages of how compilers work, learned what context-free grammars are and how to represent them with EBNF, we built a tokeniser to split our input up into small syntactic units, learned how parsers work and how we can build one ourself, and then we actually did that! A lot has gone into getting where we are and while this is by no means the end of the road we are in the final stretch of actually getting _something_ to compile.

There's a few things that our main file has to do. It has to:
  *  manage our modules
  *  read the filepath from the cmd line
  *  read the file into a string of chars
  *  run our parser
  *  run our tokeniser
  *  compile and assemble our program using Libchibi
  *  link our program with libc and the C runtime (CRT) to produce a final executable

The good news is that none of these are particularly tricky.

Before we get started I want to address that final point in a bit more detail. You might be thinking "why are we linking with libc and the C runtime? We're not implementing C". And you'd be correct; we're not. You might be tempted to think that it's because of Libchibi and the fact that we're using a backend that was originally designed for C, and while that does make it easier to link against it we could actually avoid both libc and the CRT entirely if we wanted to. The truth is though that most languages link with libc and the CRT, even if they implement their own standard library and runtime on top of them. The reason for this is that C has been the baseline for half a century. Indeed, the development of UNIX (in almost every way the predecessor to modern-day linux) was intimately tied with the development of C itself. This means that Operating Systems designed around the expectation of running C code and, while it is possible to interact with the OS directly through assembly and system calls, it is generally the most straightforward and easy way to interact with the OS. While it isn't a concern for us, libc is also the best way to interact with the OS if we might change OS between, say, Linux, MacOS, Windows, and BSD (or any other OS) because you don't have to rewrite the majority of the standard library for every OS you want to support.

With that explained, let's write some code!

Let's start by handling our modules, just like we explained in chapter 2:
```c
/************************
* File: main.c
************************/
#include "tokeniser.h"
#include "parser.h"
#include "libchibi.h"



#define TOKENISER_C
#include "tokeniser.h"
#define PARSER_C
#include "parser.h"
#define HELPERS_C
#include "helpers.h"
```

Now let's write our main function to get the filepath:
```c
//main.c

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Error, incorrect arguments\n");
        fprintf(stderr, "Usage: ChibiLisp <source file> <executable name>\n");
        return 1;
    }
    char* filepath = argv[1];
    char* executable_name = argv[2];

}
```

And now lets read the file's contents into a string:
```c
//main.c

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Error, incorrect arguments\n");
        fprintf(stderr, "Usage: ChibiLisp <source file> <executable name>\n");
        return 1;
    }
    char* filepath = argv[1];
    char* executable_name = argv[2];

    //open the file
    FILE* source_file = fopen(filepath, "r");
    if (!source_file) {
        fprintf(stderr, "Error: failed to read source file\n");
        return 1;
    }
    //get the length of the input file
    fseek(source_file, 0, SEEK_END);
    long size = ftell(source_file);
    rewind(source_file);

    //allocate the memory for our source string
    char* source_string = calloc(1, size+1);
    fread(source_string, sizeof(char), size, source_file);

    //close the file
    fclose(source_file);

    //...
}
```


Now we can tokenise and parse the string contents:
```c
int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Error, incorrect arguments\n");
        fprintf(stderr, "Usage: ChibiLisp <source file> <executable name>\n");
        return 1;
    }
    char* filepath = argv[1];
    char* executable_name = argv[2];

    //open the file
    FILE* source_file = fopen(filepath, "r");
    if (!source_file) {
        fprintf(stderr, "Error: failed to read source file\n");
        return 1;
    }
    //get the length of the input file
    fseek(source_file, 0, SEEK_END);
    long size = ftell(source_file);
    rewind(source_file);

    //allocate the memory for our source string
    char* source_string = calloc(1, size+1);
    fread(source_string, sizeof(char), size, source_file);

    //close the file
    fclose(source_file);

    ObjectList program = parse_program(tokenise(source_string));
}
```

Libchibi then provides us two functions for compiling the program: cProg and aProg. Both compile the program however aProg then also assembles it into an object file which, since we definitely want to do that anyway, is the function we will be running. We also need to create a temporary filename which we can do using the C function mkstemps.
```c
int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Error, incorrect arguments\n");
        fprintf(stderr, "Usage: ChibiLisp <source file> <executable name>\n");
        return 1;
    }
    char* filepath = argv[1];
    char* executable_name = argv[2];

    //open the file
    FILE* source_file = fopen(filepath, "r");
    if (!source_file) {
        fprintf(stderr, "Error: failed to read source file\n");
        return 1;
    }
    //get the length of the input file
    fseek(source_file, 0, SEEK_END);
    long size = ftell(source_file);
    rewind(source_file);

    //allocate the memory for our source string
    char* source_string = calloc(1, size+1);
    fread(source_string, sizeof(char), size, source_file);

    //close the file
    fclose(source_file);

    ObjectList program = parse_program(tokenise(source_string));

    char* object_template = "/tmp/chibilisp-XXXXXX.o";
    int fd = mkstemps(object_template, 2);
    if (fd < 0) {
        fprintf(stderr, "Error: failed to make temporary object file\n");
        return 1;
    }
    aProg(program, object_template);
}
```

And now all we have to do is link the object file with libc and the CRT to get our final executable. For this we have a few options but, since we're on Linux, the simplest is just to invoke gcc directly. To do that we need to fork the process, invoke gcc with the correct arguments, wait for linking to finish, then we can clean up the temporary object file (don't worry if you didn't really follow; it's very much not the focus of the book. If you are curious though I can recommend [this](https://www.youtube.com/watch?v=SwIPOf2YAgI) YouTube video as a great explainer on the topic).

```c
int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Error, incorrect arguments\n");
        fprintf(stderr, "Usage: ChibiLisp <source file> <executable name>\n");
        return 1;
    }
    char* filepath = argv[1];
    char* executable_name = argv[2];

    //open the file
    FILE* source_file = fopen(filepath, "r");
    if (!source_file) {
        fprintf(stderr, "Error: failed to read source file\n");
        return 1;
    }
    //get the length of the input file
    fseek(source_file, 0, SEEK_END);
    long size = ftell(source_file);
    rewind(source_file);

    //allocate the memory for our source string
    char* source_string = calloc(1, size+1);
    fread(source_string, sizeof(char), size, source_file);

    //close the file
    fclose(source_file);

    ObjectList program = parse_program(tokenise(source_string));

    char object_template[] = "/tmp/chibilisp-XXXXXX.o";
    int fd = mkstemps(object_template, 2);
    if (fd < 0) {
        fprintf(stderr, "Error: failed to make temporary object file\n");
        return 1;
    }
    close(fd);

    aProg(program, object_template);

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error: failed to fork process\n");
        return 1;
    }
    if (pid == 0) {
        //Child process: exec gcc
        execlp("gcc", "gcc", "-o", executable_name, object_template, NULL);
        //if exec fails:
        fprintf(stderr, "Error: failed to execute gcc");
        _exit(1);
    }
    //Parent process
    //wait for gcc to end
    int status;
    waitpid(pid, &status, 0);
    //check for error with gcc
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Error: gcc failed");
        return 1;
    }

    //clean up the temporary object file
    unlink(object_template);
}
```

There are two more changes we should make to our program in order to get it working properly. Libchibi expects us to do two things before compiling the program that we don't currently do:
  *  resection our objects
  *  register debug files

Resectioning our objects isn't strictly necessary right now since we're only producing function object, however, when we have many objects libchibi expects them to appear in a certain order based on their type (ie, objects of the same type don't change order) and to prevent us from having to manage that manually, potentially causing problems if the language provides them in a different order, Libchibi provides a handy function to do it for us: resection_object.

Registering debug files relates to the debug info we were ignoring throughout our parser. Even though we repeatedly told Libchibi that our nodes were at file 0 and line 0 because we weren't storing any debug info, it does actually still expect us to tell it what file 0 (and, indeed, any other file number we choose to use) actually is (or, rather, what name to give it). The good news is that we actually already have a name for file 0: our filepath. So we can easily construct a DebugFile struct that specifies file 0 has the name of our filepath and register is using the Libchibi function register_debug_files.

Adding these two bits of code before aProg we get:
```c
//compilation
resection_objects(program);
DebugFile df = {.file_name = filepath, .file_num = 0};
register_debug_files(&df, 1);
aProg(program, object_template);
```

And last of all, remember to add the \<unistd.h>, \<stdlib.h>, \<sys/wait.h> and \<stdio.h> header files to the include section at the start of main.c.

Now, if we compile and link with libchibi.a, we should get a finished compiler. And if we run that compiler giving it a test.chbl file containing the ChibiLisp code:
```lisp
(define proc (main (i32 ()) (body (return (i32 69)))))
```
from chapter 2 then we should get an output program. Running that program it should exit immediately. If we then run the command:
```
echo $?
```
we should see the number 69 printed. Our custom error code. And if we recompile test.chbl with a different code and run it then that number will change.

This is your very first program compiled and running in a language you wrote.

In the next chapter we will discuss how to go from this very simple program to having a language that can run hello world.

