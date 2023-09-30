#ifndef UTILS_H
#define UTILS_H

#define MAX_REGISTERS 8 /* the maximum register a machine contains, the names that they can be start from r0 and end with r7 */
#define MAX_COMMAND_LEN 80 /* the maximum command length is 80 */
#define WORD_BIT 14 /* the number of bits a word contains */
#define false 0
#define true 1

/**
 * An enum that contains every opcode type that is legal in the machine.
*/
typedef enum OPCODE_TYPE
{
    MOV,
    CMP,
    ADD,
    SUB,
    NOT,
    CLR,
    LEA,
    INC,
    DEC,
    JMP,
    BNE,
    RED,
    PRN,
    JSR,
    RTS,
    STOP,
    OPCODE_UNKOWN /* used when the opcode is not legal\unkown or initiation */
} OPCODE_TYPE;

/**
 * A data structure that contains inforamtion about the location that an error occurred on, such as: 
 * the file name, the line where the error occurred and the index on the line of the start of the error.
*/
typedef struct error_location_t error_location;

/**
 * Returns the opcode type to identify a given string.
 * @param str a given string.
 * @returns the opcode type to identify a given string, 
 * if the string is not a legal opcode type name then returns UNKOWN.
*/
OPCODE_TYPE get_opcode_type(char str[]);

/**
 * Returns the name of the opcode type.
 * @param opcode_type a given opcode type.
 * @returns the name of the opcode type, 
 * if the opcode type is UNKOWN or not a known opcode type then returns NULL.
*/
char *get_opcode_name(OPCODE_TYPE opcode_type);

/**
 * Checks if a given opcode type is used for parameter addresing.
 * Parameter addressing opcodes contain 1 operand and 2 parameters (the instruction that the opcode is part of).
 * The parameter addresing opcodes are: JMP, BNE and JSR.
 * @param opcode a given opcode type.
 * @returns tre\1 if the opcode type is used for parameter addresing, returns false\0 otherwise.
*/
int is_opcode_parameter_addresing(OPCODE_TYPE opcode);

/**
 * Checks if a given opcode type is part of the first group of opcodes.
 * Group one opcodes contain between 2-3 machine words (the instruction that the opcode is part of).
 * Those opcodes are: MOV, CMP, ADD, SUB and LEA.
 * @param opcode a given opcode type.
 * @returns tre\1 if the opcode type is part of the first group of opcodes, returns false\0 otherwise.
*/
int is_opcode_group_one(OPCODE_TYPE opcode);

/**
 * Checks if a given opcode type is part of the second group of opcodes.
 * Group two opcodes contain between 2-4 machine words (the instruction that the opcode is part of).
 * Those opcodes are: NOT, CLR, INC, DEC, JMP, BNE, RED, PRN and JSR.
 * @param opcode a given opcode type.
 * @returns tre\1 if the opcode type is part of the second group of opcodes, returns false\0 otherwise.
*/
int is_opcode_group_two(OPCODE_TYPE opcode);

/**
 * Checks if a given opcode type is part of the third group of opcodes.
 * Group two opcodes contain only 1 machine word (the instruction that the opcode is part of).
 * Those opcodes are: RTS and STOP.
 * @param opcode a given opcode type.
 * @returns tre\1 if the opcode type is part of the third group of opcodes, returns false\0 otherwise.
*/
int is_opcode_group_three(OPCODE_TYPE opcode);

/**
 * Creates an empty error loctaion and sets it's file name to a given string.
 * @param file_name a given file name.
 * @returns the newly created error location.
*/
error_location *create_empty_error_location(char *file_name);

/**
 * Ascend a given error location line value by 1.
 * @param error a given error location.
*/
void error_location_ascend_line(error_location *error);

/**
 * Sets the error location index value to a given integer.
 * @param error a given error location.
 * @param index a given index.
*/
void error_location_set_index(error_location *error, int index);

/**
 * Prints an error inforamtion (where it occurred) with a given error location and print the error message similar to printf: 
 * the function can have a diffresnt number of additional variables to add t othe string message after each '%' character.
 * @param error the current error location.
 * @param error_message a given error message.
 * @returns the total number of characters written excluding the error location, if an error occurs then a negative number is returned.
*/
int print_error_location(error_location *error, char *error_message, ...);

/**
 * Free the memory a given error location contains from the system memory.
 * @param error a given error location.
*/
void free_error_location(error_location *error);

/**
 * Checks if a given string is a legal register name and if so returns the register number in ascii.
 * @param str a given string
 * @returns the ascii number of the register if str is a legal register, returns false otherwise
*/
int is_legal_register(char str[]);


/**
 * Checks if a given character whitespace: spacebar, carriage return or tab.
 * @param c a given character integer value.
 * @returns 1 if true, returns 0 otherwise 
*/
int is_whitespace(char c);

/**
 * Checks if a given character is the end of a line: enter or null.
 * @param c a given character integer value.
 * @returns 1 if true, returns 0 otherwise 
*/
int is_end_null(char c);

/**
 * Checks if a given string is a valid integer.
 * @param str a given string
 * @returns true if str is a valid integer, returns false otherwise
*/
int is_integer(char str[]);

/**
 * Create the full name of a file string containing a given file name and a given file extension.
 * @param file_name a given file name without extensions.
 * @param new_extension a given file extension that starts with the character '.'.
 * @returns the newly created string.
*/
char *get_file_full_name(char *file_name, char *new_extension);

/**
 * Add a given extension to a given file name and open the file using a given file open type.
 * @param file_name a given file name
 * @param new_extension a given file extension
 * @param file_open_type a given file open type
 * @returns the file if opened successfully, returns NULL otherwise.
*/
FILE *open_file(char *file_name, char *new_extension, char *file_open_type);

/**
 * Create an empty binary string containing only dots.
 * @returns the created string, if the memory wasn't allocated then returns NULL
*/
char *empty_binary_dot();

/**
 * Returns a new void pointer created with malloc in a given size. 
 * If new memory could not be allocated then exit the program.
 * @param size the size to create the void pointer.
*/
void *malloc_and_check(size_t size);

/**
 * Creates a copy of a given string content. 
 * Copies the given string to the newly created string and return the string copy of the original string.
 * @param src a given string.
 * @returns the string copy of the original string.
*/
char *strdup(const char *src);

/**
 * Checks if a given character is a legal ascii character: 
 * only chars with an integer value between 0 (included) to 127 (included) are ascii characters
 * @param c a given character integer value.
 * @returns 1 if true, returns 0 otherwise 
*/
int is_ascii(int c);

#endif