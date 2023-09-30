#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "assembler.h"
#include "pre_assembler.h"
#include "utils.h"

struct machine_t
{
    int data_array[MAX_DATA]; /* the data array of the machine to store the labels data values */
    int DC:8; /* the number of data values inside the machine data array (can be maximum 256) */
    int IC; /* the number of machine words inside the instructions (starts at base 100)*/
    unsigned has_mcr:1; /* a flag that informs if an instruction had been declared for the pre assembler */
    unsigned has_label:1; /* a flag that informs if a label had been declared in the start of the command */
    unsigned initiated_labels:1; /* after the first scan set to true */
    unsigned write_entry_file:1; /* tells the machine if to create an entry file */
    unsigned write_extern_file:1; /* tells the machine if to create an extern file */
    label_table *labels_table; /* the label table of the current machine */
    instruction_list *instructions_list; /* the instruction list of the current machine */
};

int full_assembler(machine *machine, char *file_name) {
    if(pre_assembler(file_name) != false)
        return false;
    if(assembler_first_scan(machine, file_name) != false)
        return false;
    if(assembler_second_scan(machine, file_name) != false)
        return false;
    return convert_to_machine_code(machine, file_name);
}

machine *initialize_machine() {
    machine *main_machine = (machine*) malloc_and_check(sizeof(machine));
    main_machine->labels_table = NULL;
    main_machine->instructions_list = NULL;
    reset_machine(main_machine);
    return main_machine;
}

void reset_machine(machine *machine) {
    int i;
    if(machine == NULL)
        return;
    machine->DC = 0;
    machine->IC = IC_START;
    machine->has_label = false;
    machine->has_mcr = false;
    machine->initiated_labels = false;
    machine->write_entry_file = false;
    machine->write_extern_file = false;
    free_label_table(machine->labels_table);
    machine->labels_table = create_empty_label_table();
    free_instruction_list(machine->instructions_list);
    machine->instructions_list = create_empty_instruction_list();
    for(i = 0; i < MAX_DATA; i++)
        machine->data_array[i] = 0;
}

int process_addressing_parameter(char *str, machine *machine, COMMAND_TYPE *command_type, instruction *instruct, char c, int comma_count, error_location *error_info) {
    if(*command_type == COMMAND_JUMP_OPERAND || *command_type == COMMAND_SOURCE_OPERAND || *command_type == COMMAND_DEST_OPERAND) { /* addressing parameters uses only the 3 following command types */
        if(*command_type == COMMAND_JUMP_OPERAND && c == '(') { /* first we chack if the argument is addressing parameters */
            process_operand(str, machine, instruct, command_type, error_info); /* process the destination operand */
            if(*command_type != ERROR) {/* if the destination operand is legal */
                instruction_set_addressing_parameter_status(instruct, true); /* add attribute of addressing parameter to the instruction */
                *command_type = COMMAND_SOURCE_OPERAND; /* becase addressing parameter is declared then the next command type is to find the first parameter */
                return true;
            }
        }
        else if(instruct != NULL && instruction_get_addressing_parameter_status(instruct) && c == ')') { /* if addressing parameter and the char is ')' then it means that the second parameter can be processed */
            if(*command_type == COMMAND_SOURCE_OPERAND || comma_count > 0) { /* if the first parameter was found or the second parameter had not been found then error occurs */
                print_error_location(error_info, "missing the second parameter before the token ')'");
                *command_type = ERROR;
            }
            else
                process_operand(str, machine, instruct, command_type, error_info);
            return true;
        }
    }
    return false;
}

void process_operand(char str[], machine *machine, instruction *instruct, COMMAND_TYPE *command_type, error_location *error_info) {
    OPCODE_TYPE opcode_type = instruction_get_opcode_type(instruct);
    ADDRESSING_MODE addressing_type = ADRESSING_UNKOWN; /* used for checking the dressing type of str */
    int num;
    label *lbl = NULL; /* the label to contain the name str, if str is legal and exists */
    LABEL_TYPE label_type; /* the label type of lbl (if lbl does not equals to NULL) */
    if(str[0] == '#') { /* if the first character is '#' then the following string must be an integer*/
        if(is_integer(&str[1])) { /* if the followaing string is not an integer then an error occurs */
            addressing_type = ADDRESSING_IMMEDIATE;
            if(*command_type == COMMAND_SOURCE_OPERAND) {
                if(opcode_type == LEA) { /* the addressing type of the source operand of the opcode lea can't be an immediate addressing */
                    print_error_location(error_info, "integers are not allowed to be used as a source operand in the opcode lea");
                    *command_type = ERROR;
                }
                else {
                    instruction_set_input_addressing(instruct, ADDRESSING_IMMEDIATE);
                    instruction_set_input_operand(instruct, atoi(&str[1])); /* we verified that the string from the second character is an integer */
                    *command_type = COMMAND_DEST_OPERAND;
                }
            }
            else if(*command_type == COMMAND_JUMP_OPERAND) { /* the destination parameter of the opcodes: jmp, bne and jsr can't be an integer */
                print_error_location(error_info, "integers are not allowed to be used as a destination operand in the opcode %s", get_opcode_name(opcode_type));
                *command_type = ERROR;
            }
            else {
                if(opcode_type == CMP || opcode_type == PRN || instruction_get_addressing_parameter_status(instruct)) { /* the destination operand an be an integer only for the opcodes cmp and prn or the second parameter of parameter addressing opcodes */
                    if(instruction_get_addressing_parameter_status(instruct) && instruction_get_input_addressing(instruct) == ADDRESSING_REGISTER) /* if the second parameter is register then we can count it as the third word and the current integer is the fourth word */
                        machine->IC++;
                    instruction_set_output_addressing(instruct, ADDRESSING_IMMEDIATE);
                    instruction_set_output_operand(instruct, atoi(&str[1])); /* we verified that the string from the second character is an integer */
                    *command_type = EXECUTE;
                }
                else {
                    print_error_location(error_info, "integers are not allowed to be used as a destination operand in the opcode %s", get_opcode_name(opcode_type));
                    *command_type = ERROR;                                            
                }
            }
        }
        else {
            print_error_location(error_info, "the argument %s is not an integer", &str[1]);
            *command_type = ERROR;
        }
    }
    else {
        num = is_legal_register(str);
        if(num != false) { /* we check if str is a legal register name */
            addressing_type = ADDRESSING_REGISTER;
            num = num - '0'; /* to get the register number not in ascii */
            if(*command_type == COMMAND_SOURCE_OPERAND) {
                if(opcode_type == LEA) { /* the addressing type of the source operand of the opcode lea can't be a register addressing */
                    print_error_location(error_info, "registers are not allowed to be used as a source operand in the opcode lea");
                    *command_type = ERROR;
                }
                else {
                    instruction_set_input_addressing(instruct, ADDRESSING_REGISTER);
                    instruction_set_input_operand(instruct, num);
                    *command_type = COMMAND_DEST_OPERAND;
                }
            }
            else if(*command_type == COMMAND_JUMP_OPERAND) { /* the destination parameter of the opcodes: jmp, bne and jsr can't be a register */
                print_error_location(error_info, "registers are not allowed to be used as a destination operand in the opcode %s", get_opcode_name(opcode_type));
                *command_type = ERROR;
            }
            else {
                instruction_set_output_addressing(instruct, ADDRESSING_REGISTER);
                instruction_set_output_operand(instruct, num);
                *command_type = EXECUTE;
                machine->IC++; /* if the first parameter or the first operand is a register the current register will be added to the same word and then we can count them as one word, if the instruction has only a destination operand it will be the same */
            }
        }
        else {
            if(is_label_legal_format(str, error_info)) { /* if str is not a legal label format then an error occurs */
                addressing_type = ADDRESSING_DIRECT;
                /* we use machine->initiated_labels because in the first scan the label table had not been fully initiated therefore we only check the validity of str instaed of the existence of str in the label table */
                if(machine->initiated_labels != false)
                    lbl = search_in_label_table(machine->labels_table, str);
                if(lbl != NULL || machine->initiated_labels == false) {
                    label_type = label_get_type(lbl);
                    if(*command_type == COMMAND_SOURCE_OPERAND) {
                        instruction_set_input_addressing(instruct, ADDRESSING_DIRECT);
                        *command_type = COMMAND_DEST_OPERAND;
                        if(lbl != NULL) { /* we check this only in the second scan and not in the first scan */
                            if(label_type == LABEL_CODE) { /* labels that are pointing to code can't be used as a source operand */
                                print_error_location(error_info, "opcode labels are not allowed to be used as a source operand in the opcode %s", get_opcode_name(opcode_type));
                                *command_type = ERROR;
                            }
                            else {
                                if(label_type == LABEL_EXTERN) { /*if the label type is extern then add the current IC to the label extern list */
                                    label_add_extern_word_index(lbl, machine->IC);
                                    machine->write_extern_file = true;
                                }
                                instruction_set_input_operand(instruct, label_get_IC(lbl));
                            }
                        }
                    }
                    else if(*command_type == COMMAND_JUMP_OPERAND) {
                        *command_type = EXECUTE; /* we check in function "process_addressing_parameter" if the char '(' is detected and then we address parameters */
                        if(lbl != NULL) { /* we check this only in the second scan and not in the first scan */
                            if(label_type != LABEL_CODE && label_type != LABEL_EXTERN) { /* only labels that are pointing to code/extern are allowed as the destination operand for the opcodes: jmp, bne and jsr*/
                                print_error_location(error_info, "direct addressing is not allowed to be used as a destination operand in the opcode %s", get_opcode_name(opcode_type));
                                *command_type = ERROR;
                            }
                            else {                              
                                if(label_type == LABEL_EXTERN) { /*if the label type is extern then add the current IC to the label extern list */
                                    label_add_extern_word_index(lbl, machine->IC);
                                    machine->write_extern_file = true;
                                }
                                instruction_set_addressing_parameter_operand(instruct, label_get_IC(lbl));
                            }
                        }
                    }
                    else {
                        instruction_set_output_addressing(instruct, ADDRESSING_DIRECT);
                        *command_type = EXECUTE;
                        if(instruction_get_input_addressing(instruct) == ADDRESSING_REGISTER) /* if the second or third word is a register then the current label will be added into a new word */
                            machine->IC++;
                        if(lbl != NULL) { /* we check this only in the second scan and not in the first scan */
                            if(label_type == LABEL_CODE) { /* labels that are pointing to code can't be used as a destination operand or as a second parameter */
                                print_error_location(error_info, "opcode labels are not allowed to be used as a destination operand in the opcode %s", get_opcode_name(opcode_type));
                                *command_type = ERROR;
                            }
                            else {
                                if(label_type == LABEL_EXTERN) { /*if the label type is extern then add the current IC to the label extern list */
                                    label_add_extern_word_index(lbl, machine->IC);          
                                    machine->write_extern_file = true;
                                }
                                instruction_set_output_operand(instruct, label_get_IC(lbl));
                            }
                        }
                    }                                        
                }
                else { /* this is only eligible in the second scan */
                    print_error_location(error_info, "the label \"%s\" does not exist", str);
                    *command_type = ERROR;                                
                }
            }
            else
                *command_type = ERROR;
        }
    }
    if(*command_type != ERROR && addressing_type != ADDRESSING_REGISTER) /* if the current addressing type is not addressing register then current word will be added into the machine word count, becuase we checked the registers above */
        machine->IC++;
}

int assembler_first_scan(machine *machine, char input_path[]) {
    char command[MAX_COMMAND_LEN];
    char c;
    int error = false;
    COMMAND_TYPE command_type;
    OPCODE_TYPE opcode_type;
    LABEL_TYPE label_type;
    char str[MAX_COMMAND_LEN];
    int length = 0;
    label *lbl; /* the current label */
    instruction *instruct; /* the current instruction */
    int i, last_c = 0;
    int whitespace_count = 0, comma_count = 0; /* the commas and whitespace count between 2 non null, whitespace and comma character*/
    int entered_string = false; /* when entering s a string that is used for when the command type is COMMAND_LABEL_VALUE and the label type is LABEL_STRING */
    error_location *error_info; /* the error location to print when an error occurs */
    FILE *file = open_file(input_path, ".am", "r"); /* read file */
    if(file == NULL)
        return true;
    error_info = create_empty_error_location(get_file_full_name(input_path, ".am"));
    machine->DC = 0; /*reset the data and words counters */
    machine->IC = IC_START;
    while((fgets(command, MAX_COMMAND_LEN, file)) != NULL) { /* read the file line by line */
        error_location_ascend_line(error_info);
        error_location_set_index(error_info, 0);
        command_type = COMMAND_LABEL_NAME;
        opcode_type = OPCODE_UNKOWN;
        label_type = LABEL_UNKOWN;
        length = 0;
        str[0] = '\0';
        lbl = NULL;
        instruct = NULL;
        entered_string = false;
        comma_count = 0;
        whitespace_count = 0;
        str[0] = '\0';
        length = 0;
        machine->has_label = false;
        if(command[0] != ';') { /* comment lines are ignored */
            for(i = 0; i < MAX_COMMAND_LEN; i++) {
                error_location_set_index(error_info, i);
                c = command[i];
                if(command_type == EXECUTE) { /* if the command has no errors an can be exected check if there are more arguments than allowed errors */
                    if(is_end_null(c) == false && is_whitespace(c) == false) { /* if a non whitespace and non end nll character is arrived then an error occurs */
                        if(c == ',') { /* commas are illegal at the end of a command */
                            print_error_location(error_info, "Illegal comma");
                        }
                        else if(opcode_type != OPCODE_UNKOWN) { /* if the command is an instruction then print the error message based on the opcode maximum arguments allowed */
                            print_error_location(error_info, "The opcode \"%s\" ", get_opcode_name(opcode_type));
                            if(is_opcode_group_three(opcode_type)) {
                                print_error_location(error_info, "can't receive operands");
                            }
                            else if(is_opcode_group_two(opcode_type)) {
                                print_error_location(error_info, "can't receive an input operand");
                            }
                            else
                                print_error_location(error_info, "can't receive more than 2 operands");
                        }
                        else /* if the command is not an instruction then additional characters not (whitespace,end null) are not allowed */
                            print_error_location(error_info, "Extraneous text after end of command");
                        command_type = ERROR;
                    }                
                }
                else if(command_type == COMMAND_LABEL_VALUE && label_type == LABEL_STRING) { /* if a label type LABEL_STRING was declared then search for the string */
                    if(entered_string == true) { /* if the char " had been found */
                        if(c == '\"') { /* if the char is " then insert to the data array '\0' and set command type to be EXECUTE */
                            add_new_data(machine, '\0', error_info);
                            label_ascend_DC(lbl);
                            command_type = EXECUTE;
                        }
                        else {
                            if(is_end_null(c) == false) { /* if the char is not an end null then add it to the data array */
                                add_new_data(machine, c, error_info);
                                last_c = i;
                                label_ascend_DC(lbl);
                            }
                            else { /* the string was not closed therefore an error occurs */
                                error_location_set_index(error_info, last_c);
                                print_error_location(error_info, "missing \" declaration after '%c' token", command[last_c]);
                                command_type = ERROR;
                            }
                        }
                    }
                    else if(is_whitespace(c) == false) { /* if char is not whitespace then check if string is initiated with " */
                        if(c == '\"')
                            entered_string = true;
                        else if(is_end_null(c) == false) { /* if char is not the end of the file then missing a " declaration therefore an error occurs */
                            print_error_location(error_info, "missing \" declaration before '%c' token", c);
                            command_type = ERROR;
                        }
                    }
                }
                else {
                    if(is_whitespace(c) == false && is_end_null(c) == false && c != ',') {
                        if(process_addressing_parameter(str, machine, &command_type, instruct, c, comma_count, error_info) != false) { /* we check if the command type changed, and if so then reset str */
                            str[0] = '\0';
                            length = 0;
                        }
                        else {
                            str[length++] = c;
                            str[length] = '\0';
                        }              
                    }
                    else if(length > 0) { /* empty arguments are ignored */
                        if(command_type == COMMAND_LABEL_NAME) {
                            if((str[length-1] == ':' && machine->has_label == false) || label_type == LABEL_EXTERN || label_type == LABEL_ENTRY) { /* we check if the argument is a label declaration or a label argument, if the flag has_label is true then a label was declared */
                                if(label_type != LABEL_EXTERN && label_type != LABEL_ENTRY) /* check if a label is declared with ':' */
                                    str[length-1] = '\0';
                                if(is_label_legal_format(str, error_info)) { /* if str is not a legal format then an error occurs */
                                    if(label_type != LABEL_ENTRY) {
                                        lbl = search_in_label_table(machine->labels_table, str);
                                        if(lbl == NULL) { /* if the label exists in the label table then an error occurs */
                                            lbl = add_to_label_table(machine->labels_table, strdup(str));
                                            if(label_type == LABEL_EXTERN) { /* LABEL_EXETRN next command type is EXECUTE */
                                                label_set_type(lbl, LABEL_EXTERN);
                                                command_type = EXECUTE;
                                            }
                                            machine->has_label = true; /* we set the flag has_label to true */
                                        }
                                        else {
                                            print_error_location(error_info, "the label name \"%s\" had already been declared", str);
                                            command_type = ERROR;                                   
                                        }
                                    }
                                    else /* label types LABEL_ENTRY will be checked in the second scan if no error were found at the end of the file scan, but we check for EXECUTE errors */
                                        command_type = EXECUTE;
                                }
                                else
                                    command_type = ERROR;
                            }
                            else {
                                label_type = get_label_type(str);
                                if((machine->has_label == false && (label_type == LABEL_EXTERN || label_type == LABEL_ENTRY)) == false) { /* we check the 2 following label types above and */
                                    if(label_type != LABEL_UNKOWN) { /* if the argment is a valid label type */
                                        if(label_type == LABEL_DATA || label_type == LABEL_STRING) {
                                            if(machine->has_label) { /* a label name declaration must be before the usage of the 2 following label types */
                                                label_set_type(lbl, label_type);
                                                label_set_IC(lbl, machine->DC);
                                                command_type = COMMAND_LABEL_VALUE;                                                
                                            }
                                            else {
                                                print_error_location(error_info, "the label type \"%s\" requires a label name declaration before it's use", str);
                                                command_type = ERROR;                                                
                                            }
                                        }
                                        else if(label_type == LABEL_EXTERN || label_type == LABEL_ENTRY) { /* the 2 following label types cannot be used as a label data type declaration therefore an error occurs */
                                            print_error_location(error_info, "the argument \"%s\" can't be used as a label data type", str);
                                            command_type = ERROR;
                                        }
                                    }
                                    else { /* the argument must be an opcode */
                                        opcode_type = get_opcode_type(str);
                                        if(opcode_type != OPCODE_UNKOWN) { /* if opcode is valid */
                                            command_type = get_opcode_first_command(opcode_type);
                                            instruct = insert_new_instruction(machine->instructions_list, opcode_type);
                                            if(machine->has_label && lbl != NULL) { /* if a label was declared then set it's type to LABEL_CODE */
                                                label_set_type(lbl, LABEL_CODE);
                                                label_set_IC(lbl, machine->IC);
                                            }
                                            machine->IC++; /* the opcode is the first word of an instruction */
                                        }
                                        else {
                                            print_error_location(error_info, "the argument \"%s\" is not legal", str);
                                            command_type = ERROR;
                                        }
                                    }
                                }
                            }
                        }
                        else if(command_type == COMMAND_LABEL_VALUE) {
                            if(label_type == LABEL_DATA) { 
                                if(is_integer(str)) { /* if the label type is LABEL_DATA then check if argument is an integer otherwise an error occurs */
                                    add_new_data(machine, atoi(str), error_info); /* add the integer to the machine data array */
                                    label_ascend_DC(lbl);
                                }
                                else {
                                    print_error_location(error_info, "The parameter \"%s\" is not an integer", str);
                                    command_type = ERROR;
                                }
                            }
                        }
                        else if(command_type == COMMAND_SOURCE_OPERAND || command_type == COMMAND_DEST_OPERAND || command_type == COMMAND_JUMP_OPERAND) {
                            if(command_type == COMMAND_DEST_OPERAND && (instruct != NULL && instruction_get_addressing_parameter_status(instruct))) { /* if parameter addressing and the second parameter arrives here (because it's supposed to be processed above in process_addressing_parameter) then an error occurs */
                                print_error_location(error_info, "missing a ')' token after the string \"%s\"", str);
                                command_type = ERROR;
                            }
                            else
                                process_operand(str, machine, instruct, &command_type, error_info);
                        }
                        length = 0; /* resets the string */
                        str[0] = '\0';
                    }
                    if(c == ',') /* add to commas and whitespace count */
                        comma_count++;
                    else if(is_whitespace(c))
                        whitespace_count++;
                    if(command_type != ERROR) { /* checks for comma exceptions */
                        if(command_type == EXECUTE) {
                            if(opcode_type != OPCODE_UNKOWN) {
                                if(is_end_null(c) == false && is_whitespace(c) == false && comma_count > 0) {
                                    print_error_location(error_info, "Illegal comma after end of command");
                                    command_type = ERROR; 
                                }
                            }
                        }
                        else if(is_opcode_parameter_addresing(opcode_type) && instruction_get_addressing_parameter_status(instruct) && whitespace_count > 0) {
                                print_error_location(error_info, "white tokens are illegal inside parameter addressing brackets '%c'", c);
                                command_type = ERROR;
                        }
                        else if((command_type == COMMAND_DEST_OPERAND && (is_opcode_group_one(opcode_type) || (is_opcode_parameter_addresing(opcode_type) && instruction_get_addressing_parameter_status(instruct))))
                                || (command_type == COMMAND_LABEL_VALUE && label_type == LABEL_DATA && label_get_DC(lbl) > 0 )) {
                            if(comma_count > 1) {
                                print_error_location(error_info, "Multiple consecutive comma");
                                command_type = ERROR;                                
                            }
                            else if(c != ',' && is_whitespace(c) == false && is_end_null(c) == false)
                                if(comma_count == 0 && whitespace_count > 0) {
                                    print_error_location(error_info, "Missing comma before '%c' token", c);
                                    command_type = ERROR;                            
                                }                  
                        }
                        else if(comma_count > 0) {
                            print_error_location(error_info, "Illegal comma '%c'", c);
                            command_type = ERROR;                            
                        }
                    }
                    if(c != ',' && is_whitespace(c) == false && is_end_null(c) == false) { /* if the character is not a comma, whitespace and end nul then reset the counts */
                        comma_count = 0; 
                        whitespace_count = 0;
                    }
                }
                if(command_type == ERROR || is_end_null(c)) {
                    if(command_type != ERROR && command_type != EXECUTE) { /* if the command type is not ERROR or EXECUTE then a missing operand error occurs */
                        if(command_type == COMMAND_LABEL_NAME) {
                            if(label_type == LABEL_EXTERN || label_type == LABEL_ENTRY) {
                                print_error_location(error_info, "missing a label name after the \"%s\" statement", get_label_type_name(label_type));
                                command_type = ERROR;
                            }
                        }
                        if(command_type == COMMAND_LABEL_VALUE) {
                            if(label_type == LABEL_STRING) {
                                print_error_location(error_info, "missing a string declaration after the \".string\" statement");
                                command_type = ERROR;
                            }
                            else if(label_type == LABEL_DATA && label_get_DC(lbl) == 0) {
                                print_error_location(error_info, "missing an integer declaration after the \".data\" statement");
                                command_type = ERROR;
                            }
                        }
                        else if(instruct != NULL) {
                            print_error_location(error_info, NULL);
                            if(is_opcode_parameter_addresing(opcode_type) && instruction_get_addressing_parameter_status(instruct)) {
                                if(command_type == COMMAND_SOURCE_OPERAND)
                                    printf("missing the first parameter");
                                else if(command_type == COMMAND_DEST_OPERAND)
                                    printf("missing the second parameter");
                            }
                            else {
                                printf("missing the ");
                                if(command_type == COMMAND_SOURCE_OPERAND) {
                                    printf("source operand");
                                    if(is_opcode_group_one(opcode_type))
                                        printf("and the ");
                                }
                                printf("destination operand"); 
                            }
                            printf("\n");
                            command_type = ERROR;
                        }
                    }
                    break; /* stop reading the current line */
                }
            }
            if(command_type == ERROR) /* if the command type is ERROR then the function will return true */
                error = true;
        }
    }
    fclose(file); /* closes the file to insure safety */
    free_error_location(error_info);
    if(error != false) /* if the file have error then exit the function */
        return true;
    lbl = get_first_label(machine->labels_table);
    while(lbl != NULL) { /* add to the data type labels the value of IC */
        if(label_get_type(lbl) == LABEL_DATA || label_get_type(lbl) == LABEL_STRING)
            label_set_IC(lbl, label_get_IC(lbl) + machine->IC);
        lbl = get_next_label(lbl);
    }
    machine->initiated_labels = true; /* sets the first scan to be finished successfully and the label table had been built with no errors */
    return false;
}

int assembler_second_scan(machine *machine, char input_path[]) {
    char command[MAX_COMMAND_LEN];
    char c;
    int error = false;
    label *lbl = get_first_label(machine->labels_table); /* the current\next label in the file */
    label *lbl_search; /* used to search for labels for entry operations */
    instruction *instruct = get_first_intruction(machine->instructions_list); /* the crrent\next instruction in the file */
    COMMAND_TYPE command_type; /* the current command type */
    OPCODE_TYPE opcode_type;
    LABEL_TYPE label_type;
    char str[MAX_COMMAND_LEN]; /* the current argument */
    int length = 0; /* the current argument length */
    int i;
    error_location *error_info; /* the error location to print when an error occurs */
    FILE *file = open_file(input_path, ".am", "r"); /* read file */
    if(file == NULL)
        return true;
    error_info = create_empty_error_location(get_file_full_name(input_path, ".am"));
    machine->IC = IC_START; /* reset the words counter */
    while((fgets(command, MAX_COMMAND_LEN, file)) != NULL) { /* read the file line by line */
        error_location_ascend_line(error_info);
        error_location_set_index(error_info, 0);
        command_type = COMMAND_LABEL_NAME;
        opcode_type = OPCODE_UNKOWN;
        label_type = LABEL_UNKOWN;
        str[0] = '\0';
        length = 0;
        if(command[0] != ';') { /* comment lines are ignored */
            for(i = 0; i < MAX_COMMAND_LEN; i++) {
                error_location_set_index(error_info, i);
                c = command[i];
                /* we checked in the first scan that every line doesn't have runtime errors therefore we don't need to check again: at EXECUTE or at null characters */
                if(command_type != EXECUTE) {
                    if(is_whitespace(c) == false && is_end_null(c) == false && c != ',') {
                        if(process_addressing_parameter(str, machine, &command_type, instruct, c, 0, error_info)) { /* we check if the command type changed, and if so then reset str */
                            str[0] = '\0';
                            length = 0;
                        }
                        else {
                            str[length++] = c;
                            str[length] = '\0';
                        }
                    }
                    else if(length > 0) { /* empty arguments are ignored */
                        if(command_type == COMMAND_LABEL_NAME) {
                            if(label_type == LABEL_ENTRY) { /* if the first argument of the command is an entry declaration */
                                lbl_search = search_in_label_table(machine->labels_table, str);
                                if(lbl_search != NULL) { /* we know no errors were found in the first scan therefore we only nedd to check if the label name exists in the label table */
                                    if(label_get_type(lbl_search) == LABEL_EXTERN) { /* if the label type was declared LABEL_EXTERN then an error occurs */
                                        print_error_location(error_info, "label can't be extern and entry at the same time");
                                        command_type = ERROR;
                                    }
                                    else { /* we set the label as entry for the entry file (if no errors were found during the second scan) */
                                        label_set_entry_status(lbl_search, true);
                                        machine->write_entry_file = true;
                                        command_type = EXECUTE;
                                    }
                                }
                                else {
                                    print_error_location(error_info, "The label \"%s\" doesn't exist", str);
                                    command_type = ERROR;
                                }
                            }
                            else if(str[length-1] == ':') { /* in the first scan we inserted every label into the label table and check the validity of the command */
                                if(lbl != NULL) {
                                    if(label_get_type(lbl) != LABEL_CODE) { /* if the label type is not LABEL_CODE we can skip this line becuase we checked the validity of the command in the first scan*/
                                        lbl = get_next_label(lbl);
                                        break;
                                    }
                                    lbl = get_next_label(lbl);
                                }
                                else
                                    break;
                            } 
                            else { /* the argument must be a label data type declaration or an opcode */
                                label_type = get_label_type(str);
                                if(label_type != LABEL_UNKOWN) { /* if str is a label type */
                                    if(label_type != LABEL_ENTRY) { /*if the label type is not entry then we checked the validity of them in the first scan, therefore we can skip them */
                                        lbl = get_next_label(lbl);
                                        break;
                                    }
                                }
                                else { /* if str is an opcode */
                                    opcode_type = instruction_get_opcode_type(instruct);
                                    command_type = get_opcode_first_command(opcode_type);
                                    machine->IC++; /* the opcode is the first word of every instruction */
                                }
                            }
                        }
                        else if(command_type == COMMAND_SOURCE_OPERAND || command_type == COMMAND_DEST_OPERAND || command_type == COMMAND_JUMP_OPERAND) /* process the instruction operands */
                            process_operand(str, machine, instruct, &command_type, error_info);
                        length = 0;
                        str[0] = '\0';
                    }
                }
                if(command_type == ERROR || is_end_null(c)) /* if an error occurs or a null character had been reached then exit the line */
                    break;
            }
            if(command_type == ERROR) /* if the line had an error then the function will return true */
                error = true;
            if(opcode_type != OPCODE_UNKOWN && instruct != NULL) /* if the current line has an opcode then move to the next instruction */
                instruct = get_next_instruction(instruct);
        }
    }
    fclose(file); /* closes the file to insure safety */
    free_error_location(error_info);
    return error;
}

int convert_to_machine_code(machine *machine, char *file_name) {
    instruction *instruct = get_first_intruction(machine->instructions_list);
    label *lbl; /* the current label */
    char *str; /* the binary representation of the data */
    int i, j, line_num;
    int IC = IC_START; /* to update the IC number in the object file */
    FILE *file_object, *file_entry, *file_extern;
    file_object = open_file(file_name, ".ob", "w"); /* write the object file */
    if(file_object == NULL)
        return false;
    fprintf(file_object , "%i %i", (machine->IC - IC_START), machine->DC); /* the first line includes the words and data count */
    while(instruct != NULL) { /* add to file all the instructions words */
        fprintf(file_object , "\n0%i %s", IC, get_first_word(instruct)); /* adds the number of the crrent word and the first word of the instruction */
        IC++; /* the first word was added to file so increase IC by 1 */
        get_other_words(file_object, instruct, &IC); /* add the other words (second, third and fourth) if they exist */
        instruct = get_next_instruction(instruct);
    }
    str = empty_binary_dot();
    if(str == NULL) /* checks if memory was allocated */
        return false;
    for(i = 0; i < machine->DC; i++) { /* add to file all the data words */
        for(j = 0; j < WORD_BIT; j++) /* reset binary dot for a safer approach */
            str[j] = '.';
        str[WORD_BIT] = '\0';
        add_binary(str, 0, WORD_BIT, machine->data_array[i]); /* get the binary dot of the data word */
        fprintf(file_object,"\n0%i %s", IC, str); /* adds the word to the file */
        IC++;
    }
    fclose(file_object);
    line_num = 0;
    if(machine->write_entry_file != false) { /* create entry file only if an entry label type was declared */
        file_entry = open_file(file_name, ".ent", "w"); /* write the entry file */
        if(file_entry == NULL)
            return false;
        lbl = get_first_label(machine->labels_table);
        while(lbl != NULL) {
            add_entry_to_file(file_entry, lbl, &line_num);
            lbl = get_next_label(lbl);
        }
        fclose(file_entry);
    }
    line_num = 0;
    if(machine->write_extern_file != false) { /* create extern file only if an extern labels were used in instructions */
        file_extern = open_file(file_name, ".ext", "w"); /* write the extern file */
        if(file_extern == NULL)
            return false;
        lbl = get_first_label(machine->labels_table);
        while(lbl != NULL) {
            add_extern_list_to_file(file_extern, lbl, &line_num);
            lbl = get_next_label(lbl);
        }
        fclose(file_extern);
    }
    return true;
}

COMMAND_TYPE get_opcode_first_command(OPCODE_TYPE opcode_type) {
    if(opcode_type == OPCODE_UNKOWN)
        return ERROR;
    if(is_opcode_group_three(opcode_type))
        return EXECUTE;
    if(is_opcode_group_one(opcode_type))
        return COMMAND_SOURCE_OPERAND;
    if(is_opcode_parameter_addresing(opcode_type))
        return COMMAND_JUMP_OPERAND;
    return COMMAND_DEST_OPERAND;
}

int is_label_legal_format(char str[], error_location *error_info) {
    int i;
    if(str == NULL)
        return false;
    if(!isalpha(str[0])) { /* the first character of a legal label name is an alphabetical letter */
        print_error_location(error_info, "the first character of a label name must be alphabetical");
        return false;
    }
    /* a legal label name cannot be a reserved word of the machine */
    if(get_opcode_type(str) != OPCODE_UNKOWN)
        return false;
    if(is_legal_register(str) != false)
        return false;
    for(i = 1; i < MAX_LABEL_NAME && str[i] != '\0'; i++)
        if(isalpha(str[i]) == false && isdigit(str[i]) == false) { /* every character other then the first must be an alphabetical letter of a digit */
            print_error_location(error_info, "a label name can contain only alphabetical letters and digits");
            return false;
        }
    if(i == MAX_LABEL_NAME) { /* the maximum size of a legal label name is MAX_LABEL_LENGTH */
        if(str[i] != '\0') {
            print_error_location(error_info, "the argument \"%s\" is bigger then the maximum label name length: %i", str, MAX_LABEL_NAME);
            return false;            
        }
    }
    return true;
}

void add_new_data(machine *machine, int value, error_location *error_info) {
    if(machine == NULL) return;
    if(machine->DC >= MAX_DATA) {
        print_error_location(error_info, "the maximum number of data the machine can store from labels with the label type LABEL_DATA or LABEL_STRING is %i", MAX_DATA);
        exit(0);
    }
    machine->data_array[machine->DC++] = value;
}