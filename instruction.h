#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include "utils.h"

#define OPCODE_BIT 4 /* the number of bits of a word until the opcode bits and the opcode bits length */
#define ADDRESSING_SOURCE_BIT 8 /* the start bit of the addressing source bits in the machine word */
#define ADDRESSING_DESTINATION_BIT 10 /* the start bit of the addressing destination bits in the machine word */
#define ENCODING_START_BIT 12 /* the number of bits of a word until the encoding bits */
#define INT_BIT 12 /* the number of bits that a machine word\machine data contain without the 2 encoding bits */

/**
 * An enum used for determining the addressing type of the operands.
*/
typedef enum
{
    ADDRESSING_IMMEDIATE, /* operands that are numbers use this addressing type */
    ADDRESSING_DIRECT, /* opernads that are labels with the label type LABEL_DATA or LABEL_STRING use this addressing type */
    ADDRESSING_PARAMATER, /* operand that are labels that use addressing parameter (the input and output operands will be used as 2 parameters that can't be addressing parameter) use this addressing type */
    ADDRESSING_REGISTER, /* operands that are registers use this addressing type */
    ADRESSING_UNKOWN /* operands that are unkown data type use this addressing type (such as initiating an instruction) */
} ADDRESSING_MODE;

/**
 * A data structure that is used for storing system intructions, such as function with parameters and variables 
 * and contains the next instruction the system needs to operate.
*/
typedef struct instruction_t instruction;

/**
 * A data structure that is used for storing linked instructions with a constant insertion time.
*/
typedef struct instruction_list instruction_list;

/**
 * Creates an empty instruction and set its opcode type to a given opcode type.
 * @param opcode_type the opcode type to set on the instruction
 * @returns a pointer to the instruction, returns NULL if run out of memory
*/
instruction *create_instruction(OPCODE_TYPE opcode_type);

/**
 * Creates an empty instruction list.
 * @returns the newly created instruction list.
*/
instruction_list* create_empty_instruction_list();

/**
 * Creates an empty instruction and sets its opcode type to a given opcode type.
 * After that inserts the newly created instruction to the end of a given instruction list and returns the new instrction.
 * @param list a given instruction list.
 * @param opcode_type a given opcode type.
 * @returns the newly created instruction, returns NULL if failed to create.
*/
instruction *insert_new_instruction(instruction_list *list, OPCODE_TYPE opcode_type);

/**
 * Returns the opcode type of a given instruction.
 * @param instruct a given instruction.
 * @returns the opcode type of the given instruction, 
 * if the instruction equals to NULL then returns OPCODE_UNKOWN.
*/
OPCODE_TYPE instruction_get_opcode_type(instruction *instruct);

/**
 * Returns the input addressing mode of a given instruction.
 * @param instruct a given instruction.
 * @returns the input addressing mode of the given instruction, 
 * if the instruction equals to NULL then returns ADRESSING_UNKOWN.
*/
ADDRESSING_MODE instruction_get_input_addressing(instruction *instruct);

/**
 * Sets the instruction input addressing mode to a given addressing mode.
 * @param instruct a given instruction.
 * @param addressing_mode a given addressing mode.
*/
void instruction_set_input_addressing(instruction *instruct, ADDRESSING_MODE addressing_mode);

/**
 * Sets the instruction output addressing mode to a given addressing mode.
 * @param instruct a given instruction.
 * @param addressing_mode a given addressing mode.
*/
void instruction_set_output_addressing(instruction *instruct, ADDRESSING_MODE addressing_mode);

/**
 * Sets the instruction addressing parameter operand to a given integer.
 * If the instruction opcode is an addressing parameter opcode and 
 * the instructions is not addressing parameter then the addressing parameter operand is used as the output operand.
 * @param instruct a given instruction.
 * @param value a given integer.
*/
void instruction_set_addressing_parameter_operand(instruction *instruct, int value);

/**
 * Sets the instruction input operand to a given integer.
 * If the instruction is addressing parameter then the input operand is used as the first parameter.
 * @param instruct a given instruction.
 * @param input_p a given integer.
*/
void instruction_set_input_operand(instruction *instruct, int input_p);

/**
 * Sets the instruction output operand to a given integer.
 * If the instruction is addressing parameter then the output operand is used as the second parameter.
 * @param instruct a given instruction.
 * @param output_p a given integer.
*/
void instruction_set_output_operand(instruction *instruct, int output_p);

/**
 * Returns the status of addressing parameter for a given instruction.
 * @param instruct a given instruction.
 * @returns the status of addressing parameter for a given instruction, 
 * if instruct equals to NULL then returns false\0.
*/
int instruction_get_addressing_parameter_status(instruction *instruct);

/**
 * Sets the instruction addressing parameter status to a given integer.
 * @param instruct a given instruction.
 * @param addressing_parameter_status a given integer.
*/
void instruction_set_addressing_parameter_status(instruction *instruct, int addressing_parameter_status);

/**
 * Returns the next instruction after a given instruction.
 * @param instruct a given instruction.
 * @returns the next instruction to use after a given instruction, if instruct is NULL then returns NULL.
*/
instruction *get_next_instruction(instruction *instruct);

/**
 * Returns the first instruction of an instruction list.
 * @param list a given instruction list.
 * @returns the first instruction of the instruction list.
*/
instruction *get_first_intruction(instruction_list *list);

/**
 * Returns the unique binary representation of the first word of a given instruction. 
 * The first machine word structre is the following (from left to right): 
 * 0-1 bits inform the first parameter type (only when addressing parameter): can be a number, register or label. 
 * 2-3 bits inform the second parameter type (only when addressing parameter): can be a number, register or label. 
 * 4-7 bits inform the opcode type of the instruction. 
 * 8-9 bits inform the input operand addressing type. 
 * 10-11 bits inform the output operand addressing type. 
 * 12-13 bits inform the endcoding type of the instruction.
 * @param instruct a given instruction.
 * @returns a string containing the binary representation of the first word of a given instruction. 
*/
char *get_first_word(instruction *instruct);

/**
 * Writes all the other words after the first word (maximum 3 words) to a given file from a given instruction. 
 * Also adds the number of words the instruction contains to a given number pointer. 
 * The 2-4 machine words structre is the following for each addressing type (from left to right): 
 * ADDRESSING_IMMEDIATE: 0-11 bits for the number that the operand is equals to. 
 * 
 * ADDRESSING_DIRECT: 0-11 bits fo the IC value that a label contains that the operand is equals to. 
 * 
 * ADDRESSING_PARAMATER: the label operand addressing can only be ADDRESSING_DIRECT and will use it to. 
 * the input and output operand will be used as 2 parameters with the addressing type: ADDRESSING_IMMEDIATE, ADDRESSING_DIRECT or ADDRESSING_REGISTER. 
 * 
 * ADDRESSING_REGISTER: 0-5 bits for the input operand register number, 6-11 bits for the output operand register number. 
 * if both input and output operands are ADDRESSING_REGISTER then add them to the same word. 
 * 
 * The last 12-13 bits of each machine word is used for the encoding type of the machine word.
 * @param file a given file to write on the words.
 * @param instruct a given instruction.
 * @param IC a given pointer to an integer.
 *  
*/
void get_other_words(FILE *file, instruction *instruct, int *IC);

/**
 * Converts a given integer to a binary representation in a given size 
 * and adds it to a given string at a given index.
 * This function creates a unique binary representation, as such '0' becomes '.' and '1' becomes '/'.
 * @param str a given string.
 * @param index the start index to add the binary representation of str
 * @param size the size to create the binary representation of str, also known as offset index.
 * @param num a given number.
*/
void add_binary(char* str, int index, int size, int num);

/**
 * Free the memory a given instruction contains from the system memory.
 * @param head a given macro table.
*/
void free_instruction(instruction *head);

/**
 * Free the memory a given instruction list contains from the system memory.
 * @param list a given instruction list.
*/
void free_instruction_list(instruction_list *list);

#endif