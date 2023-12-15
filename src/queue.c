/**
 * @file queue.c
 * @authors Jayden Mingle
 *
 * @date 2023-11-30
 */
#include "queue.h"

Queue *queue_create()
{
    Queue *new_queue = (Queue *)calloc(1, sizeof(Queue));

    if (new_queue == NULL) { // calloc failed
        return NULL;
    }

    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->length = 0;

    return new_queue;
}

void queue_destroy(Queue *queue)
{
    QueueNode *curr = queue->head, *next = NULL;

    while(curr != NULL)
    {
        next = curr->next;
        free(curr);
        curr = next;
    }

    free(queue);
}

void enqueue(Queue *queue, void *data)
{
    QueueNode *new_node = (QueueNode *)calloc(1, sizeof(QueueNode));

    if(new_node == NULL) {  //calloc failed
        return;
    }

    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = queue->tail;

    if (queue->tail != NULL) {
        queue->tail->next = new_node;
    }

    queue->tail = new_node;

    if (queue->head == NULL) {
        queue->head = new_node;
    }

    queue->length++;
}

void *dequeue(Queue *queue)
{
    if (queue->head == NULL) {  // empty list
        return NULL;
    }

    QueueNode *node = queue->head;
    
    void *data = node->data;

    queue->head = node->next;

    if (queue->head != NULL) {
        queue->head->prev = NULL;
    } else {
        queue->tail = NULL;
    }

    free(node);
    queue->length--;

    return data;
}