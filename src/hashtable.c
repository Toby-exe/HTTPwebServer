/**
 * @file hashtable.c
 * @authors Jayden Mingle
 *
 * @date 2023-11-30
 */
#include "hashtable.h"

/**
 * Since a hash table can store any type of data, there isn't one function that can
 * that can be used to hash every type of value. For example, our server uses this library
 * for storing and retrieving MIME types. This involves hashing a string 
 * representing a file extension into an index. Another different program could be a 
 * bank account manager. In that case, the key could be a long representing the account number of a customer
 * which would need a very different hash function.
*/
extern HashTable *Hashtable_create(int size, int (*hashf)(void *, int, int))
{
    if (size < 1) {
        size = DEFAULT_SIZE;
    }

    if (hashf == NULL) {    //use default hash function
        hashf = str_hashf;
    }

    HashTable *ht = (HashTable *)calloc(1, sizeof(HashTable));

    if (ht == NULL) {   // calloc failed
        return NULL;
    }

    ht->size = size;
    ht->num_entries = 0;
    ht->load_factor = 0;
    ht->buckets = malloc(size * sizeof(LinkedList *));
    ht->hashf = hashf;

    for (int i = 0; i < size; i++) {
        ht->buckets[i] = llist_create();
    }

    return ht;
}

void Hashtable_destroy(HashTable *ht)
{
    for (int i = 0; i < ht->size; i++) 
    {
        LinkedList *list = ht->buckets[i];

		llist_foreach(list, free_htentry, NULL);
        llist_destroy(list);
    }

    free(ht);
}

/**
 * Put to hash table with a string key (for LRU cache)
 */
void *Hashtable_put(HashTable *ht, char *key, void *data)
{
    return Hashtable_put_bin(ht, key, strlen(key), data);
}

/**
 * Put to hash table with a binary key
 */
void *Hashtable_put_bin(HashTable *ht, void *key, int key_size, void *data)
{
    int index = ht->hashf(key, key_size, ht->size);

    // printf("hashed key is %d\n", index);

    LinkedList *list = ht->buckets[index];

    HtEntry *entry = malloc(sizeof(HtEntry));

    entry->key = malloc(key_size);
    memcpy(entry->key, key, key_size);

    entry->key_size = key_size;
    entry->hashed_key = index;
    entry->data = data;

    if (llist_append(list, entry) == NULL) 
    {
        free(entry->key);
        free(entry);
        return NULL;
    }

    Hashtable_update(ht, 1);

    return data;
}

/**
 * Get value from the hash table with a string key (for LRU cache)
 */
void *Hashtable_get(HashTable *ht, char *key)
{
    return Hashtable_get_bin(ht, key, strlen(key));
}

/**
 * Get from the hash table with a binary data key
 */
void *Hashtable_get_bin(HashTable *ht, void *key, int key_size)
{
    int index = ht->hashf(key, key_size, ht->size);

    LinkedList *list = ht->buckets[index];

    HtEntry cmp_entry;
    cmp_entry.key = key;
    cmp_entry.key_size = key_size;

    HtEntry *val_entry = llist_find(list, &cmp_entry, Hashtable_cmpfn);

    if (val_entry == NULL) {
        return NULL;
    }

    return val_entry->data;
}

/**
 * Delete from the hashtable by string key (for LRU cache)
 */
void *Hashtable_delete(HashTable *ht, char *key)
{
    return Hashtable_delete_bin(ht, key, strlen(key));
}

/**
 * Delete from the hashtable by binary key
 *
 * NOTE: does *not* free the data--just free's the hash table entry
 */
void *Hashtable_delete_bin(HashTable *ht, void *key, int key_size)
{
    int index = ht->hashf(key, key_size, ht->size);

    LinkedList *list = ht->buckets[index];

    HtEntry cmp_entry;  //making this a pointer and mallocing fixes double free
    cmp_entry.key = key;
    cmp_entry.key_size = key_size;

    HtEntry *del_entry = llist_delete(list, &cmp_entry, Hashtable_cmpfn);
    
	if (del_entry == NULL) {
		return NULL;
	}

	void *data = del_entry->data;

    // free_htentry(del_entry, NULL);
    free(del_entry);
    
    Hashtable_update(ht, -1);

	return data;
}

/**
 * @brief Do something for each entry in a hash table
 *
 * @param[in] ht A pointer to a hash table
 * @param[in] f The function to perform on each entry
 * @param[in] arg arguments of the applied function
 */
extern void Hashtable_foreach(HashTable *ht, void (*f)(void *, void *), void *arg)
{
    return;
}

/**
 * @brief Resizes a hashtable based on a growth factor
 * 
 * Details: Since chaining is used for collision resolution, there may be cases where hash table entries
 *          pile up in one bucket, increasing search time. This issue is resolved by resizing the table 
 *          if load_factor grows above 60% or if a single list gets longer than 5 elements.
 * 
 * @param[in] ht A pointer to the hash table to resize
*/
extern HashTable *Hashtable_resize(HashTable *ht)
{
    return NULL;
}

void free_htentry(void *htent, void *arg)
{
    (void)arg;  // function passed into foreach must have 2 parameters
    
    HtEntry *free_ent = (HtEntry *)htent;
    //free(free_ent->data); //can't do this because delete needs data to return
    free(free_ent->key);
	free(htent);
}

/**
 * Don't need to compare the entries to see if they're literally the same Hash table entry
 * they just need to have the same key.
*/
extern int Hashtable_cmpfn(void *a, void *b)
{
    HtEntry *entryA = a, *entryB = b;

    int diff = entryB->key_size - entryA->key_size;

    //sizes of the keys are different; they can't be equal
    if (diff) {
        return diff;
    }

    return memcmp(entryA->key, entryB->key, entryA->key_size);
}

void Hashtable_update(HashTable *ht, int d)
{
    ht->num_entries += d;
    ht->load_factor = (float)(ht->num_entries / ht->size);
}

/**
 * Details: based on the Robin-Karp rolling hash function
 *          This hash function turns a string key into an index in the buckets array
 *          and each bucket contains a linked list.
 */
int str_hashf(void *key, int data_size, int table_size) 
{
    const int R = 31; // Small prime
    int h = 0;
    unsigned char *p = key;

    for (int i = 0; i < data_size; i++) {
        h = (R * h + p[i]) % table_size;
    }

    return h;
}

void Hashtable_display(HashTable *ht, void (*printfn)(void *))
{
    for(int i = 0; i < ht->size; i++)
    {
        /* modified version of llist_print(ht->buckets[i], typefn); */
        LinkedList *list = ht->buckets[i];
        Node *curr_node = list->head;

        if(list->length == 0)
        {
            printf("+---+        ");
            printf("\n");
            printf("|   |        ");
            printf("\n");

            if(i == ht->size - 1)
                printf("+---+        ");

            continue;
        }

        for(int j = 0; j < list->length; j++) {
            printf("+---+        ");
        }
        printf("\n");

        while (curr_node != NULL) {
            HtEntry *entry = (HtEntry *)curr_node->data;
            printfn(entry->data);

            if(curr_node->next != NULL)
                printf("  --->  ");

            curr_node = curr_node->next;
        }
        printf("\n");
    }
}
