#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

/**
 * A data structure used for storing lines from a given file. 
 * Also the structure is linked (linked list).
*/
typedef struct line_t line;

/**
 * A data structure that is used for storing linked lines with a constant insertion time.
*/
typedef struct line_list_t line_list;

/**
 * A data structure used for macros from a given file: 
 * the macro name and lines.
 * Also the structure is linked (linked list).
*/
typedef struct macro_t macro;

/**
 * A data structure that is used for storing linked macros with a constant insertion time.
*/
typedef struct macro_table_t macro_table;

/**
 * Process the macro declarations of the .as file and creates a .am file if there were no errors in the file. 
 * @param file_name the file name
 * @returns true if the file has errors, returns false otherwise
*/
int pre_assembler(char *file_name);

/**
 * Creates an empty macro table.
 * @returns the newly created macro table.
*/
macro_table* create_empty_macro_table();

/**
 * Creates an empty macro.
 * @returns the newly created macro.
*/
macro* create_empty_macro();

/**
 * Creates an empty line.
 * @returns the newly created line.
*/
line *create_empty_line();

/**
 * Creates an empty line and sets its line context to a given string.
 * After that inserts the newly created line to the end of a given line list and returns the new line.
 * @param line a given line list.
 * @param str a given string.
 * @returns the newly created line, returns NULL if failed to create.
*/
line *add_to_line_list(line_list *list, char str[]);

/**
 * Creates an empty macro and sets its name to a given name.
 * After that inserts the newly created macro to the end of a given macro table and returns the new macro.
 * @param table a given macro table.
 * @param name a given name.
 * @returns the newly created macro, returns NULL if failed to create.
*/
macro *add_to_macro_table(macro_table *table, char name[]);

/**
 * Returns the first line of a line list.
 * @param list a given line list.
 * @returns the first line of the line list.
*/
line *get_first_line(line_list *list);

/**
 * Returns the first macro of a macro table.
 * @param table a given macro table.
 * @returns the first macro of the macro table.
*/
macro *get_first_macro(macro_table *table);

/**
 * Searches a given macro table to find a macro that its name equals to a given name. 
 * If this macro exists then return the first occurrence in the macro table.
 * @param table a given macro table.
 * @param name a given name.
 * @returns the macro that its name equals to a given name, if not exists then returns NULL.
*/
macro *search_in_macro_table(macro_table *table, char name[]);

/**
 * Free the memory a given line contains from the system memory.
 * @param head a given line.
*/
void free_line(line *head);

/**
 * Free the memory a given macro contains from the system memory.
 * @param head a given macro.
*/
void free_macro(macro *head);

/**
 * Free the memory a given macro table contains from the system memory.
 * @param table a given macro table.
*/
void free_macro_table(macro_table *table);

/**
 * Checks if a given string is a macro opcode: mcr or endmcr
 * @param str a given string
 * @returns true if str is a mcro opcode, returns false otherwise.
*/
int is_macro_opcode(char str[]);

#endif