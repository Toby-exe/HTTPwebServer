/**
 * @file linkedlist.c
 * @authors Jayden Mingle
 *
 * @date 2023-11-30
 */
#include "linkedlist.h"

/**
 * @brief Allocates a new instance of a singly linked list
 * 
 * @return The new linked list
 */
LinkedList *llist_create()
{
    LinkedList *new_list = (LinkedList *)calloc(1, sizeof(LinkedList));
    new_list->head = NULL;
    new_list->length = 0;

    return new_list;
}

/**
 * @brief inserts a new node at the head of a linked list
 *
 * Details: the new node will hold the data passed in by the function.
 *
 * @param[in, out] list A pointer to a linked list 
 * @param[in] data A pointer to some data to be stored in the new node
 * @return A pointer to the data that was inserted, or NULL if memory allocation failed
 */
void *llist_insert(LinkedList *list, void *data)
{
    Node *new_node = (Node *)calloc(1, sizeof(Node));

    if(new_node == NULL) {  //calloc failed
        return NULL;
    }

    new_node->data = data;
    new_node->next = list->head;
    list->head = new_node;

    list->length++;

    return data;
}

/**
 * @brief inserts a new node at the tail of a linked list
 *
 * Details: the new node will hold the data passed in by the function.
 *
 * @param[in, out] list A pointer to a linked list 
 * @param[in] data A pointer to some data to be stored in the new node
 * @return A pointer to the data that was inserted, or NULL if memory allocation failed.
 */
void *llist_append(LinkedList *list, void *data)
{
    Node *tail = list->head;

    if(tail == NULL) {  // if the list is empty just insert at head
        return llist_insert(list, data);
    }

    Node *new_node = (Node *)calloc(1, sizeof(Node));

    if(new_node == NULL) {  // calloc failed
        return NULL;
    }

    // traverse through the list to find the last node
    while(tail->next != NULL) {
        tail = tail->next;
    }

    new_node->data = data;
    tail->next = new_node;

    list->length++;

    return data;
}

/**
 * @brief finds the first element of a list
 *
 * @param[in] list A pointer to a linked list
 * @return A pointer to the data at the head of the linked list
 */
void *llist_get_head(LinkedList *list)
{
    if (list->head == NULL) {
		return NULL;
	}

	return list->head->data;
}

/**
 * @brief finds the last element of a list
 *
 * @param[in] list A pointer to a linked list 
 * @return A pointer to the data at the tail of the linked list
 */
void *llist_get_tail(LinkedList *list)
{
    Node *tail = list->head;

    if (tail == NULL) { // empty list
		return NULL;
	}

    while(tail->next != NULL) {
        tail = tail->next;
    }

    return tail->data;
}

/**
 * @brief checks if a list contains a certain element
 *
 * Details: Since there's no way to cover every case of what can be stored in data, 
 *          llist_find() can be given any function that compares any two things. 
 *          cmpfn (the comparison function) returns 0 if the node's data is equal.
 *          
 * @param[in] list A pointer to a linked list 
 * @param[in] data A pointer to some data to find in a node
 * @param[in] cmpfn(data1, data2) A function that compares two values of any type
 * @return A pointer to the data in a node in the linked list
 */
void *llist_find(LinkedList *list, void *data, int (*cmpfn)(void *, void *))
{
    Node *node = list->head;

    if (node == NULL) { // empty list
		return NULL;
	}

    while(node != NULL)
    {
        if (cmpfn(data, node->data) == 0) {
			break;
		}

		node = node->next;
    }

    if (node == NULL) { // traversed whole list and node wasn't found
		return NULL;
	}

    return node->data;
}

/**
 * @brief deletes an element in the list
 *
 * Details: Deletes the first instance of a node that holds the data. Since there's no 
 *          way to cover every case of what can be stored in data, llist_delete() can be 
 *          given any function that compares any two things. cmpfn (the comparison function) 
 *          returns 0 if the node's data is equal.
 *          
 *          A node in the list is freed but the node's data is not. Since this LinkedList 
 *          library is designed to be generic, there can be a linked list of structures.
 *          Those structs may also have their own fields that should be freed in the event 
 *          that the node is deleted. So freeing data is left for the libraries of those
 *          structures to handle.
 * 
 * @param[in] list A pointer to a linked list 
 * @param[in] data A pointer to some data to find in a node
 * @param[in] cmpfn(data1, data2) A function that compares two values of any type
 * @return A pointer to the data at a node in the linked list
 */
void *llist_delete(LinkedList *list, void *data, int (*cmpfn)(void *, void *))
{
    Node *curr = list->head;
    Node *prev = NULL;

    while(curr != NULL)
    {
        if (cmpfn(data, curr->data) == 0)
        {
            void *data = curr->data;

            if(prev == NULL) {  // node is the head of the list
                list->head = curr->next;
            } else {
                prev->next = curr->next;
            }

            free(curr);         // Free the node

            list->length--;
            return data;
		}

        prev = curr;
		curr = curr->next;
    }

    return NULL;
}

/**
 * @brief deletes the first element of a list
 * 
 * Details: The first node of the linked list is freed but its data is not.
 *          *** See explanation in llist_delete() ***
 *
 * @param[in] list A pointer to a linked list
 * @return A pointer to the data at the head of the linked list
 */
void *llist_delete_head(LinkedList *list)
{
    if (list->head == NULL) {   // list is empty
		return NULL;
	}

    Node *temp = list->head;
    void *data = temp->data;
    list->head = list->head->next;

    free(temp);
    list->length--;

    return data;
}

/**
 * @brief deletes the last element of a list
 * 
 * Details: The last node of the linked list is freed but its data is not.
 *          *** See explanation in llist_delete() ***
 *
 * @param[in] list A pointer to a linked list 
 * @return A pointer to the data at the head of the linked list
 */
void *llist_delete_tail(LinkedList *list)
{
    if (list->head == NULL) {   // empty list
        return NULL;
    }

    Node *temp = list->head;
    Node *prev = NULL;
    
    while (temp->next != NULL) 
    {
        prev = temp;
        temp = temp->next;
    }

    void *data = temp->data;
    
    if (prev != NULL) {
        prev->next = NULL;
    } else {
        list->head = NULL;
    }

    free(temp);
    list->length--;

    return data;
}


/**
 * @brief destroys a linked list
 *
 * Details: Each node of the linked list is freed but its data is not.
 *          *** See explanation in llist_delete() ***
 *
 * @param[in] list The linked list to be destroyed and freed
 */
void llist_destroy(LinkedList *list)
{
    Node *curr = list->head;
    Node *next = NULL;

    while(curr != NULL)
    {
        next = curr->next;
        free(curr);         // Free the node
        curr = next;
    }

    free(list);
}

/**
 * @brief get the number of elements in a linked list
 *
 * @param[in] list A pointer to a linked list
 * @return the length of the given linked list
 */
int llist_count(LinkedList *list)
{
    return list->length;
}

/**
 * @brief Perform some operation on each item in a list
 *
 * @param[in] list A pointer to a linked list
 * @param[in] Func The function to perform on a node's data
 * @param[in] arg The arguments received by the function
 */
void llist_foreach(LinkedList *list, void (*Func)(void *, void *), void *arg)
{
    Node *curr = list->head;
    Node *next = NULL;
    
    while(curr != NULL)
    {
        next = curr->next;
        Func(curr->data, arg);
        curr = next;
    }
}

/**
 * @brief translates a linked list into a null-terminated array.
 *
 * @param[in] list A pointer to a linked list
 * @return a new array containing all the elements in the list in the correct order
 */
void **llist_as_array(LinkedList *list)
{
    if (list->head == NULL) {   // empty list
		return NULL;
	}

	void **arr = malloc((sizeof *arr) * list->length + 1);

	Node *curr;
	int i;

	for (i = 0, curr = list->head; curr != NULL; i++, curr = curr->next) {
		arr[i] = curr->data;
	}

	arr[i] = NULL;    // null terminate the new array

	return arr;
}

/**
 * @brief frees an array representing a linked list.
 * 
 * Details: Frees the array itself and not the values at each index
 *          *** See explanation in llist_delete() ***
 *
 * @param[in] a A pointer to an array
 */
void llist_array_free(void **a)
{
    free(a);
}

/**
 * @brief prints a linked list
 * 
 * Details: prints a linked list to stdout (can be modified to output to any file
 *          descriptor). It is shown in the following format:
 * 
 *          +---+        +---+        +---+
 *          | a |  --->  | b |  --->  | c | 
 *          +---+        +---+        +---+
 * 
 *          Since there's no way to cover every case of what can be stored in data, 
 *          llist_delete() can be given any function that prints a single instance
 *          of a type of data.
 *
 * @param[in] list A pointer to a linked list
 * @param[in] printfn(data) A function that prints data based on a type
 */
void llist_print(LinkedList *list, void (*printfn)(void *))
{
    Node *curr_node = list->head;

    printf("\n");

    if(list->length == 0)
    {
        printf("+---+        ");
        printf("\n");
        printf("|   |        ");
        printf("\n");
        printf("+---+        ");
        printf("\n");
        return;
    }

    for(int i = 0; i < list->length; i++) {
        printf("+---+        ");
    }
    printf("\n");

    while (curr_node != NULL) {
        printfn(curr_node->data);

        if(curr_node->next != NULL)
            printf("  --->  ");

        curr_node = curr_node->next;
    }
    printf("\n");
    
    for(int i = 0; i < list->length; i++) {
        printf("+---+        ");
    }
    printf("\n");
}

/**
 * @brief prints a character stored in a linked list node.
 * 
 * @param[in] n A pointer to a character
 */
void print_as_char(void *n)
{
   printf("| %c |", *(char *)(n));
}

/**
 * @brief prints a string stored in a linked list node.
 * 
 * @param[in] n A pointer to a character
 */
void print_as_string(void *n)
{
    char *str = (char *)n;
    printf("| %s |", str);
}
