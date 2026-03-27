/************************
* File: parser.h
************************/

#ifndef PARSER_H
#define PARSER_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include "helpers.h"
#include "tokeniser.h"
#include "libchibi.h"

ObjectList parse_program(Token_Seq tokens);

DeclareOption(Type, Type_Option);
DeclareOption(Node, Node_Option);
DeclareOption(Object, Object_Option);

DeclareSeq(char*, char_ptr_Seq);

#endif

#ifdef PARSER_C
#undef PARSER_C

Token_Seq tokens;
int idx;

Type_Option parse_base_type();
Type_Option parse_type();

Node_Option parse_value_literal() {
    Node output_node;
    Expect(Node_Option, tokens.data[idx++].kind == TOK_L_PAREN);
    switch (tokens.data[idx++].kind) {
        case (TOK_KEYWORD_I8): {
            Expect(Node_Option, tokens.data[idx].kind == TOK_INTEGER_LITERAL);
            output_node = create_char_literal_node((char)tokens.data[idx++].data.integer_literal, 0, 0);
            //again, no debug info yet
            break;
        } case (TOK_KEYWORD_I16): {
            Expect(Node_Option, tokens.data[idx].kind == TOK_INTEGER_LITERAL);
            output_node = create_short_literal_node((short)tokens.data[idx++].data.integer_literal, 0, 0);
            //again, no debug info yet
            break;
        } case (TOK_KEYWORD_I32): {
            Expect(Node_Option, tokens.data[idx].kind == TOK_INTEGER_LITERAL);
            output_node = create_int_literal_node((int)tokens.data[idx++].data.integer_literal, 0, 0);
            //again, no debug info yet
            break;
        } case (TOK_KEYWORD_I64): {
            Expect(Node_Option, tokens.data[idx].kind == TOK_INTEGER_LITERAL);
            output_node = create_long_literal_node((long)tokens.data[idx++].data.integer_literal, 0, 0);
            //again, no debug info yet
            break;
        } case (TOK_KEYWORD_U8): {
            Expect(Node_Option, tokens.data[idx].kind == TOK_INTEGER_LITERAL);
            output_node = create_char_literal_node((char)tokens.data[idx++].data.integer_literal, 0, 0);
            //again, no debug info yet
            break;
        } case (TOK_KEYWORD_U16): {
            Expect(Node_Option, tokens.data[idx].kind == TOK_INTEGER_LITERAL);
            output_node = create_short_literal_node((short)tokens.data[idx++].data.integer_literal, 0, 0);
            //again, no debug info yet
            break;
        } case (TOK_KEYWORD_U32): {
            Expect(Node_Option, tokens.data[idx].kind == TOK_INTEGER_LITERAL);
            output_node = create_int_literal_node((int)tokens.data[idx++].data.integer_literal, 0, 0);
            //again, no debug info yet
            break;
        } case (TOK_KEYWORD_U64): {
            Expect(Node_Option, tokens.data[idx].kind == TOK_INTEGER_LITERAL);
            output_node = create_long_literal_node((long)tokens.data[idx++].data.integer_literal, 0, 0);
            //again, no debug info yet
            break;
        } case (TOK_KEYWORD_F32): {
            Expect(Node_Option, tokens.data[idx].kind == TOK_INTEGER_LITERAL || tokens.data[idx].kind == TOK_DECIMAL_LITERAL);
            if (tokens.data[idx].kind == TOK_INTEGER_LITERAL) {
                output_node = create_float_literal_node((float)tokens.data[idx++].data.integer_literal, 0, 0);
                //again, no debug info yet
            } else {
                output_node = create_float_literal_node((float)tokens.data[idx++].data.decimal_literal, 0, 0);
                //again, no debug info yet
            }
            break;
        } case (TOK_KEYWORD_F64): {
            Expect(Node_Option, tokens.data[idx].kind == TOK_INTEGER_LITERAL || tokens.data[idx].kind == TOK_DECIMAL_LITERAL);
            if (tokens.data[idx].kind == TOK_INTEGER_LITERAL) {
                output_node = create_double_literal_node((double)tokens.data[idx++].data.integer_literal, 0, 0);
                //again, no debug info yet
            } else {
                output_node = create_double_literal_node((double)tokens.data[idx++].data.decimal_literal, 0, 0);
                //again, no debug info yet
            }
            break;
        } case (TOK_KEYWORD_F80): {
            Expect(Node_Option, tokens.data[idx].kind == TOK_INTEGER_LITERAL || tokens.data[idx].kind == TOK_DECIMAL_LITERAL);
            if (tokens.data[idx].kind == TOK_INTEGER_LITERAL) {
                output_node = create_long_double_literal_node((long double)tokens.data[idx++].data.integer_literal, 0, 0);
                //again, no debug info yet
            } else {
                output_node = create_long_double_literal_node((long double)tokens.data[idx++].data.decimal_literal, 0, 0);
                //again, no debug info yet
            }
            break;
        } case (TOK_KEYWORD_BOOL): {
            Expect(Node_Option, tokens.data[idx].kind == TOK_BOOL_LITERAL);
            output_node = create_int_literal_node((int)tokens.data[idx++].data.bool_literal, 0, 0);
            //again, no debug info yet
            break;
        } case (TOK_KEYWORD_STRING): {
            fprintf(stderr, "Error: string literals not yet supported\n");
            exit(1);
            //supporting string literals requires declaring global variables which we haven't implemented yet
            break;
        } default: Expect(Node_Option, false);
    }
    Expect(Node_Option, tokens.data[idx++].kind == TOK_R_PAREN);
    Node_Option output = Just(Node_Option, output_node);
    return output;
}

Type_Option parse_declaration(char** name) {
    Expect(Type_Option, tokens.data[idx++].kind == TOK_L_PAREN);
    Type_Option decl_type = parse_base_type();
    Expect(Type_Option, decl_type.present);
    Expect(Type_Option, tokens.data[idx].kind == TOK_IDENTIFIER);
    //don't consume the token yet as we still need the identifier string
    char* name_temp = tokens.data[idx++].data.identifier;
    //don't modify name until we know we won't fail
    Expect(Type_Option, tokens.data[idx++].kind == TOK_R_PAREN);
    *name = name_temp;
    return decl_type;
}

Type_Option parse_base_type() {
    TokenKind base_type_tokens[] = {
        TOK_KEYWORD_F80, TOK_KEYWORD_F64,
        TOK_KEYWORD_F32, TOK_KEYWORD_U64,
        TOK_KEYWORD_U32, TOK_KEYWORD_U16,
        TOK_KEYWORD_U8, TOK_KEYWORD_I64,
        TOK_KEYWORD_I32, TOK_KEYWORD_I16,
        TOK_KEYWORD_I8, TOK_KEYWORD_BOOL};
    BaseType base_types[] = {
        BTY_LDOUBLE, BTY_DOUBLE,
        BTY_FLOAT, BTY_LONG,
        BTY_INT, BTY_SHORT,
        BTY_CHAR, BTY_LONG,
        BTY_INT, BTY_SHORT,
        BTY_CHAR, BTY_INT};
    //BaseType is a type provided by Libchibi to represent base types
    Type_Option output = Nothing;
    for (int i = 0; i < 12; i++) {
        if (tokens.data[idx].kind == base_type_tokens[i]) {
            output = Just(Type_Option, create_base_type(base_types[i]));
            idx++;
        }
    }
    if (!output.present && tokens.data[idx++].kind == TOK_KEYWORD_STRING) 
        output = Just(Type_Option, create_ptr_type(create_base_type(BTY_CHAR)));
    
    return output;
}

Type_Option parse_proc_type(char_ptr_Seq* arg_names) {
    Expect(Type_Option, tokens.data[idx++].kind == TOK_L_PAREN);
    Type_Option return_type = parse_type();
    Expect(Type_Option, return_type.present);
    Expect(Type_Option, tokens.data[idx++].kind == TOK_L_PAREN);

    TypeList arg_types = empty_list;
    //TypeList is a type provided by Libchibi to store lists of types
    char_ptr_Seq arg_names_temp = {0};
    while (tokens.data[idx].kind != TOK_R_PAREN) {
        Type_Option arg_type;
        char* arg_name;
        arg_type = parse_declaration(&arg_name);
        Expect(Type_Option, arg_type.present);
        append(char_ptr_Seq, arg_names_temp, arg_name);
        append_type(arg_types, arg_type.value);
    }

    Expect(Type_Option, tokens.data[idx++].kind == TOK_R_PAREN);
    Expect(Type_Option, tokens.data[idx++].kind == TOK_R_PAREN);

    for (int i = 0; i < arg_names_temp.len; i++) {
        append(char_ptr_Seq, *arg_names, arg_names_temp.data[i]);
    }
    //append to arg_names after all Expects so we know we won't fail
    
    Type output_type = create_function_type(return_type.value, arg_types);
    Type_Option output = Just(Type_Option, output_type);
    return output;
}

Type_Option parse_type() {
    int current_idx = idx;
    Type_Option output;

    char_ptr_Seq arg_names = {0};
    //this must be provided to parse_proc_type but will be unused

    output = parse_proc_type(&arg_names);
    if (output.present) {
        return output;
    } else idx = current_idx;

    output = parse_base_type();
    if (output.present) {
        return output;
    } else idx = current_idx;

    return (Type_Option)Nothing;
}

Node_Option parse_return_statement() {
    Expect(Node_Option, tokens.data[idx++].kind == TOK_L_PAREN);
    Expect(Node_Option, tokens.data[idx++].kind == TOK_KEYWORD_RETURN);
    Node_Option value = parse_value_literal();
    Expect(Node_Option, value.present);
    Expect(Node_Option, tokens.data[idx++].kind == TOK_R_PAREN);

    Node output_node = create_return_node(value.value, 0, 0);
    //again, no debug info
    Node_Option output = Just(Node_Option, output_node);
    return output;
}

Node_Option parse_statement(char_ptr_Seq* local_names, TypeList* local_types) {
    int current_idx = idx;
    Node_Option output;

    output = parse_return_statement();
    //parse_return_statement doesn't need the local names/types Seqs since it can't declare local variables
    if (output.present) {
        return output;
    } else idx = current_idx;

    return (Node_Option)Nothing;
}

Node_Option parse_proc_body(char_ptr_Seq* local_names, TypeList* local_types) {
    Expect(Node_Option, tokens.data[idx++].kind == TOK_L_PAREN);
    Expect(Node_Option, tokens.data[idx++].kind == TOK_KEYWORD_BODY);
    NodeList body_list = empty_list;
    //NodeList is a type provided by Libchibi to represent lists of AST nodes
    while(tokens.data[idx].kind != TOK_R_PAREN) {
        Node_Option new_node = parse_statement(local_names, local_types);
        Expect(Node_Option, new_node.present);
        append_node(body_list, new_node.value);
        //append_node is a function provided by Libchibi to add nodes to node lists
    }
    Expect(Node_Option, tokens.data[idx++].kind == TOK_R_PAREN);
    
    Node body_node = create_block_node(body_list, 0, 0);
    //again we have no debug info yet form the tokeniser
    Node_Option output = Just(Node_Option, body_node);
    return output;
}

Object_Option parse_proc_definition_inner() {
    Expect(Object_Option, tokens.data[idx++].kind == TOK_L_PAREN);
    Expect(Object_Option, tokens.data[idx].kind == TOK_IDENTIFIER);
    //we don't consume this token yet because we still need to get the identifier string
    char* name = tokens.data[idx++].data.identifier;
    char_ptr_Seq arg_names = {0};
    Type_Option proc_type = parse_proc_type(&arg_names);
    //Type is a type provided by Libchibi to represent types
    Expect(Object_Option, proc_type.present);
    char_ptr_Seq local_names = {0};
    TypeList local_types = empty_list;
    Node_Option proc_body = parse_proc_body(&local_names, &local_types);
    //Node is a type provided by Libchibi to represent AST nodes
    Expect(Object_Option, proc_body.present);
    Expect(Object_Option, tokens.data[idx++].kind == TOK_R_PAREN);
    Object_Option output = Just(Object_Option, create_function_definition(name, proc_type.value, arg_names.len, arg_names.data, local_names.len, local_types, local_names.data, proc_body.value));
    return output;
}

Object_Option parse_proc_definition() {
    Object_Option output;
    Expect(Object_Option, tokens.data[idx++].kind == TOK_L_PAREN);
    Expect(Object_Option, tokens.data[idx++].kind == TOK_KEYWORD_DEFINE);
    Expect(Object_Option, tokens.data[idx++].kind == TOK_KEYWORD_PROC);
    output = parse_proc_definition_inner();
    Expect(Object_Option, output.present);
    Expect(Object_Option, tokens.data[idx++].kind == TOK_R_PAREN);
    return output;
}

Object_Option parse_definition() {
    int current_idx = idx;
    Object_Option output;

    output = parse_proc_definition();
    if (output.present) {
        return output;
    } else idx = current_idx;

    return (Object_Option)Nothing;
}

Object parse_object() {
    int current_idx = idx;
    Object_Option output;

    output = parse_definition();
    if (output.present) {
        return output.value;
    } else idx = current_idx;

    fprintf(stderr, "Error: failed to parse object\n");
    exit(1);
}

ObjectList parse_program(Token_Seq tokens_arg) {
    tokens = tokens_arg;
    idx = 0;
    ObjectList output = empty_list; //empty_list is a macro provided by Libchibi to represent an empty list
    while (tokens.data[idx].kind != TOK_EOF) append_object(output, parse_object());
    //append_object is a function provided by Libchibi to append objects to object lists
    return output;
}

#endif