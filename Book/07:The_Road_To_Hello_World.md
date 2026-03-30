<h1 align=center>Chapter 07: The Road To Hello World</h1>

Last chapter we compiled our first ChibiLisp program to quit with a custom exit code. But as anyone who has learned a programming language that _wasn't_ assembly can tell you, the **true** first program of any language is the humble Hello World. So how do we do that?

Well, we're obviously going to need a way to print to the console and for now I'm going to suggest that we actually use printf because we're already linking with libc; when we write our own standard library we can wrap it or create our own alternative then. To use printf we're therefore going to need a way to call procedures (even though printf is a C function, in ChibiLisp it is a procedure). And a way to tell the language about external procedures we're linking against. We're also going to need string literals which, if you remember from chapter 5, also requires us to introduce global variables. While we're at it, we may as well add a few more features as well. Let's say that our aim now will be to add these features to ChibiLisp:

  *  procedure calls
  *  external procedure declaration
  *  global variable declarations
  *  global variable definitions
  *  string literals
  *  local variable declarations
  *  variable assignment
  *  if statements
  *  if-elif-else statements
  *  block statements
  *  while loops
  *  break and continue statements
  *  addition
  *  subtraction
  *  multiplication
  *  division
  *  modulo
  *  logical AND
  *  logical OR
  *  logical XOR
  *  logical NOT
  *  bitwise AND
  *  bitwise OR
  *  bitwise XOR
  *  bitwise NOT
  *  shift left
  *  shift right
  *  equality testing
  *  non-equality testing
  *  inequality testing (<, <=, >, >=)
  *  variable literals
  *  procedures with void return type
  *  pointers
  *  memory allocation
  *  sizeof

That seems like a lot but as you'll see it's actually no more complex than what we have already built. In fact, that is the exact reason I've chosen to introduce so many new features at once; there's almost nothing here that you can't already do with what you've learned up to this point.

Also note that, for the moment, our feature set is oriented much more towards writing C-style imperative code. Later in the book we'll introduce more features for supporting functional code.

For each feature, we can go through, decide what that feature looks like and then adjust our grammar accordingly:

### Procedure Calls:
```lisp
(<function name> <arg 1> <arg 2> <arg 3> ...)
```
We can then add this symbol to our parser grammar:
```EBNF
<proc_call> ::= "l_paren" "identifier"
                <value_literal>* "r_paren";
```
And update our statement symbol:
```EBNF
<statement> ::= <return_statement> | <proc_call>;
```
\
<br>
### External Procedure Declarations:
```lisp
(extern proc (<procedure name> (<return type>
    ((<arg type 1> <arg name 1>) (<arg type 2> <arg name 2>) ...)
)))
```
We can then add these symbols to our parser grammar:
```EBNF
<object_declaration> ::= <proc_declaration>;
<proc_declaration> ::= <extern_proc_decl>;
<extern_proc_decl> ::= "l_paren" "keyword_extern" "keyword_proc"
                       <extern_proc_decl_inner> "r_paren";
<extern_proc_decl_inner> ::= "l_paren" "identifier"
                             <proc_type> "r_paren";
```
And update our object symbol:
```EBNF
<object> ::= <definition> | <object_declaration>;
``` 
Not forgetting to add this symbol to our tokeniser grammar as well:
```EBNF
<keyword_extern> ::= "extern";
```
\
<br>
### Global Variable Declarations:
```lisp
(var (<type> <name>))
```
We can then add this symbol to our parser grammar:
```EBNF
<var_declaration> ::= "l_paren" "keyword_var"
                       <declaration> "r_paren";
```
Update our object_declaration symbol:
```EBNF
<object_declaration> ::= <proc_declaration> | <var_declaration>;
```
And add this symbol to our tokeniser grammar:
```EBNF
<keyword_var> ::= "var";
```
\
<br>
### Global Variable Definitions:
```lisp
(define var (<type> <name>) (<type> <value>))
```
We can then add this symbol to our parser grammar:
```EBNF
<var_definition> ::= "l_paren" "keyword_define" "keyword_var"
                     <declaration> <value_literal> "r_paren";
```
And update our definition symbol:
```EBNF
<definition> ::= <proc_definition> | <var_definition>;
```
\
<br>
### String Literals:
We actually already have the grammar for this so all we have to do is adapt the code for global variables to be able to correctly form the AST.
\
<br>
### Local Variable Declarations:
This will look exactly the same as a global variable declaration except within a scope. We can therefore update our statement symbol:
```EBNF
<statement> ::= <return_statement> | <proc_call>
              | <var_declaration>;
```
\
<br>
### Variable Assignment:
```lisp
(assign <variable name> (<type> <value>))
```
We can then add this symbol to our parser grammar:
```EBNF
<var_assign_statement> ::= "l_paren" "keyword_assign"
                           "identifier" <value_literal>
                           "r_paren";
```
Update our statement symbol:
```EBNF
<statement> ::= <return_statement> | <proc_call>
              | <var_declaration> | <var_assign_statement>;
```
And add this symbol to our tokeniser grammar:
```EBNF
<keyword_assign> ::= "assign";
```
\
<br>
### If Statements:
```lisp
(if (bool <true or false>) <conditional statement>)
```
Note: when we introduce expressions later this chapter other than value literals this will become more useful.

We can then add this symbol to our parser grammar:
```EBNF
<if_statement> ::= "l_paren" "keyword_if" <value_literal>
                   <statement> "r_paren";
```
Update our statement symbol:
```EBNF
<statement> ::= <return_statement> | <proc_call>
              | <var_declaration> | <var_assign_statement>
              | <if_statement>;
```
And add this symbol to our tokeniser grammar:
```EBNF
<keyword_if> ::= "if";
```
\
<br>
### If-Elif-Else Statement
```lisp
(if (bool <value true or false>) <statement> 
elif (bool) <value true or false> <statement>
else <statement>)
```
We can update our if_statement symbol to this:
```EBNF
<if_statement> ::= "l_paren" "keyword_if" <value_literal>
                   <statement> ("keyword_elif"
                   <value_literal> <statement>)*
                   ["keyword_else" <statement>]
                   "r_paren";
```
If you remember from chapter 1 these square brackets represent optional symbols.

Last, as usual, add these symbols to our tokeniser grammar:
```EBNF
<keyword_elif> ::= "elif";
<keyword_else> ::= "else";
```
\
<br>
### Block Statements:
we're actually going to use the existing proc_body symbol but rename it to body_statement and update our statement symbol accordingly (don't forget to change it's name in proc_definition_inner as well):
```EBNF
<statement> ::= <return_statement> | <proc_call>
              | <var_declaration> | <var_assign_statement>
              | <if_statement> | <body_statement>;
```
\
<br>
### While Loops:
```lisp
(while (bool <true or false>) <conditional statement>)
```
We can then add this symbol to our parser grammar:
```EBNF
<while_loop> ::= "l_paren" "keyword_while" <value_literal>
                   <statement> "r_paren";
```
Update our statement symbol:
```EBNF
<statement> ::= <return_statement> | <proc_call>
              | <var_declaration> | <var_assign_statement>
              | <if_statement> | <body_statement>
              | <while_loop>;
```
And add this symbol to our tokeniser grammar:
```EBNF
<keyword_while> ::= "while";
```
\
<br>
### Break and Continue Statements:
```lisp
break
```
and
```lisp
continue
```
We can then add these symbols to our parser grammar:
```EBNF
<break_statement> ::= "keyword_break";
<continue_statement> ::= "keyword_continue";
```
Update our statement symbol:
```EBNF
<statement> ::= <return_statement> | <proc_call>
              | <var_declaration> | <var_assign_statement>
              | <if_statement> | <body_statement>
              | <while_loop> | <break_statement>
              | <continue_statement>;
```
And add these symbols to our tokeniser grammar:
```EBNF
<keyword_break> ::= "break";
<keyword_continue> ::= "continue";
```
\
<br>
### Addition:
Before we implement addition we need to replace every reference, except for variable definition, to the value_literal symbol with a reference to the expression symbol. We will then repeatedly expand this symbol with different options throughout this chapter:
```EBNF
<expression> ::= <value_literal>;
```
Our addition expressions look like this:
```lisp
(+ <lhs expression> <rhs expression>)
```
Meaning we can then add this symbol to our parser grammar:
```EBNF
<add_expression> ::= "l_paren" "symbol_plus" <expression>
                     <expression> "r_paren";
```
Update our expression symbol:
```EBNF
<expression> ::= <value_literal> | <add_expression>;
```
And add this symbol to our parser grammar:
```EBNF
<symbol_plus> ::= "+";
```
\
<br>
### Subtraction, Multiplication, Division, and Modulo:
These look exactly the same as addition but with their respective symbols meaning we can add these symbols to our parser grammar:
```EBNF
<sub_expression> ::= "l_paren" "symbol_minus" <expression>
                     <expression> "r_paren";
<mul_expression> ::= "l_paren" "symbol_times" <expression>
                     <expression> "r_paren";
<div_expression> ::= "l_paren" "symbol_divide" <expression>
                     <expression> "r_paren";
<mod_expression> ::= "l_paren" "symbol_modulo" <expression>
                     <expression> "r_paren";
```
Update our expression symbol:
```EBNF
<expression> ::= <value_literal> | <add_expression>
               | <sub_expression> | <mul_expression>
               | <div_expression> | <mod_expression>;
```
And add these symbols to our token grammar:
```EBNF
<symbol_minus> ::= "-";
<symbol_times> ::= "*";
<symbol_divide> ::= "/";
<symbol_modulo> ::= "%";
```
\
<br>
### Logical AND, OR, NOT, and XOR
```lisp
(log <and, or, xor> <lhs expression> <rhs expression>)
(log not <expression>)
```
Note that we use the words 'and', 'or', 'not' and 'xor' since the logical symbols for these: '∧', '∨', '¬', and '⊻' are not easy to type on a normal keyboard (and it fits lisp-style syntax better to not use C's alternatives).

We can then add these symbols to our parser grammar:
```EBNF
<log_and_expr> ::= "l_paren" "keyword_log" "keyword_and"
                   <expression> <expression> "r_paren";
<log_or_expr> ::= "l_paren" "keyword_log" "keyword_or"
                  <expression> <expression> "r_paren";
<log_not_expr> ::= "l_paren" "keyword_log" "keyword_not"
                   <expression> "r_paren";
<log_xor_expr> ::= "l_paren" "keyword_log" "keyword_xor"
                   <expression> <expression> "r_paren";
```
Update our expression symbol:
```EBNF
<expression> ::= <value_literal> | <add_expression>
               | <sub_expression> | <mul_expression>
               | <div_expression> | <mod_expression>
               | <log_and_expr> | <log_or_expr>
               | <log_not_expr> | <log_xor_expr>;
```
And add these symbols to our tokeniser grammar:
```EBNF
<keyword_log> ::= "log";
<keyword_and> ::= "and";
<keyword_or> ::= "or";
<keyword_not> ::= "not";
<keyword_xor> ::= "xor";
```
Note that we use the keyword log to distinguish the logical and bitwise versions rather than, say, using the symbolic and keyword versions (eg, '&' vs 'and') because that makes it explicit which is which and does not require the programmer to memorise which representation is logical vs bitwise.
\
<br>
### Bitwise AND, OR, NOT, and XOR:
```lisp
(log <and, or, xor> <lhs expression> <rhs expression>)
(log not <expression>)
```
We can then add these symbols to our parser grammar:
```EBNF
<bit_and_expr> ::= "l_paren" "keyword_bit" "keyword_and"
                   <expression> <expression> "r_paren";
<bit_or_expr> ::= "l_paren" "keyword_bit" "keyword_or"
                  <expression> <expression> "r_paren";
<bit_not_expr> ::= "l_paren" "keyword_bit" "keyword_not"
                   <expression> "r_paren";
<bit_xor_expr> ::= "l_paren" "keyword_bit" "keyword_xor"
                   <expression> <expression> "r_paren";
```
Update our expression symbol:
```EBNF
<expression> ::= <value_literal> | <add_expression>
               | <sub_expression> | <mul_expression>
               | <div_expression> | <mod_expression>
               | <log_and_expr> | <log_or_expr>
               | <log_not_expr> | <log_xor_expr>
               | <bit_and_expr> | <bit_or_expr>
               | <bit_not_expr> | <bit_xor_expr>;
```
And add this symbols to our tokeniser grammar:
```EBNF
<keyword_bit> ::= "bit";
```
\
<br>
### Shift Left and Shift Right:
```lisp
(<shl or shr> <value to shift> <amount to shift by>)
```
We can then add these symbols to our parser grammar:
```EBNF
<shl_expression> ::= "l_paren" "keyword_shl" <expression>
                     <expression> "r_paren";
<shr_expression> ::= "l_paren" "keyword_shr" <expression>
                     <expression> "r_paren";
```
Update our expression symbol:
```EBNF
<expression> ::= <value_literal> | <add_expression>
               | <sub_expression> | <mul_expression>
               | <div_expression> | <mod_expression>
               | <log_and_expr> | <log_or_expr>
               | <log_not_expr> | <log_xor_expr>
               | <bit_and_expr> | <bit_or_expr>
               | <bit_not_expr> | <bit_xor_expr>
               | <shl_expression> | <shr_expression>;
```
And add these symbols to our tokeniser grammar:
```EBNF
<keyword_shl> ::= "shl";
<keyword_shr> ::= "shr";
```
\
<br>
### Equality and Non-Equality Testing:
```lisp
(= <lhs expression> <rhs expression>)
(/= <lhs expression> <rhs expression>)
```
We can then add these symbols to our parser grammar:
```EBNF
<eq_expression> ::= "l_paren" "symbol_eq" <expression>
                     <expression> "r_paren";
<neq_expression> ::= "l_paren" "symbol_neq" <expression>
                     <expression> "r_paren";
```
Update our expression symbol:
```EBNF
<expression> ::= <value_literal> | <add_expression>
               | <sub_expression> | <mul_expression>
               | <div_expression> | <mod_expression>
               | <log_and_expr> | <log_or_expr>
               | <log_not_expr> | <log_xor_expr>
               | <bit_and_expr> | <bit_or_expr>
               | <bit_not_expr> | <bit_xor_expr>
               | <shl_expression> | <shr_expression>
               | <eq_expression> | <neq_expression>;
```
And add these symbols to our tokeniser grammar:
```EBNF
<symbol_eq> ::= "=";
<symbol_neq> ::= "/=";
```
\
<br>
### Inequality Testing:
```lisp
(< <lhs expression> <rhs expression>)
(<= <lhs expression> <rhs expression>)
(> <lhs expression> <rhs expression>)
(>= <lhs expression> <rhs expression>)
```
We can then add these symbols to our parser grammar:
```EBNF
<gt_expression> ::= "l_paren" "symbol_gt" <expression>
                    <expression> "r_paren";
<ge_expression> ::= "l_paren" "symbol_ge" <expression>
                    <expression> "r_paren";
<lt_expression> ::= "l_paren" "symbol_lt" <expression>
                    <expression> "r_paren";
<le_expression> ::= "l_paren" "symbol_le" <expression>
                    <expression> "r_paren";
```
Update our expression symbol:
```EBNF
<expression> ::= <value_literal> | <add_expression>
               | <sub_expression> | <mul_expression>
               | <div_expression> | <mod_expression>
               | <log_and_expr> | <log_or_expr>
               | <log_not_expr> | <log_xor_expr>
               | <bit_and_expr> | <bit_or_expr>
               | <bit_not_expr> | <bit_xor_expr>
               | <shl_expression> | <shr_expression>
               | <eq_expression> | <neq_expression>
               | <gt_expression> | <ge_expression>
               | <lt_expression> | <le_expression>;
```
And add these symbols to our tokeniser grammar:
```EBNF
<symbol_gt> ::= ">";
<symbol_ge> ::= ">=";
<symbol_lt> ::= "<";
<symbol_le> ::= "<=";
```
\
<br>
### Variable Literals:
literally just the variable name. We can therefore add this symbol to our parser grammar:
```EBNF
<variable_literal> ::= "identifier";
```
And update the expression symbol, although while we're at it let's add procedure calls as an expression type (since procedure calls return a value):
```EBNF
<expression> ::= <value_literal> | <add_expression>
               | <sub_expression> | <mul_expression>
               | <div_expression> | <mod_expression>
               | <log_and_expr> | <log_or_expr>
               | <log_not_expr> | <log_xor_expr>
               | <bit_and_expr> | <bit_or_expr>
               | <bit_not_expr> | <bit_xor_expr>
               | <shl_expression> | <shr_expression>
               | <eq_expression> | <neq_expression>
               | <gt_expression> | <ge_expression>
               | <lt_expression> | <le_expression>
               | <variable_literal> | <proc_call>;
```
\
<br>
### Procedures With Void Return Type:
We can just update our proc_type symbol:
```EBNF
<proc_type> ::= "l_paren" (<type> | "keyword_void")
                "l_paren" <declaration>* "r_paren"
                "r_paren";
```
And add this symbol to our tokeniser grammar:
```EBNF
<keyword_void> ::= "void";
```
\
<br>
### Pointers:
```lisp
(ptr to <value>)
(from ptr <value>)
(ptr <type>)
```
Note how these mirror the ability to create pointer types, pointers to values, and to dereference pointers in C.

We can then add these symbols to our parser grammar:
```EBNF
<pointer_value> ::= "l_paren" "keyword_ptr" "keyword_to"
                    <expression> "r_paren";
<deref_value> ::= "l_paren" "keyword_from" "keyword_ptr"
                  <expression> "r_paren";
<pointer_type> ::= "l_paren" "keyword_ptr" <type> "r_paren";
```
Update our expression and type symbols:
```EBNF
<expression> ::= <value_literal> | <add_expression>
               | <sub_expression> | <mul_expression>
               | <div_expression> | <mod_expression>
               | <log_and_expr> | <log_or_expr>
               | <log_not_expr> | <log_xor_expr>
               | <bit_and_expr> | <bit_or_expr>
               | <bit_not_expr> | <bit_xor_expr>
               | <shl_expression> | <shr_expression>
               | <eq_expression> | <neq_expression>
               | <gt_expression> | <ge_expression>
               | <lt_expression> | <le_expression>
               | <variable_literal> | <proc_call>
               | <pointer_value> | <deref_value>;

<type> ::= <proc_type> | <base_type> | <pointer_type>;
```
And add these symbols to our tokeniser grammar:
```EBNF
<keyword_ptr> ::= "ptr";
<keyword_to> ::= "to";
<keyword_from> ::= "from";
```
Note that we still dereference from a value rather than a pointer value because a variable could store a pointer. While we're at it let's make sure that's the case be refactoring our grammar for declarations to use base or pointer types rather than typing everything our by hand again:
```EBNF
<declaration> ::= "l_paren" (<base_type> | <pointer_type>)
                  "identifier" "r_paren";
```
\
<br>
### Memory Allocation:
```lisp
(allocate <number to allocate> of <number of bytes to allocate>)
(free <pointer to free>)
```
We can then add this symbol to our parser grammar:
```EBNF
<allocation> ::= "l_paren" "keyword_allocate" <expression>
                 "keyword_of" <expression> "r_paren";
<free_statement> ::= "l_paren" "keyword_free" <expression>
                     "r_paren";
```
Update our expression and statement symbols:
```EBNF
<expression> ::= <value_literal> | <add_expression>
               | <sub_expression> | <mul_expression>
               | <div_expression> | <mod_expression>
               | <log_and_expr> | <log_or_expr>
               | <log_not_expr> | <log_xor_expr>
               | <bit_and_expr> | <bit_or_expr>
               | <bit_not_expr> | <bit_xor_expr>
               | <shl_expression> | <shr_expression>
               | <eq_expression> | <neq_expression>
               | <gt_expression> | <ge_expression>
               | <lt_expression> | <le_expression>
               | <variable_literal> | <proc_call>
               | <pointer_value> | <deref_value>
               | <allocation>;

<statement> ::= <return_statement> | <proc_call>
              | <var_declaration> | <var_assign_statement>
              | <if_statement> | <body_statement>
              | <while_loop> | <break_statement>
              | <continue_statement> | <free_statement>;
```
And add these symbols to our tokeniser grammar:
```EBNF
<keyword_allocate> ::= "allocate";
<keyword_of> ::= "of";
<keyword_free> ::= "free";
```
\
<br>
### Sizeof
```lisp
(sizeof <type>)
```
We can then add this symbol to our parser grammar:
```EBNF
<sizeof> ::= "l_paren" "keyword_sizeof" <type> "r_paren";
```
Update our expression symbol:
```EBNF
<expression> ::= <value_literal> | <add_expression>
               | <sub_expression> | <mul_expression>
               | <div_expression> | <mod_expression>
               | <log_and_expr> | <log_or_expr>
               | <log_not_expr> | <log_xor_expr>
               | <bit_and_expr> | <bit_or_expr>
               | <bit_not_expr> | <bit_xor_expr>
               | <shl_expression> | <shr_expression>
               | <eq_expression> | <neq_expression>
               | <gt_expression> | <ge_expression>
               | <lt_expression> | <le_expression>
               | <variable_literal> | <proc_call>
               | <pointer_value> | <deref_value>
               | <allocation> | <sizeof>;
```
And add this symbol to our tokeniser grammar:
```EBNF
<keyword_sizeof> ::= "sizeof";
```
\
<br>
And that's it! We covered a lot of ground this chapter and I encourage you to look back over it, try to understand why we might've made some of the design decisions we did and what the alternatives could've been. Make sure, as well, that you can understand the grammar for each of our new features. Next chapter we will go through and implement each of these features.

