#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "utils.h"

struct error_location_t {
    char *file_name; /* the file name where the error occurs */
    int line; /* the line index in the file */
    int index; /* the character index in the current line */
};

OPCODE_TYPE get_opcode_type(char str[]) {
    if(strcmp(str, "mov") == 0)
        return MOV;
    if(strcmp(str, "cmp") == 0)
        return CMP;
    if(strcmp(str, "add") == 0)
        return ADD;
    if(strcmp(str, "sub") == 0)
        return SUB;
    if(strcmp(str, "not") == 0)
        return NOT;
    if(strcmp(str, "clr") == 0)
        return CLR;
    if(strcmp(str, "lea") == 0)
        return LEA;
    if(strcmp(str, "inc") == 0)
        return INC;
    if(strcmp(str, "dec") == 0)
        return DEC;
    if(strcmp(str, "jmp") == 0)
        return JMP;
    if(strcmp(str, "bne") == 0)
        return BNE;
    if(strcmp(str, "red") == 0)
        return RED;
    if(strcmp(str, "prn") == 0)
        return PRN;
    if(strcmp(str, "jsr") == 0)
        return JSR;
    if(strcmp(str, "rts") == 0)
        return RTS;
    if(strcmp(str, "stop") == 0)
        return STOP;
    return OPCODE_UNKOWN;
}

char *get_opcode_name(OPCODE_TYPE opcode_type) {
    if(opcode_type == MOV)
        return "mov";
    if(opcode_type == CMP)
        return "cmp";
    if(opcode_type == ADD)
        return "add";
    if(opcode_type == SUB)
        return "sub";
    if(opcode_type == NOT)
        return "not";
    if(opcode_type == CLR)
        return "clr";
    if(opcode_type == LEA)
        return "lea";
    if(opcode_type == INC)
        return "inc";
    if(opcode_type == DEC)
        return "dec";
    if(opcode_type == JMP)
        return "jmp";
    if(opcode_type == BNE)
        return "bne";     
    if(opcode_type == RED)
        return "red";
    if(opcode_type == PRN)
        return "prn";
    if(opcode_type == JSR)
        return "jsr";
    if(opcode_type == RTS)
        return "rts";                                                                                                           
    if(opcode_type == STOP)
        return "stop";
    return NULL;    
}

int is_opcode_parameter_addresing(OPCODE_TYPE opcode) {
    return opcode == JMP || opcode == BNE || opcode == JSR;
}

int is_opcode_group_one(OPCODE_TYPE opcode) {
    return opcode == MOV || opcode == CMP || opcode == ADD || opcode == SUB || opcode == LEA; 
}

int is_opcode_group_two(OPCODE_TYPE opcode) {
    return opcode == NOT || opcode == CLR || opcode == INC || opcode == DEC || opcode == JMP 
        || opcode == BNE || opcode == RED || opcode == PRN || opcode == JSR;
}

int is_opcode_group_three(OPCODE_TYPE opcode) {
    return opcode == RTS || opcode == STOP;
}

error_location *create_empty_error_location(char *file_name) {
    error_location *error_info = malloc_and_check(sizeof(error_location));
    error_info->file_name = file_name;
    error_info->line = 0;
    error_info->index = 0;
    return error_info;
}

void error_location_ascend_line(error_location *error) {
    if(error != NULL)
        error->line++;
}

void error_location_set_index(error_location *error, int index) {
    if(error != NULL)
        error->index = index;
}

int print_error_location(error_location *error, char *error_message, ...) {
    va_list args;
    int count;
    if(error != NULL)
        printf("Error(%s:%i:%i): ", error->file_name, error->line, error->index);
    if(error_message != NULL) { /* use the function vprintf with va_list to decode the additional variables the function received (if there are) with the error message */
        va_start(args, error_message); /* starts the va_list with the error_message */
        count = vprintf(error_message, args); /* get the number of characters that are printed, if an error occurs then it is a negative number */
        va_end(args); /* closes the va_list */
        printf("\n");
    }
    return count;
}

void free_error_location(error_location *error) {
    if(error == NULL) return;
    free(error->file_name);
    free(error);
}

int is_legal_register(char str[]) {
    if(str[0] == 'r' && str[2] == '\0') { /* a legal register name length is 2 and the first character is 'r' */
        if(isdigit(str[1]) && (str[1] - '0') < MAX_REGISTERS) /* checks if the the second character is a digit and is smaller than the biggest register digit: MAX_REGISTERS*/
            return str[1]; /* the value of the '0' is bigger than 0 therefore an error will not occur with the value returned */
    }
    return false;
}

int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r';
}

int is_end_null(char c) {
    return c == '\n' || c == '\0'; 
}

int is_integer(char str[]) {
    int check = true;
    int length = 0;
    if(str[0] == '+' || str[0] == '-') { /* the first character of an integer can only be: '+', '-' or a digit */
        length++;
        if(str[length] == '\0')
            check = 0;
    }
    while(str[length] != '\0' && check != false) /* every other character must be a digit */
        check = isdigit(str[length++]);
    return check;
}

char *get_file_full_name(char *file_name, char *new_extension) {
    char *file_full_name;
    if(file_name == NULL || new_extension == NULL)
        return NULL;
    file_full_name = (char*) malloc_and_check(strlen(file_name) + strlen(new_extension) + 1); /* the length of the 2 string inluding '\0' at the end */
    file_full_name[0] = '\0'; /* sets the string to be empty */
    strcpy(file_full_name, file_name);
    strcat(file_full_name, new_extension);
    return file_full_name;
}

FILE *open_file(char *file_name, char *new_extension, char *file_open_type) {
    FILE *file;
    char *file_full_name;
    if(strcmp(file_open_type, "r") != 0 && strcmp(file_open_type, "w") != 0) { /* the function can only read files or write on files */
        printf("Error: The file open type %s is illegal\n", file_open_type);
        return NULL;
    }
    file_full_name = get_file_full_name(file_name, new_extension);
    if(file_full_name == NULL)
        return NULL;
    file = fopen(file_full_name, file_open_type);
    if(file == NULL) {
        printf("Error: The file %s can't be opened\n", file_full_name);
        return NULL;
    }
    return file;
}


char *empty_binary_dot() {
    int i;
    char *str = (char*) malloc_and_check(sizeof(char) * (WORD_BIT + 1)); /* the size of a machine word is WORD_BIT plus '\0' character at the end */
    for(i = 0; i < WORD_BIT; i++)
        str[i] = '.';
    str[WORD_BIT] = '\0';
    return str;
}

void *malloc_and_check(size_t size) {
    void *ptr = malloc(size);
    if(ptr == NULL) { /* if malloc failed then exit the program */
        printf("Error: malloc out of space, could not allocate new memory\n");
        exit(0);
    }
    return ptr;
}

char *strdup(const char *src) {
    char *dst = malloc_and_check(strlen(src) + 1);  /* the size of the original string and 1 char for '\0' at the end */
    strcpy(dst, src);
    return dst;                            
}

int is_ascii(int c) {
    return ((c >= 0) && (c <= 127));
}