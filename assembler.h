#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "instruction.h"
#include "label.h"

#define MAX_DATA 256 /* the maximum number of data that a file can contain (maximum number for the DC counter) */
#define IC_START 100 /* the first value of the IC counter */

/**
 * An enum used for determining the current step to scan lines for a given file.
*/
typedef enum COMMAND_TYPE
{
    COMMAND_LABEL_NAME, /* argument can be a label declaration, label type or opcode*/
    COMMAND_LABEL_VALUE, /* arguments can be integers for LABEL_DATA or a string for LABEL_STRING */
    COMMAND_SOURCE_OPERAND, /* the opcodes that can use this: group 1 or parameter addressing opcodes */
    COMMAND_DEST_OPERAND, /* the opcodes that can use this are every legal opcodes except group 3*/
    COMMAND_JUMP_OPERAND, /* only the opcodes: jmp, bne and jsr use this as the destination operand */
    EXECUTE, /* the command can be executed */
    ERROR /* an error occurs */
} COMMAND_TYPE;

/**
 * A data structure used for computing the assembler for files. 
 * It contains informative flags, instructions, labels and the DC and IC counters: 
 * DC is used for calculating the number of data that the file contain (such as LABEL_DATA or LABEL_STRING labels data). 
 * IC is used for calculating the number of machine words that a file contain (the number of words of each instruction).
*/
typedef struct machine_t machine;

/**
 * Activates the pre assembler, first assembler scan, second assembler scan 
 * and the conversion from machine to code.
 * At the end of each step, if an error occurs then exit the function.
 * @param machine the current machine to use.
 * @param file_name a given file name without extension.
 * @returns false\0 if an error occurs, returns true\1 otherwise.
*/
int full_assembler(machine *machine, char *file_name);

/**
 * Creates a new machine and initaite its data by resetting the machine.
 * @returns the newly created machine.
*/
machine *initialize_machine();

/**
 * Resets the machine: 
 * Set IC to 100, set 0 to every integer in the data array, creates the label table and instruction list, and set everything else to 0.
 * @param machine a given machine.
*/
void reset_machine(machine *machine);

/**
 * Checks if a given command type or a given instruction is used for addressing parameters 
 * and builds the instruction according to the command type.
 * @param str a given argument
 * @param machine the mcahine to use on
 * @param command_type the current command type
 * @param instruct the current instruction
 * @param c the current char
 * @param comma_count the number of commas between two non white space characters
 * @param error_info the current error location info.
 * @returns true if the command type was changed, returns false otherwise
*/
int process_addressing_parameter(char *str, machine *machine, COMMAND_TYPE *command_type, instruction *instruct, char c, int comma_count, error_location *error_info);

/**
 * Checks if a given argumet is legal for the current command type, and if so then add it's value and addressing type to the instruction and point to the next command type, 
 * otherwise prints an error message and inform the command type that an error oocurs, and if added successfully then count the number of words the instruction will use
 * in the first scan the label table is not fully initiated therefore we only check the validity of the argment 
 * and in the second scan if the argument is not an integer or a register we check if the label name exists in the label table 
 * and if the label type is matching the command condition, otherwise we print the error message and inform the command type that an error oocurs. 
 * @param str a given argument
 * @param mchine the machine to use on
 * @param instruct the current instruction
 * @param command_type a pointer to the current command
 * @param error_info the current error location info.
*/
void process_operand(char str[], machine *machine, instruction *instruct, COMMAND_TYPE *command_type, error_location *error_info);

/**
 * Process the file to check if error exists, and if not then insert instructions and labels into the machine. 
 * In the first scan only the runtime errors can be found therefore when a label operand is being checked then check only if it is legal. 
 * The first scan also builds the label and instructions table, and the label operands of the instruction will be processed in the second scan if no errors are to be found in this function. 
 * If an error occrs then print it's message.
 * @param machine a given machine to use
 * @param input_path the file name without extension
 * @returns false if the file does not have errors, retrns true otherwise
*/
int assembler_first_scan(machine *machine, char input_path[]);

/**
 * Process the file to check if error exists, and if not then update the machine code to include labels as operands. 
 * Beacuse the first scan checks for runtime error and can't check for label operands bcause the label table had not been yet fully initiated, 
 * then the the file will be scan for a second time to check for label operands errors, and checks for entry declaration for labels and set them as entry.
 * @param machine a given machine to use
 * @param input_path the file name without extension
 * @returns false if the file does not have errors, retrns true otherwise
*/
int assembler_second_scan(machine *machine, char input_path[]);

/**
 * Converts the code into machine code, create object, entry and extern file only if the first and second scan were successfully completed without error.
 * @param machine the current machine
 * @param file_name a given file name without extension
 * @returns true if the creation was successfully completed, otherwise returns false
*/
int convert_to_machine_code(machine *machine, char *file_name);

/**
 * Checks for the first command type to process the instruction second word (if not group 3) according to the opcode type. 
 * group 3 have only one word therefore the next command type is EXECUTE
 * Returns the first word after the opcode command.
 * @param opcode_type a given opcode type
 * @returns the first command type to process the instruction second word (if not group 3) according to the opcode type, 
 * if the opcode type is UNKOWN then returns ERROR otherwise.
*/
COMMAND_TYPE get_opcode_first_command(OPCODE_TYPE opcode_type);

/**
 * Checks if a given string is a legal label name format, and if not then prints an error message.
 * @param str a given string
 * @param error_info the current error location info.
 * @returns true if str is a legal label name format, returns false otherwise
*/
int is_label_legal_format(char str[], error_location *error_info);

/**
 * Adds new data to the data array of a given machine. 
 * If the number of data in the machine (DC) is bigger\equal to MAX_DATA then an error occurs and exists the program.
 * @param machine a given machine.
 * @param value a given value.
 * @param error_info the current error location info.
*/
void add_new_data(machine *machine, int value, error_location *error_info);

#endif