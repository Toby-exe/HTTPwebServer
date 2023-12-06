/**
 * @file hashtable.h
 * @brief A library for hash tables
 * @authors Jayden Mingle
 * 
 * Details: A hash table in this library is generic, meaning you can make a hash table
 *          that uses any type as a key and that can store any type.
 *          Chaining is used for collision resolution via the linked list library.
 * 
 * @date 2023-11-30
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
} HtEntry;                  // Each node in a bucket contains a HtEntry

typedef struct {
    LinkedList **buckets;   // OR Bucket *buckets; if chaining not used
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

/* ----------{ Miscellaneous }---------- */
extern void Hashtable_foreach(HashTable *ht, void (*f)(void *, void *), void *arg);
extern HashTable *Hashtable_resize(HashTable *ht);

/* ----------{ Helper Methods }---------- */
extern void free_htentry(void *htent, void *arg);
extern int Hashtable_cmpfn(void *a, void *b);
extern void Hashtable_update(HashTable *ht, int d);

/* ----------{ Output methods for testing }---------- */
extern void Hashtable_display(HashTable *ht, void (*printfn)(void *));

#endif