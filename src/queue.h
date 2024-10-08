/**
 * @file queue.h
 * @brief A library for operations on a queue.
 * @authors 
 * 
 * Details: 
 * This queue is implemented as a doubly-linked list. Each node in the queue contains a pointer to the data it holds    
 * and pointers to the next and previous nodes in the queue. The queue itself keeps track of its length and has pointers  
 * to the head and tail nodes. The queue can grow and shrink dynamically as nodes are added and removed.
 * 
 * Assumptions/Limitations: 
 * This implementation assumes that the data stored in each node has been
 * already been properly allocated and that the caller is responsible for managing this memory.
 * 
 * @date 2023-12-04
 */
#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* =========={ METHOD 2: DOUBLY-LINKED LIST }========== */
// pros: all operations are O(1) and the queue can grow and shrink dynamically
// cons: uses more memory than the other two methods

/* ----------{ STRUCTURES AND TYPES }---------- */

typedef struct qnode {
    void *data;
    struct qnode *next;
    struct qnode *prev;
} QueueNode;

typedef struct queue {
    int length;
    QueueNode *head;
    QueueNode *tail;
} Queue;

/* ----------{ FUNCTION PROTOTYPES }---------- */

extern Queue *queue_create();
extern void queue_destroy(Queue *queue);
extern void enqueue(Queue *queue, void *data);
extern void *dequeue(Queue *queue);

/* =============== ALTERNATIVE IMPLEMENTATION =============== */

/* =========={ METHOD 1: BOUNDED BUFFER (used in prodcons.c example) }========== */
// pros: all operations are O(1) and is memory efficient
// cons: size is fixed from creation

// #define BUFFER_SIZE 256  // size of bounded buffer queue should be the same size as the backlog + thread pool size

/* ----------{ STRUCTURES AND TYPES }---------- */

// typedef struct Queue {
//     volatile void *buffer[BUFFER_SIZE];          /* the bounded buffer */
//     volatile int head;                           /* index of next enqueue */
//     volatile int tail;                           /* index of next dequeue */
//     volatile int fill_level;                     /* fill level of buffer */
// } Queue;

/* =========={ METHOD 3: SINGLY-LINKED LIST }========== */
// pros: uses less memory than a doubly linked list while still allowing for variable length
// cons: enqueue becomes O(n)

// #include "linkedlist.h"

/* ----------{ STRUCTURES AND TYPES }---------- */

// typedef LinkedList Queue;

/* ----------{ FUNCTION PROTOTYPES }---------- */

// extern Queue *queue_create();
// extern void queue_destroy(Queue *queue);
// extern void *queue_enqueue(Queue *queue, void *data);
// extern void *queue_dequeue(Queue *queue);
// extern int queue_count(Queue *queue);

#endif