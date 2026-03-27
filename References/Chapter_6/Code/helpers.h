
/************************
* File: helpers.h
************************/

#ifndef HELPERS_H
#define HELPERS_H



#define DeclareSeq(T, Name) typedef struct {T* data; long len; long cap;} Name
#define append(Name, SEQ, VAL) do {\
Name* seq = &SEQ;\
if(seq->len >= seq->cap) {\
    if(seq->cap == 0) seq->cap = 1;\
    void* old_data = seq->data;\
    seq->cap *= 2;\
    seq->data = calloc(seq->cap, sizeof(VAL));\
    if (old_data != NULL) {\
        memcpy(seq->data, old_data, seq->len*sizeof(VAL));\
        free(old_data);\
    }\
}\
seq->data[seq->len] = VAL;\
seq->len += 1;\
} while (0)

int string_in_array(char* string, char* array[], int array_len);


#define DeclareOption(T, Name) typedef struct{bool present; T value;} Name
#define Nothing {.present = false}
#define Just(Name, V) (Name){.present = true, .value = V}
#define Expect(Name, E) do {if (!(E)) return (Name)Nothing;} while(false)

#endif

#ifdef HELPERS_C
#undef HELPERS_C

int string_in_array(char* string, char* array[], int array_len) {
    for (int i = 0; i < array_len; i++) {
        if (strcmp(string, array[i]) == 0) return i;
    }
    return -1;
}

#endif

