/**
 * @file hashtable.c
 * @authors Jayden Mingle
 *
 * @date 2023-11-30
 */
#include "hashtable.h"

/**
 * @brief Creates a new hash table
 *
 * Details: This function creates a new hash table of a given size and uses the provided hash function. 
 *          If the function receives an invalid size or isn't given a hash function, default values are used.
 * 
 * @param[in] size The size of the hash table
 * @param[in] hashf The hash function to be used
 * @return A pointer to the newly created hash table, or NULL if memory allocation failed
 */
extern HashTable *Hashtable_create(int size, int (*hashf)(void *, int, int))
{
    if (size < 1) {
        size = DEFAULT_SIZE;
    }

    if (hashf == NULL) {    // use default hash function
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

/**
 * @brief Destroys a hash table
 *
 * Details: This function deallocates the memory used by the hash table and its entries.
 *          The linked list in each bucket is destroyed as well as the HtEntry's stored in each
 *          node. However the key and data inside an entry are not freed. The caller is responsible
 *          for this.
 * 
 * @param[in] ht The hash table to be destroyed
 */
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
 * @brief Inserts a key-value pair into a hash table using a string key
 *
 * Details: Wrapper function that specifies use of a string key.
 * 
 * @param[in] ht A pointer to a hash table
 * @param[in] key A pointer to a string key
 * @param[in] data A pointer to a value
 * @return The value that was inserted, or NULL if the insertion failed
 */
void *Hashtable_put(HashTable *ht, char *key, void *data)
{
    return Hashtable_put_bin(ht, key, strlen(key), data);
}

/**
 * @brief Inserts a key-value pair into the hash table
 *
 * Details: This function inserts a key-value pair into the hash table. The key and
 *          value can point to an object of any data type. Creates a HtEntry instance
 *          containing the passed in key and value and appends it to the end of
 *          the bucket that the key hashes to. The hash table's size and load_factor are updated accordingly
 * 
 * @param[in, out] ht A pointer to a hash table
 * @param[in] key A pointer to a key
 * @param[in] key_size The size of the key
 * @param[in] data A pointer to a value
 * @return The value that was inserted, or NULL if the insertion failed
 */
void *Hashtable_put_bin(HashTable *ht, void *key, int key_size, void *data)
{
    int index = ht->hashf(key, key_size, ht->size);

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
 * @brief Retrieves a value from the hash table using a string key
 *
 * Details: Wrapper function that specifies use of a string key.
 * 
 * @param[in] ht A pointer to a hash table
 * @param[in] key A pointer to a string key
 * @return The value associated with the key, or NULL if the key is not found
 */
void *Hashtable_get(HashTable *ht, char *key)
{
    return Hashtable_get_bin(ht, key, strlen(key));
}

/**
 * @brief Retrieves a value from the hash table
 *
 * Details: This function uses the hashed key to determine which bucket the value is in.
 *          Using a custom comparison function to compare two HtEntry's, it searches for
 *          a matching value in the bucket's linked list. 
 * 
 * @param[in] ht A pointer to a hash table
 * @param[in] key A pointer to a key
 * @param[in] key_size The size of the key
 * @return The value associated with the key, or NULL if the key is not found
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
 * @brief Deletes a key-value pair from the hash table using a string key
 *
 * Details: Wrapper function that specifies use of a string key.
 * 
 * @param[in] ht A pointer to a hash table
 * @param[in] key A pointer to a string key
 * @return The value that was deleted, or NULL if the key was not found
 */
void *Hashtable_delete(HashTable *ht, char *key)
{
    return Hashtable_delete_bin(ht, key, strlen(key));
}


/**
 * @brief Retrieves a value from the hash table
 *
 * Details: Uses the same method as Hashtable_get() to find the HtEntry containing the
 *          desired key. The HtEntry is removed from the linked list and it's memory is
 *          deallocated. note that it does not free the data - just free's the hash table entry.
 *          The hash table's size and load_factor are updated accordingly
 * 
 * @param[in, out] ht A pointer to a hash table
 * @param[in] key A pointer to a key
 * @param[in] key_size The size of the key
 * @return The value that was deleted, or NULL if the key was not found
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
 * @brief Frees a hash table entry
 *
 * Deatils: This function frees only a hash table entry, not the key and data inside of it.
 * 
 * @param[in] htent The hash table entry to be freed
 * @param[in] arg An argument that is not used in this function
 */
void free_htentry(void *htent, void *arg)
{
    (void)arg;
	free(htent);
}

/**
 * @brief Compares two hash table entries
 *
 * Details: This function compares two hash table entries based on their keys. 
 *          It does not check if they are the same entry.
 * 
 * @param[in] a The first hash table entry
 * @param[in] b The second hash table entry
 * @return returns 0 if the keys of the hash table entries are identical, non-zero otherwise
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

/**
 * @brief Updates the number of entries and the load factor of the hash table
 *
 * Details: These values of a hash table are updated based on a given delta.
 * 
 * @param[in] ht The hash table
 * @param[in] d The delta to be added to the number of entries
 */
void Hashtable_update(HashTable *ht, int d)
{
    ht->num_entries += d;
    ht->load_factor = (float)(ht->num_entries / ht->size);
}

/**
 * @brief Hashes a string key into an index in the buckets array.
 * 
 * @param[in] key The key to be hashed
 * @param[in] data_size The size of the key
 * @param[in] table_size The size of the hash table
 * @return The hashed key
 */
int str_hashf(void *key, int data_size, int table_size) 
{
    const int R = 31; // random prime number
    int hashed_key = 0;
    unsigned char *c = key;

    for (int i = 0; i < data_size; i++) {
        hashed_key = (R * hashed_key + c[i]) % table_size;
    }

    return hashed_key;
}

/**
 * @brief Displays the hash table
 *
 * Details: This function prints each bucket of the hash table as a linked list, displaying
 *          each node's data.
 *          Uses a similar format as llist_display():
 * 
 *          +---+        +---+        +---+
 *          | a |  --->  | b |  --->  | c | 
 *          +---+        +---+        +---+
 *          | d |  --->  | e |
 *          +---+        +---+
 * 
 * @param[in] ht The hash table
 * @param[in] printfn The function used to print the data in the hash table entries
 */
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