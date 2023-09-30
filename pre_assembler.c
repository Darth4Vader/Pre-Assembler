#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pre_assembler.h"
#include "utils.h"

struct line_t
{
    char *line_context; /* the line context of the current line */
    struct line_t *next; /* the next line */
};

struct line_list_t
{
    line *head; /* the first line in the list */
    line *root; /* the last line in the list */
};

struct macro_t
{
    char *name; /* the macro name */
    line_list *lines; /* the lines that the macro contains */
    struct macro_t *next; /* the next macro */
};

struct macro_table_t
{
    macro *head; /* the first macro in the list */
    macro *root; /* the last macro in the list */
};

int pre_assembler(char *file_name) {
    /* the size of the 2 string is MAX_COMMAND_LEN + 1 to check if the string fgets is longer than MAX_COMMAND_LEN */
    char command[MAX_COMMAND_LEN+1]; /* the current line */
    char str[MAX_COMMAND_LEN+1]; /* the current argument */
    int flag_mcr_on = false; /* check if a macro was declared */
    int check_mcr_name = false; /* checks if to check o a macro name declaration */
    int line_end = false; /* checks if the line had reached the first or second word and can read the next line */
    macro_table *macro_table; /* the machine macro table */
    macro *mcr; /* the current macro */
    FILE *input; /* the .as file to read */
    FILE *output; /* the .am file to write */
    error_location *error_info; /* the error location to print when an error occurs */
    char c;
    int i;
    int length = 0;
    int error = false; /* check if then current line has macro errors */
    int file_errors = false; /* check if then file has macro errors */
    input = open_file(file_name, ".as", "r");
    if(input == NULL) /* check if the .as file had been opened successfully */
        return true;
    output = open_file(file_name, ".am", "w");
    if(output == NULL) { /* check if the .am file had been opened successfully */
        fclose(input); /* close the .as file to insure safety */
        return true;
    }
    error_info = create_empty_error_location(get_file_full_name(file_name, ".as"));
    macro_table = create_empty_macro_table();
    while((fgets(command, MAX_COMMAND_LEN+1, input)) != NULL) {
        error_location_ascend_line(error_info);
        error_location_set_index(error_info, 0);
        line_end = false;
        check_mcr_name = false;
        error = false;
        length = 0;
        str[0] = '\0';
        if(strlen(command) >= MAX_COMMAND_LEN && is_end_null(command[MAX_COMMAND_LEN-1]) == false) { /* check if the string length is longer then MAX_COMMAND_LEN becuase the maximum length of a line in the .am file is MAX_COMMAND_LEN */
            if(command[0] != ';') { /* comment lines are not part of the .am file and thus are not exceptions */
                print_error_location(error_info, "The line length is bigger than the maximum length of a line in the .am file that is %i", (MAX_COMMAND_LEN));
                error = true;
            }
            while((fgets(command, MAX_COMMAND_LEN+1, input)) != NULL) { /* move to the next line or to the end of file (if this is the last line in the file) */
                if(strchr(command, '\n') != false) /* check if the next fgets will give a new line */
                    break;
            }
        } /* comment lines are ignored */
        else if(command[0] != ';') {
            for(i = 0; i < MAX_COMMAND_LEN && (line_end == false || check_mcr_name == true); i++) { /* read only the first word (non whitespace characters) if check_mcr_name is false otherwise read also the second word */
                error_location_set_index(error_info, i);
                c = command[i];
                if(is_ascii(c) == false) { /* only ascii characters are allowed */
                    print_error_location(error_info, "The character is not asscii");
                    error = true;
                }
                if(is_whitespace(c) == false && is_end_null(c) == false) {
                    str[length++] = c;
                    str[length] = '\0';
                }
                else if(length > 0) {
                    line_end = true; /* the first word had been read */
                    if(check_mcr_name != false) { /* if str a macro name declaration then check if it is not a machine used word */
                        if(get_opcode_type(str) != OPCODE_UNKOWN || is_macro_opcode(str)) { /* a macro name can't be an opcode */
                            print_error_location(error_info, "the opcode 5\"%s\" can't be used as a macro name declaration", str);
                            error = true;
                        }
                        if(is_legal_register(str) != false) { /* a macro name can't be a register */
                            print_error_location(error_info, "registers can't be used as a macro name declaration");
                            error = true;
                        }
                        if(search_in_macro_table(macro_table, str) != NULL) { /* every macro name must be unique */
                            print_error_location(error_info, "the macro \"%s\" had already been declared before", str);
                            error = true;
                        }
                        else {
                            mcr = add_to_macro_table(macro_table, strdup(str));
                            check_mcr_name = false;
                        }
                    }
                    else { /* the first word of the command */
                        macro *head = search_in_macro_table(macro_table, str);
                        if(head != NULL) { /* if str is a macro name then replace it with every line in the macro */
                            line *lines = get_first_line(head->lines);
                            while(lines != NULL) {
                                if(flag_mcr_on != false) /* if the flag_mcr_on is true then copy the current macro lines into the declared macro */
                                    add_to_line_list(mcr->lines, strdup(lines->line_context));
                                else /* else puts the line into the .am file */
                                    fprintf(output, "%s", lines->line_context);                            
                                lines = lines->next;
                            }
                        }
                        else if(flag_mcr_on != false) { /* if a macro was declared */
                            if(strcmp(str, "endmcr") == 0) /* checks if the macro closes */
                                flag_mcr_on = false;
                            else /* puts the current line into the macro */
                                add_to_line_list(mcr->lines, strdup(command));
                        }
                        else if(strcmp(str, "mcr") == 0) { /* if the first word is mcr then read the second word */
                                check_mcr_name = true;
                                flag_mcr_on = true;
                                line_end = false;
                            }
                        else /* puts the crrent line into the .am file */
                            fprintf(output, "%s", command);                    
                        }
                    length = 0; /* resets the current argument */
                    str[0] = '\0';
                }
                if(is_end_null(c) != false || error == true) /* if the line has an error or the current character is end null then stop reading the current line */
                    break;
            }
            if(error != false) /* if the command type is ERROR then the function will return true */
                file_errors = true;
            else if(line_end == false) { /* if the line has only whitespace characters or empty */
                if(flag_mcr_on != false) /* if the flag_mcr is true the add the current line into the macro */
                    add_to_line_list(mcr->lines, strdup(command));
                else /* add it into the .am file */
                    fprintf(output, "%s", command);
            }
        }
    }
    fclose(output); /* closes the files to insure safety */
    fclose(input);
    free_macro_table(macro_table); /* free the macro table */
    free_error_location(error_info);
    return file_errors;
}

macro_table* create_empty_macro_table() {
    macro_table *list = (macro_table*) malloc_and_check(sizeof(macro_table));
    list->head = NULL;
    list->root = NULL;
    return list;
}

macro *create_empty_macro() {
    macro *head = (macro*) malloc_and_check(sizeof(macro));
    head->name = NULL;
    head->lines = (line_list*) malloc_and_check(sizeof(line_list));
    head->lines->head = NULL;
    head->lines->root = NULL;
    head->next = NULL;
    return head;
}

line *create_empty_line() {
    line *head = (line*) malloc_and_check(sizeof(line));
    head->line_context = NULL;
    head->next = NULL;
    return head;
}

line *add_to_line_list(line_list *list, char str[]) {
    line *root;
    if(list == NULL) return NULL;
    root = create_empty_line();
    if(list->head == NULL)
        list->head = root;
    else if(list->root != NULL)
        list->root->next = root;
    root->line_context = str;
    list->root = root;
    return root;
}

macro *add_to_macro_table(macro_table *table, char name[]) {
    macro *root;
    if(table == NULL) return NULL;
    root = create_empty_macro();
    if(table->head == NULL)
        table->head = root;
    else if(table->root != NULL)
        table->root->next = root;
    root->name = name;
    table->root = root;
    return root;
}

line *get_first_line(line_list *list) {
    return list != NULL ? list->head : NULL;
}

macro *get_first_macro(macro_table *table) {
    return table != NULL ? table->head : NULL;
}

macro *search_in_macro_table(macro_table *table, char name[]) {
    macro *head;
    if(table == NULL) return NULL;
    head = table->head;
    while(head != NULL) {
        if(head->name != NULL && strcmp(head->name, name) == 0)
            return head;
        head = head->next;
    }
    return NULL;
}

void free_line(line *head) {
    line *root;
    if(head == NULL) return;
    root = head->next;
    free(head->line_context);
    free(head);
    free_line(root);
}

void free_macro(macro *head) {
    macro *root;
    if(head == NULL) return;
    root = head->next;
    free(head->name);
    if(head->lines != NULL) {
        free_line(head->lines->head);
        free(head->lines);
    }
    free(head);
    free_macro(root);
}

void free_macro_table(macro_table *table) {
    if(table == NULL) return;
    free_macro(table->head);
    free(table);
}

int is_macro_opcode(char str[]) {
    if(strcmp(str, "mcr") == 0)
        return true;
    if(strcmp(str, "endmcr") == 0)    
        return true;
    return false;
}
