#ifndef LABEL_H
#define LABEL_H

#define MAX_LABEL_NAME 30 /* the maximum length of a label name is 30 */

/**
 * An enum used for determining the label type of the labels.
*/
typedef enum LABEL_TYPE
{
    LABEL_UNKOWN, /* the label type is illegal or the label was created */
    LABEL_DATA,
    LABEL_STRING,
    LABEL_ENTRY,
    LABEL_EXTERN,
    LABEL_CODE
} LABEL_TYPE;

/**
 * A data structure that is used for storing system labels, that contains the name, label type and value of each label. 
 * Also the structure is linked (linked list).
*/
typedef struct  label_t label;

/**
 * A data structure that is used for storing linked labels that contains the first label and last label in line.
*/
typedef struct label_table_t label_table;

/**
 * Creates an empty label and sets it's label type to LABEL_UNKOWN.
 * @returns the newly created label.
*/
label *create_empty_label();

/**
 * Creates an empty label table.
 * @returns the newly created label table.
*/
label_table *create_empty_label_table();

/**
 * Adds to a label an extern word index only if the label type is LABEL_EXTERN.
 * @param lbl a given label.
 * @param value a given extern word index.
*/
void label_add_extern_word_index(label *lbl, int value);

/**
 * Creates an empty label and sets its name to a given name.
 * After that inserts the newly created label to the end of a given label table and returns the new label.
 * @param table a given label table.
 * @param name a given name.
 * @returns the newly created label, returns NULL if failed to create.
*/
label *add_to_label_table(label_table *table, char *name);

/**
 * Searches a given label table to find a label that its name equals to a given name. 
 * If this label exists then return the first occurrence in the label table.
 * @param table a given label table.
 * @param name a given name.
 * @returns the label that its name equals to a given name, if not exists then returns NULL.
*/
label *search_in_label_table(label_table *table, char name[]);

/**
 * Returns the label type of a given label.
 * @param lbl a given label.
 * @returns the label type of the label, if the label equals to NULL then returns LABEL_UNKOWN.
*/
LABEL_TYPE label_get_type(label *lbl);

/**
 * Sets the label type of a given label to a given label type and 
 * checks if the label type is LABEL_EXTERN. If yes then creates an extern list for the label, 
 * otherwise sets the label extern list to NULL.
 * @param head a given label.
 * @param label_type a given label type.
*/
void label_set_type(label *head, LABEL_TYPE label_type);

/**
 * Returns the label IC value.
 * @param lbl a given label.
 * @returns the label IC value, if the label equals to NULL then returns 0.
*/
int label_get_IC(label *lbl);

/**
 * Sets the label IC value to a given integer.
 * @param lbl a given label.
 * @param IC the label IC value.
*/
void label_set_IC(label *lbl, int IC);

/**
 * Returns the label DC value.
 * @param lbl a given label.
 * @returns the label DC value, if the label equals to NULL then returns 0.
*/
int label_get_DC(label *lbl);

/**
 * Ascend a given label DC value by 1;
 * @param lbl a given label.
*/
void label_ascend_DC(label *lbl);

/**
 * Sets the entry status of a given label: if true\1 then the label will be used in the entry file.
 * @param lbl a given label.
 * @param status 
*/
void label_set_entry_status(label *lbl, int status);

/**
 * Returns the next label after a given label.
 * @param lbl a given label.
 * @returns the next label to use after a given label, if lbl is NULL then returns NULL.
*/
label *get_next_label(label *lbl);

/**
 * Returns the first label of a label table.
 * @param table a given label table.
 * @returns the first label of the label table.
*/
label *get_first_label(label_table *table);

/**
 * Free the memory a given label contains from the system memory.
 * @param head a given label.
*/
void free_label(label *head);

/**
 * Free the memory a given label table contains from the system memory.
 * @param table a given label table.
*/
void free_label_table(label_table *table);

/**
 * Checks if the label type of a given label is LABEL_EXTERN, and if true 
 * then adds its extern list to a given file in the following way: 
 * for every extern member create a new line with the label name, 1 gap and the extern word number.
 * for every extern that the function writes to the file append line_num by 1.
 * @param file a given file to write extern list on.
 * @param lbl a given label.
 * @param line_num a given line number.
*/
void add_extern_list_to_file(FILE *file, label *lbl, int *line_num);

/**
 * Checks if a given label entry status is true\1 
 * and if so then adds the label name, 1 gap and the label IC value to a given file.
 * if the file is entry then append line_num by 1.
 * @param file a given file to write entry on.
 * @param lbl a given label.
 * @param line_num a given line number.
*/
void add_entry_to_file(FILE *file, label *lbl, int *line_num);

/**
 * Returns the label type to identify a given string.
 * @param str a given string.
 * @returns the label type to identify a given string, 
 * if the string is not a legal label type name then returns LABEL_UNKOWN.
*/
LABEL_TYPE get_label_type(char str[]);

/**
 * Returns the name of the labelz type.
 * @param label_type a given label type.
 * @returns the name of the label type, 
 * if the label type is LABEL_UNKOWN or not a known label type then returns NULL.
*/
char *get_label_type_name(LABEL_TYPE label_type);

#endif