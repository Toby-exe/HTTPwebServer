/**
 * @file linkedlist.h
 * @brief A library for singly-linked list operations
 * @authors Jayden Mingle
 * 
 * Details:
 * This file contains the structure definitions and function prototypes for creating 
 * and manipulating a non-circular, singly-linked list. 
 * A linked list in this library is generic, meaning you can make
 * a linked list that stores any type in each of it's nodes.
 * 
 * Assumptions/Limitations:
 * This implementation assumes that the data stored in each node has been
 * properly allocated and that the user is responsible for managing this memory. 
 * It does not provide any mechanism for deep copying or deep comparison of data elements.
 * Since the data field of a node can contain anything, when we remove a node from the list, 
 * we only free the node itself and not the data in it. We assume that freeing the data 
 * will occur in another file at the appropraite time before the program executes.
 * 
 * @date 2023-12-2
 */
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ----------{ STRUCTURES AND TYPES }---------- */

typedef struct node {
    void *data;
	struct node *next;
} Node;

typedef struct list {
	Node *head;
	int length;
} LinkedList;

/* ----------{ FUNCTION PROTOTYPES }---------- */
/* ----------{ Creation and Cleanup }---------- */
extern LinkedList *llist_create();
extern void llist_destroy(LinkedList *list);

/* ----------{ Insert methods }---------- */
extern void *llist_insert(LinkedList *list, void *data);
extern void *llist_append(LinkedList *list, void *data);

/* ----------{ Search methods - get the data of a certain node }---------- */
extern void *llist_find(LinkedList *list, void *data, int (*cmpfn)(void *, void *));
extern void *llist_get_head(LinkedList *list);
extern void *llist_get_tail(LinkedList *list);

/* ----------{ Remove methods }---------- */
extern void *llist_delete(LinkedList *list, void *data, int (*cmpfn)(void *, void *));
extern void *llist_delete_head(LinkedList *list);
extern void *llist_delete_tail(LinkedList *list);

/* ----------{ "ArrayList" methods }---------- */
extern void llist_foreach(LinkedList *list, void (*Func)(void *, void *), void *arg);
extern void **llist_as_array(LinkedList *list);
extern void llist_array_free(void **a);

/* ----------{ Output methods for testing }---------- */
extern void llist_print(LinkedList *list, void (*printfn)(void *));
extern void print_as_int(void *n);
extern void print_as_double(void *n);
extern void print_as_float(void *n);
extern void print_as_char(void *n);
extern void print_as_string(void *n);

/* ----------{ Miscellaneous }---------- */
extern int llist_count(LinkedList *list);

#endif
