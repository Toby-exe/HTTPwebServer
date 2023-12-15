/**
 * @file pool.h
 * @brief A library for thread pool operations
 * @authors 
 * 
 * Details: 
 * This thread pool is implemented as an array of worker threads and a queue of tasks. 
 * Each worker thread continuously fetches a task from the queue and executes it. 
 * The thread pool provides functions for creating and destroying the pool, 
 * as well as adding tasks to the pool. It also includes experimental support for 
 * dynamic resizing of the thread pool.
 * 
 * Assumptions/Limitations: 
 * It's assumed that the size of the thread pool doesn't change after the pool has been created, 
 * except when using the experimental resize functions. The current implementation does not 
 * provide a way to cancel tasks once they have been added to the pool or a way to pause the pool
 * but not destroy it.
 * 
 * @date 2023-12-06
 */
#ifndef POOL_H
#define POOL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "queue.h"

#define THREAD_POOL_SIZE 16

/* ----------{ STRUCTURES AND TYPES }---------- */

typedef int bool;
#define true 1
#define false 0

typedef void *(*task_func)(void *arg);

typedef struct Task {
    task_func func;
    void *arg;
} Task;

typedef struct Thread {
    pthread_t thread;
    struct ThreadPool *pool;        /* reference to the pool that the thread is in (for access to its condition variables and mutexes) */
    int id;                         /* an id used for testing */
} Thread;

typedef struct ThreadPool {
    Thread **threads;               /* threads of the thread pool */
    volatile int active_threads;
    volatile int working_threads;
    pthread_mutex_t pool_lock;      /* synchronize reading from and writing to the pool (all threads in the pool have access to the same shared thread pool struct) */
    pthread_cond_t task_available;  /* used to signal when the queue has an available task to be processed */
    pthread_cond_t threads_idle;    /* used to signal when there are no threads processing tasks */
    Queue *task_queue;              /* queue of tasks that worker threads will be servicing */
    bool on;                        /* boolean that determines whether the thread pool is active or not */
} ThreadPool;

/* ----------{ FUNCTION PROTOTYPES }---------- */
/* ----------< ThreadPool >---------- */
extern ThreadPool *thread_pool_create(size_t pool_size);
extern void thread_pool_destroy(ThreadPool *pool);
extern void thread_pool_add_task(ThreadPool *pool, task_func function, void* arg);
extern void thread_pool_wait(ThreadPool *pool);
/* dynamic thread pool resizing (EXPERIMENTAL) */
extern void thread_pool_grow(ThreadPool *pool, size_t num);
extern void thread_pool_shrink(ThreadPool *pool, size_t num);

/* ----------< Thread >---------- */
extern Thread *thread_create(ThreadPool *pool, int id);
extern void thread_destroy(Thread *thread);
extern void *thread_do(void *arg);

/* ----------< Tasks >---------- */
extern Queue *task_queue_create();
extern void task_queue_destroy(Queue *queue);

#endif