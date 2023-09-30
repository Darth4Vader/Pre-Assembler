#include <stdio.h>
#include <stdlib.h>
#include "instruction.h"

/**
 * An enum used for identifying the encoding for every machine word.
*/
typedef enum ENCODING_TYPE
{
    ENCODING_A, /* used for encoding direct operand words */
    ENCODING_E, /* used for encoding external labels */
    ENCODING_R /* used for encoding relocatable words: local labels in a given file that are not extern */
} ENCODING_TYPE;

struct instruction_t
{
    OPCODE_TYPE opcode; /* the opcode type of the function */
    ADDRESSING_MODE input_a; /* the addressing type of the input parameter */
    ADDRESSING_MODE output_a; /* the addressing type of the output parameter */
    int label_p; /* this operand is used as output operand for addressing parameter opcode instructions */
    int input_p; /* this operand is used as the first parameter for addressing parameter opcode instructions, otherise as input operand */
    int output_p; /* this operand is used as the second parameter for addressing parameter opcode instructions, otherise as output operand */
    unsigned is_addressing_parameter:1; /* check if the instruction is addressing parameter: input_p and output_p will be used as 2 parameters */
    struct instruction_t *next; /* the next instruction */
};

struct instruction_list
{
    instruction *head; /* the first instruction in the list */
    instruction *root; /* the last instruction in the list */
};

instruction *create_instruction(OPCODE_TYPE opcode_type) {
    instruction *instrct = (instruction*) malloc_and_check(sizeof(instruction));
    instrct->opcode = opcode_type;
    instrct->label_p = 0;    
    instrct->input_a = ADRESSING_UNKOWN;
    instrct->input_p = 0;
    instrct->output_a = ADRESSING_UNKOWN;
    instrct->output_p = 0;
    instrct->is_addressing_parameter = false;
    instrct->next = NULL;
    return instrct;
}

instruction_list* create_empty_instruction_list() {
    instruction_list *list = (instruction_list*) malloc_and_check(sizeof(instruction_list));
    list->head = NULL;
    list->root = NULL;
    return list;
}

instruction *insert_new_instruction(instruction_list *list, OPCODE_TYPE opcode_type) {
    instruction *root;
    if(list == NULL) return NULL;
    root = create_instruction(opcode_type);
    if(list->head == NULL)
        list->head = root;
    else if(list->root != NULL)
        list->root->next = root;
    list->root = root;
    return root;
}

OPCODE_TYPE instruction_get_opcode_type(instruction *instruct) {
    return instruct != NULL ? instruct->opcode : OPCODE_UNKOWN;
}

ADDRESSING_MODE instruction_get_input_addressing(instruction *instruct) {
    return instruct != NULL ? instruct->input_a : ADRESSING_UNKOWN;
}

void instruction_set_input_addressing(instruction *instruct, ADDRESSING_MODE addressing_mode) {
    if(instruct != NULL)
        instruct->input_a = addressing_mode;
}

void instruction_set_output_addressing(instruction *instruct, ADDRESSING_MODE addressing_mode) {
    if(instruct != NULL)
        instruct->output_a = addressing_mode;
}

void instruction_set_addressing_parameter_operand(instruction *instruct, int value) {
    if(instruct != NULL)
        instruct->label_p = value;
}

void instruction_set_input_operand(instruction *instruct, int input_p) {
    if(instruct != NULL)
        instruct->input_p = input_p;
}

void instruction_set_output_operand(instruction *instruct, int output_p) {
    if(instruct != NULL)
        instruct->output_p = output_p;
}

int instruction_get_addressing_parameter_status(instruction *instruct) {
    return instruct != NULL ? instruct->is_addressing_parameter : false;
}

void instruction_set_addressing_parameter_status(instruction *instruct, int addressing_parameter_status) {
    if(instruct != NULL)
        instruct->is_addressing_parameter = addressing_parameter_status;
}

instruction *get_next_instruction(instruction *instruct) {
    return instruct != NULL ? instruct->next : NULL;
}

instruction *get_first_intruction(instruction_list *list) {
    return list != NULL ? list->head : NULL;
}

char *get_first_word(instruction *instrct) {
    OPCODE_TYPE opcode;
    ADDRESSING_MODE addressing_type = 0;
    char *str;
    if(instrct == NULL)
        return NULL;
    str = empty_binary_dot();
    opcode = instrct->opcode;
    if(is_opcode_parameter_addresing(opcode)) { /* if the opcode is parameter addressing opcodes then check if the label is addressing parameters or not */
        if(instrct->is_addressing_parameter) {
            add_binary(str, 0, 2, instrct->input_a);
            add_binary(str, 2, 2, instrct->output_a);
            addressing_type = ADDRESSING_PARAMATER;
        }
        else
            addressing_type = ADDRESSING_DIRECT;
        add_binary(str, ADDRESSING_DESTINATION_BIT, 2, addressing_type);
    }
    else {
        if(!is_opcode_group_three(opcode)) { /* group 3 opcodes don't have operands */
            if(!is_opcode_group_two(opcode)) { /* group 1 opcodes have 2 operands */
                add_binary(str, ADDRESSING_SOURCE_BIT, 2, instrct->input_a);
            }
            add_binary(str, ADDRESSING_DESTINATION_BIT, 2, instrct->output_a); /* group 2 opcodes have only output operand */
        }
    }
    add_binary(str, OPCODE_BIT, 4, opcode); /* add the opcode bits to the string */
    add_binary(str, ENCODING_START_BIT, 2, 0); /* add the encoding bits to the string, the firt machine word encoding bits type is 0 */
    str[WORD_BIT] = '\0';
    return str;
}

void get_other_words(FILE *file, instruction *instrct, int *IC) {
    int add_operand = true; /* check if there is another machine word */
    ENCODING_TYPE encoding_type = ENCODING_A;
    int i;
    char *str;
    if(is_opcode_group_three(instrct->opcode))
        return;
    str = empty_binary_dot();
    if(is_opcode_parameter_addresing(instrct->opcode)) { /* if the opcode is parameter addressing opcodes then check if the label is addressing parameters or not */
        add_binary(str, 0, INT_BIT, instrct->label_p);
        if(instrct->label_p == 0) /* the labels word index starts from 100 then it will be 0 when the label is extern */
            encoding_type = ENCODING_E;
        else
            encoding_type = ENCODING_R;
        add_binary(str, ENCODING_START_BIT, 2, encoding_type);
        str[WORD_BIT] = '\0';
        fprintf(file, "\n0%i %s", *IC, str);
        add_operand = instrct->is_addressing_parameter; /* if the label is not addressing parameter then the are no more machine words in the current instruction */
        (*IC)++; /* append IC by 1 */
    }
    if(add_operand != false && is_opcode_group_three(instrct->opcode) == false) { /* group 3 opcodes don't have more machine words */
        for(i = 0; i < WORD_BIT; i++) /* resets the unique binary string */
            str[i] = '.';
        encoding_type = ENCODING_A;
        if(is_opcode_group_one(instrct->opcode) || (is_opcode_parameter_addresing(instrct->opcode) && add_operand != false)) { /* group 1 opcodes and addressing parameter instructions have 2 operands\parameters */
            if(instrct->input_a == ADDRESSING_REGISTER && instrct->output_a == ADDRESSING_REGISTER) {
                add_binary(str, 0, 6, instrct->input_p);
                add_binary(str, 6, 6, instrct->output_p);
                add_operand = false; /* if the operands\parameters addressing type is both ADDRESSING_REGISTER then they will be used in the same machine word */
            }
            else {
                if(instrct->input_a == ADDRESSING_IMMEDIATE || instrct->input_a == ADDRESSING_DIRECT)
                    add_binary(str, 0, INT_BIT, instrct->input_p);
                else if(instrct->input_a == ADDRESSING_REGISTER)
                    add_binary(str, 0, 6, instrct->input_p);
                if(instrct->input_a == ADDRESSING_DIRECT) {
                    if(instrct->input_p == 0)
                        encoding_type = ENCODING_E;
                    else
                        encoding_type = ENCODING_R;
                }
            }
            add_binary(str, ENCODING_START_BIT, 2, encoding_type);
            str[WORD_BIT] = '\0';
            fprintf(file, "\n0%i %s", *IC, str);
            (*IC)++; /* append IC by 1 */
        }
        if(add_operand != false) { /* if the instruction have only output operand or group 1 opcodes and addressing parameter instructions output operand addressing type is not ADDRESSING_REGISTER */
            for(i = 0; i < WORD_BIT; i++) /* resets the unique binary string */
                str[i] = '.';
            encoding_type = ENCODING_A;
            if(instrct->output_a == ADDRESSING_IMMEDIATE || instrct->output_a == ADDRESSING_DIRECT)
                add_binary(str, 0, INT_BIT, instrct->output_p);
            else if(instrct->output_a == ADDRESSING_REGISTER)
                add_binary(str, 6, 6, instrct->output_p);
            if(instrct->output_a == ADDRESSING_DIRECT) {
                if(instrct->output_p == 0)
                    encoding_type = ENCODING_E;
                else
                    encoding_type = ENCODING_R;
            }
            add_binary(str, ENCODING_START_BIT, 2, encoding_type);
            str[WORD_BIT] = '\0';
            fprintf(file, "\n0%i %s", *IC, str);
            (*IC)++; /* append IC by 1 */
        }
    }
}

void add_binary(char* str, int index, int size, int num) {
    if(size > 0) {
        add_binary(str, index, size-1, num>>1); /* the next bit can be viewed with the shift operation >> by 1 */
        str[index+(size-1)] = (num&1) ? '/' : '.';
    }
}

void free_instruction(instruction *head) {
    instruction *root;
    if(head == NULL) return;
    root = head->next;
    free(head);
    free_instruction(root);   
}

void free_instruction_list(instruction_list *list) {
    if(list == NULL) return;
    free_instruction(list->head);
    free(list);
}