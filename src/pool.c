#include "pool.h"

/* ----------< ThreadPool >---------- */

/**
 * @brief Creates a new thread pool.
 *
 * @details This function creates a new thread pool with the specified size.
 *          It initializes the thread pool structure, creates the worker threads, and starts them.
 *
 * @param[in] pool_size The size of the thread pool.
 * @return A pointer to the newly created thread pool. If the thread pool
 *         could not be created, it returns NULL.
 */
ThreadPool *thread_pool_create(size_t pool_size)
{

    if (pool_size <= 0)
    {
        pool_size = THREAD_POOL_SIZE;
    }

    ThreadPool *new_pool = (ThreadPool *)calloc(1, sizeof(ThreadPool));
    if (new_pool == NULL)
    {
        perror("POOL CREATION FAILED\n");
        return NULL; // Failed to allocate memory
    }

    new_pool->threads = malloc(pool_size * sizeof(Thread *));
    if (new_pool->threads == NULL)
    {
        perror("THREADS CREATION FAILED\n");
        free(new_pool);
        return NULL; // Failed to allocate memory
    }

    new_pool->active_threads = 0;
    new_pool->working_threads = 0;

    if (pthread_mutex_init(&new_pool->pool_lock, NULL) != 0)
    {
        free(new_pool->threads);
        free(new_pool);
        return NULL; // Failed to initialize the mutex
    }

    if (pthread_cond_init(&new_pool->task_available, NULL) != 0)
    {
        pthread_mutex_destroy(&new_pool->pool_lock);
        free(new_pool->threads);
        free(new_pool);
        return NULL; // Failed to initialize the condition variable
    }

    if (pthread_cond_init(&new_pool->threads_idle, NULL) != 0)
    {
        pthread_cond_destroy(&new_pool->task_available);
        pthread_mutex_destroy(&new_pool->pool_lock);
        free(new_pool->threads);
        free(new_pool);
        return NULL; // Failed to initialize the condition variable
    }

    new_pool->task_queue = task_queue_create();
    if (new_pool->task_queue == NULL)
    {
        perror("QUEUE CREATION FAILED\n");
        pthread_cond_destroy(&new_pool->threads_idle);
        pthread_cond_destroy(&new_pool->task_available);
        pthread_mutex_destroy(&new_pool->pool_lock);
        free(new_pool->threads);
        free(new_pool);
        return NULL; // Failed to create the task queue
    }

    for (size_t i = 0; i < pool_size; i++)
    {
        new_pool->threads[i] = thread_create(new_pool, i);
        if (new_pool->threads[i] == NULL)
        {
            perror("THREAD ERROR\n");
            thread_pool_destroy(new_pool);
            return NULL; // Failed to create a thread
        }
    }

    while (new_pool->active_threads != pool_size) // wait for all threads to initialize
        ;

    return new_pool;
}

void thread_pool_destroy(ThreadPool *pool)
{
    if (pool == NULL)
    {

        return;
    }

    // thread_pool_wait(pool);

    volatile int thread_count = pool->active_threads;

    pthread_mutex_lock(&pool->pool_lock);
    pool->active_threads = 0;
    pthread_cond_broadcast(&pool->task_available);
    pthread_mutex_unlock(&pool->pool_lock);

    // sleep(1);   /* give one second to kill idle threads (if threads are detached upon creation) */

    for (int i = 0; i < thread_count; i++)
    {
        pthread_join(pool->threads[i]->thread, NULL); // ensure all threads terminate before destroying them
        thread_destroy(pool->threads[i]);
    }

    free(pool->threads);

    task_queue_destroy(pool->task_queue);
    pthread_cond_destroy(&pool->threads_idle);
    pthread_cond_destroy(&pool->task_available);
    pthread_mutex_destroy(&pool->pool_lock);

    free(pool);
}

void thread_pool_add_task(ThreadPool *pool, task_func function, void *arg)
{
    Task *task = (Task *)malloc(sizeof(Task));
    task->func = function;
    task->arg = arg;

    pthread_mutex_lock(&pool->pool_lock);
    enqueue(pool->task_queue, task);
    pthread_cond_signal(&pool->task_available);
    pthread_mutex_unlock(&pool->pool_lock);

    printf("\033[1;31m");
    printf("Task added to queue\n");
    printf("\033[0m");
}

void thread_pool_wait(ThreadPool *pool)
{
    pthread_mutex_lock(&pool->pool_lock);
    while (pool->working_threads > 0)
    {
        pthread_cond_wait(&pool->threads_idle, &pool->pool_lock);
    }
    pthread_mutex_unlock(&pool->pool_lock);
}

void thread_pool_grow(ThreadPool *pool, size_t num)
{
    pthread_mutex_lock(&pool->pool_lock);
    pool->threads = realloc(pool->threads, (pool->active_threads + num) * sizeof(Thread *));
    for (size_t i = pool->active_threads; i < pool->active_threads + num; i++)
    {
        pool->threads[i] = thread_create(pool, i);
        if (pool->threads[i] == NULL)
        {
            // Handle error...
            break;
        }
    }
    pool->active_threads += num;
    pthread_mutex_unlock(&pool->pool_lock);
}

void thread_pool_shrink(ThreadPool *pool, size_t num)
{
    pthread_mutex_lock(&pool->pool_lock);
    for (size_t i = 0; i < num; i++)
    {
        if (pool->active_threads == 0)
        {
            break;
        }
        pool->active_threads--;
        pthread_cancel(pool->threads[pool->active_threads]->thread);
        thread_destroy(pool->threads[pool->active_threads]);
    }
    pool->threads = realloc(pool->threads, pool->active_threads * sizeof(Thread *));
    pthread_mutex_unlock(&pool->pool_lock);
}

/* ----------< Thread >---------- */

/**
 * @brief Creates a new worker thread.
 *
 * @param[in] pool A pointer to the thread pool that the thread will belong to.
 * @param[in] id The ID of the thread.
 * @return A pointer to the newly created thread. If the thread could not be created, it returns NULL.
 */
Thread *thread_create(ThreadPool *pool, int id)
{
    Thread *thread = malloc(sizeof(Thread));
    if (thread == NULL)
    {
        return NULL;
    }

    thread->pool = pool;
    thread->id = id;

    if (pthread_create(&thread->thread, NULL, thread_do, thread) != 0)
    {
        perror("THREAD CREATION FAILED\n");
        free(thread);
        return NULL;
    }
    // pthread_detach(thread->thread);   // if detached, remove join in thread_pool_destroy

    return thread;
}

/**
 * @brief Destroys a worker thread.
 *
 * @details This function destroys a worker thread.
 *          It stops the thread, waits for it to finish its current task,
 *          and then frees all the resources associated with the thread.
 *
 * @param thread A pointer to the thread to be destroyed.
 * @return void
 */
void thread_destroy(Thread *thread)
{
    if (thread == NULL)
    {

        return;
    }

    // pthread_cancel(thread->thread);  // if thread_do returns, the thread terminates
    free(thread);
}

/**
 * @brief The function to be executed by all worker threads.
 *
 * @details This function is the main function that the worker threads execute.
 *          It fetches tasks from the work queue and processes them.
 *
 * @param arg A pointer to the argument to be passed to the function.
 *            This is usually a pointer to the thread's own structure.
 */
void *thread_do(void *arg)
{
    Thread *thread = (Thread *)arg;
    ThreadPool *pool = thread->pool;

    pthread_mutex_lock(&pool->pool_lock);
    pool->active_threads++;

    pthread_mutex_unlock(&pool->pool_lock);

    for (;;)
    {
        pthread_mutex_lock(&pool->pool_lock);

        while (pool->task_queue->length == 0 && pool->active_threads > 0)
        {
            pthread_cond_wait(&pool->task_available, &pool->pool_lock);
        }

        if (pool->active_threads == 0)
        {

            pthread_mutex_unlock(&pool->pool_lock);
            break;
        }

        Task *task = dequeue(pool->task_queue);

        printf("\033[1;31m");
        printf("Task dequeued\n");
        printf("\033[0m");
        pool->working_threads++;

        pthread_mutex_unlock(&pool->pool_lock);

        if (task != NULL)
        {
            task->func(task->arg);
            free(task);
        }

        printf("\033[1;31m");
        printf("Task executed\n");
        printf("\033[0m");

        pthread_mutex_lock(&pool->pool_lock);
        pool->working_threads--;
        if (pool->working_threads == 0)
        {

            pthread_cond_signal(&pool->threads_idle);
        }
        pthread_mutex_unlock(&pool->pool_lock);
    }

    // pthread_mutex_lock(&pool->pool_lock);
    // pool->active_threads--;
    // if "on" field is implemented, change stop condition and un comment this code

    // pthread_mutex_unlock(&pool->pool_lock);

    return NULL;
}

/* ----------< Tasks >---------- */

Queue *task_queue_create()
{
    Queue *new_queue = queue_create();
    if (new_queue == NULL)
    {
        free(new_queue);
        return NULL; // Failed to create the queue
    }

    return new_queue;
}

void task_queue_destroy(Queue *queue)
{
    if (queue != NULL)
    {
        queue_destroy(queue);
    }
}
