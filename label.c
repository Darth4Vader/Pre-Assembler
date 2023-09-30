#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "label.h"
#include "utils.h"

/**
 * A data structure used for storing lines the IC\machine words indexes where a given label occurs on. 
 * This structure is only used for labels with the label type LABEL_EXTERN.
 * Also the structure is linked (linked list).
*/
typedef struct extern_word
{
    int value; /* the current machine word index */
    struct extern_word *next; /* the next extern word */
} extern_word;

/**
 * A data structure that is used for storing externs words with a constant insertion time.
*/
typedef struct extern_list
{
    extern_word *head; /* the first extern word in the list */
    extern_word *root; /* the last extern in the list */
} extern_list;

struct label_t
{
    char *name; /* the name of the label */
    LABEL_TYPE type; /* the label type of the label */
    int IC; /* the machine word index of the label, only used for labels with the label type LABEL_ENTRY */
    int DC; /* the number of data that the label contains, only used for labels with the label type LABEL_DATA of LABEL_STRING */
    extern_list *externs_list; /* the label extern list, only if the label type of the label is LABEL_EXTERN */
    unsigned is_entry:1; /* set true\1 if the label is entry (used for the entry file)*/
    struct label_t *next; /* the next label */
};

struct label_table_t
{
    label *head; /* the first label in the list */
    label *root; /* the last label in the list */
};

/**
 * Creates an empty extern_word.
 * @returns the newly created extern_word.
*/
extern_word *create_empty_extern_word() {
    extern_word *head = (extern_word*) malloc_and_check(sizeof(extern_word));
    head->value = 0;
    head->next = 0;
    return head;
}

/**
 * Creates an empty extern list.
 * @returns the newly created extern list.
*/
extern_list *create_empty_extern_list() {
    extern_list *list = (extern_list*) malloc_and_check(sizeof(extern_list));
    list->head = NULL;
    list->root = NULL;
    return list;
}

label *create_empty_label() {
    label *head = (label*) malloc_and_check(sizeof(label));
    head->name = NULL;
    head->IC = 0;
    head->DC = 0;
    head->type = LABEL_UNKOWN;
    head->is_entry = false;
    head->externs_list = NULL;
    head->next = NULL;
    return head;
}

label_table *create_empty_label_table() {
    label_table *table = (label_table*) malloc_and_check(sizeof(label_table));
    table->head = NULL;
    table->root = NULL;
    return table;    
}

void label_add_extern_word_index(label *lbl, int value) {
    extern_list *list;
    extern_word *root;
    if(lbl == NULL) return;
    if(lbl->type != LABEL_EXTERN || lbl->externs_list == NULL) return; /* only labels with the label type LABEL_EXTERN can use this function to add a new extern word index */
    list = lbl->externs_list;
    root = create_empty_extern_word();
    if(list->head == NULL)
        list->head = root;
    else if(list->root != NULL)
        list->root->next = root;
    root->value = value;
    list->root = root;
}

label *add_to_label_table(label_table *table, char *name) {
    label *root;
    if(table == NULL) return NULL;
    root = create_empty_label();
    if(table->head == NULL)
        table->head = root;
    else if(table->root != NULL)
        table->root->next = root;
    root->name = name;
    table->root = root;
    return root;
}

label *search_in_label_table(label_table *table, char name[]) {
    label *head;
    if(table == NULL) return NULL;
    head = table->head;
    while(head != NULL) {
        if(head->name != NULL && strcmp(head->name, name) == 0)
            return head;
        head = head->next;
    }
    return NULL;
}

LABEL_TYPE label_get_type(label *lbl) {
    return lbl != NULL ? lbl->type : LABEL_UNKOWN;
}

void label_set_type(label *head, LABEL_TYPE label_type) {
    head->type = label_type;
    if(label_type == LABEL_EXTERN) /* if the label type of the label is LABEL_EXTERN then create an empty extern list for the label */
        head->externs_list = create_empty_extern_list();
    else
        head->externs_list = NULL;
}

int label_get_IC(label *lbl) {
    return lbl != NULL ? lbl->IC : 0;
}

void label_set_IC(label *lbl, int IC) {
    if(lbl != NULL)
        lbl->IC = IC;
}

int label_get_DC(label *lbl) {
    return lbl != NULL ? lbl->DC : 0;
}

void label_ascend_DC(label *lbl) {
    if(lbl != NULL)
        lbl->DC++;
}

void label_set_entry_status(label *lbl, int status) {
    if(lbl != NULL)
        lbl->is_entry = status;
}

label *get_next_label(label *lbl) {
    return lbl != NULL ? lbl->next : NULL;
}

/**
 * Returns the first extern_word of an extern_word list.
 * @param list a given extern_word list.
 * @returns the first extern_word of the extern_word list.
*/
extern_word *get_first_extern_word(extern_list *list) {
    return list != NULL ? list->head : NULL;
}

label *get_first_label(label_table *table) {
    return table != NULL ? table->head : NULL;
}

/**
 * Free the memory a given extern list contains from the system memory.
 * @param list a given extern list.
*/
void free_extern_list(extern_list *list) {
    extern_word *head, *next;
    if(list == NULL) return;
    next = list->head;
    while(next != NULL) {
        head = next;
        next = next->next;
        free(head);
    }
    free(list);
}

void free_label(label *head) {
    label *root;
    if(head == NULL) return;
    root = head->next;
    free(head->name);
    free_extern_list(head->externs_list);
    free(head);
    free_label(root);
}

void free_label_table(label_table *table) {
    if(table == NULL) return;
    free_label(table->head);
    free(table);
}

void add_extern_list_to_file(FILE *file, label *lbl, int *line_num) {
    extern_word *extrn; /* the extern list of the current label (if label is extern) */
    if(lbl->type == LABEL_EXTERN) { /* only if the label type is extern then add it's word counters into the extern file */
        extrn = get_first_extern_word(lbl->externs_list);
        while(extrn != NULL) {
            if(*line_num > 0)
                fprintf(file, "\n");            
            fprintf(file, "%s %i", lbl->name, extrn->value); /* adds the current label name and IC counter into the extern file */
            extrn = extrn->next;
            (*line_num)++;
        }
    }   
}

void add_entry_to_file(FILE *file, label *lbl, int *line_num) {
    if(lbl->is_entry != false) { /* add the label name and data counter only if the label was marked as entry */
        if(*line_num > 0)
            fprintf(file, "\n");
        fprintf(file, "%s %i", lbl->name, lbl->IC);
        (*line_num)++;
    }
}

LABEL_TYPE get_label_type(char str[]) {
    if(str == NULL)
        return LABEL_UNKOWN;
    if(strcmp(str, ".data") == 0)
        return LABEL_DATA;
    if(strcmp(str, ".string") == 0)
        return LABEL_STRING;
    if(strcmp(str, ".entry") == 0)
        return LABEL_ENTRY;
    if(strcmp(str, ".extern") == 0)
        return LABEL_EXTERN;
    return LABEL_UNKOWN;
}

char *get_label_type_name(LABEL_TYPE label_type) {
    if(label_type == LABEL_DATA)
        return ".data";
    if(label_type == LABEL_STRING)
        return ".string";
    if(label_type == LABEL_ENTRY)
        return ".entry";
    if(label_type == LABEL_EXTERN)
        return ".extern";
    return NULL;
}