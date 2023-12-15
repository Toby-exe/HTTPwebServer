/**
 * @file hashtable.h
 * @brief A library for hash table operations
 * @authors Jayden Mingle
 * 
 * Details:
 * This file contains the structure definitions and function prototypes for creating 
 * and manipulating a hash table. 
 * A hash table in this library is generic, meaning you can make a hash table
 * that uses any data type as a key and that can store any data type. Chaining 
 * is used for collision resolution via the linked list library.
 * 
 * Assumptions/Limitations:
 * This implementation assumes that the key and data stored in each hash table entry has been
 * properly allocated and that the user is responsible for managing this memory. 
 * Since the key and data fields of an entry can contain anything, 
 * when we delete an entry from the hash table using the entry's key, 
 * we only free the entry itself. We assume that freeing the data
 * will be done by the caller at the appropraite time before the program executes.
 * 
 * @date 2023-12-1
 */
#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "linkedlist.h"

#define DEFAULT_SIZE 16 // 128
#define DEFAULT_GROW_FACTOR 2

/* ----------{ STRUCTURES AND TYPES }---------- */

typedef struct {
    void *key;
    int key_size;
    int hashed_key;
    void *data;
} HtEntry;  /* Each node in a bucket contains a HtEntry */

typedef struct {
    LinkedList **buckets;
    int size;
    int num_entries;
    float load_factor;
    int (*hashf)(void *data, int data_size, int bucket_count);
} HashTable;

/* ----------{ FUNCTION PROTOTYPES }---------- */
/* ----------{ Creation and Cleanup }---------- */
extern HashTable *Hashtable_create(int size, int (*hashf)(void *, int, int));
extern void Hashtable_destroy(HashTable *ht);

/* ----------{ Hash functions }---------- */
extern int str_hashf(void *key, int data_size, int table_size);     /* default hash function */

/* ----------{ Insert methods }---------- */
extern void *Hashtable_put(HashTable *ht, char *key, void *data);   //Hashtable_put_str
extern void *Hashtable_put_bin(HashTable *ht, void *key, int key_size, void *data);//general method should be called Hashtable_put

/* ----------{ Search methods }---------- */
extern void *Hashtable_get(HashTable *ht, char *key);
extern void *Hashtable_get_bin(HashTable *ht, void *key, int key_size);

/* ----------{ Remove methods }---------- */
extern void *Hashtable_delete(HashTable *ht, char *key);
extern void *Hashtable_delete_bin(HashTable *ht, void *key, int key_size);

/* ----------{ Helper Methods }---------- */
extern void free_htentry(void *htent, void *arg);
extern int Hashtable_cmpfn(void *a, void *b);
extern void Hashtable_update(HashTable *ht, int d);

/* ----------{ Output methods for testing }---------- */
extern void Hashtable_display(HashTable *ht, void (*printfn)(void *));

#endif