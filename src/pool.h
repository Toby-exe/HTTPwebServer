#ifndef POOL_H
#define POOL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "queue.h"

#define THREAD_POOL_SIZE 32

/* ----------{ STRUCTURES AND TYPES }---------- */

typedef void* (*task_func)(void *arg);

typedef struct Task {
    task_func func;
    void *arg;
} Task;

// might not need this struct (instead of passing a Thread into pthread_create, pass in the ThreadPool)
typedef struct Thread {
    pthread_t thread;
    struct ThreadPool *pool;        /* reference to the pool that the thread is in (for access to its condition variables and mutexes) */
    int id;                         /* an id used for testing */
} Thread;

typedef struct ThreadPool {
    Thread **threads;               /* Thread *threads[THREAD_POOL_SIZE]; */
    volatile int active_threads;
    volatile int working_threads;
    pthread_mutex_t pool_lock;      /* synchronize reading from and writing to the pool (all threads in the pool have access to the same shared thread pool struct) */
    pthread_cond_t task_available;  /* used to signal when the queue has an available task to be processed */
    pthread_cond_t threads_idle;    /* used to signal when there are no threads processing tasks */
    Queue *task_queue;              /* queue of tasks that worker threads will be servicing */
    int on;                         /* boolean that determines whether the thread pool is active or not */
} ThreadPool;

/* ----------{ FUNCTION PROTOTYPES }---------- */
/* ----------< ThreadPool >---------- */
extern ThreadPool *thread_pool_create(size_t pool_size);
extern void thread_pool_destroy(ThreadPool *pool);
extern void thread_pool_add_task(ThreadPool *pool, task_func function, void* arg);
extern void thread_pool_wait(ThreadPool *pool);
/* dynamic thread pool resizing (EXPERIMENTAL)*/
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
