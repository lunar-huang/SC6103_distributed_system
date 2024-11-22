#include <stdint.h>  // Include to define types like uint8_t and uint32_t
#include "server.h"  // Include the server-specific header
#include <stdio.h>   // Standard I/O functions
#include <stdlib.h>  // Standard library functions
#include <pthread.h> // For thread management
#include <unistd.h>  // For UNIX standard functions (like sleep)

#define MAX_THREADS 10  // Define the maximum number of threads in the pool

// Define the structure for a Task, which contains a function and its arguments
typedef struct {
    void (*function)(void *);  // Pointer to the task function
    void *argument;  // Pointer to the task function's argument
} Task;

// Define the structure for the ThreadPool
typedef struct {
    Task *task_queue;  // Queue of tasks to be executed
    int queue_size;    // Current number of tasks in the queue
    int queue_front;   // Front index of the task queue
    int queue_rear;    // Rear index of the task queue
    int queue_capacity;  // Capacity of the task queue
    pthread_t *threads;  // Array of threads in the pool
    int num_threads;   // Number of threads in the pool
    pthread_mutex_t mutex;  // Mutex to protect shared data
    pthread_cond_t cond;  // Condition variable to signal threads
    int stop;  // Flag to indicate if the thread pool should stop
} ThreadPool;

// Static global thread pool instance
static ThreadPool pool;

// Forward declaration of the worker function executed by each thread
void *thread_worker(void *arg);

// Initialize the thread pool with a specified number of threads
void thread_pool_init(int num_threads) {
    if (num_threads > MAX_THREADS) {
        // Limit the number of threads to MAX_THREADS if the input exceeds the limit
        num_threads = MAX_THREADS;
    }

    pool.queue_size = 0;  // Initially, the task queue is empty
    pool.queue_front = 0;  // Front of the task queue is at 0
    pool.queue_rear = 0;   // Rear of the task queue is at 0
    pool.queue_capacity = 100;  // Set default task queue capacity to 100
    pool.stop = 0;  // Initially, the pool is not stopping
    pool.num_threads = num_threads;  // Set the number of threads in the pool

    // Allocate memory for the task queue
    pool.task_queue = (Task *)malloc(pool.queue_capacity * sizeof(Task));
    if (pool.task_queue == NULL) {
        // If memory allocation fails, print an error and exit
        perror("Failed to allocate memory for task queue");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the threads in the pool
    pool.threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    if (pool.threads == NULL) {
        // If memory allocation fails, print an error, free task queue, and exit
        perror("Failed to allocate memory for threads");
        free(pool.task_queue);
        exit(EXIT_FAILURE);
    }

    // Initialize the mutex and condition variable
    pthread_mutex_init(&pool.mutex, NULL);
    pthread_cond_init(&pool.cond, NULL);

    // Create threads in the pool and have them run the thread_worker function
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool.threads[i], NULL, thread_worker, NULL);
    }
}

// Add a task to the thread pool
void thread_pool_add_task(void (*function)(void *), void *arg) {
    pthread_mutex_lock(&pool.mutex);  // Lock the mutex to protect task queue access

    // Check if the task queue is full
    if (pool.queue_size == pool.queue_capacity) {
        printf("Task queue is full, task cannot be added.\n");
        pthread_mutex_unlock(&pool.mutex);  // Unlock the mutex and return
        return;
    }

    // Create a new task and add it to the queue
    Task task;
    task.function = function;  // Set the function pointer for the task
    task.argument = arg;  // Set the function argument
    pool.task_queue[pool.queue_rear] = task;  // Add task at the rear of the queue
    pool.queue_rear = (pool.queue_rear + 1) % pool.queue_capacity;  // Move rear pointer circularly
    pool.queue_size++;  // Increment the queue size

    pthread_cond_signal(&pool.cond);  // Signal the worker threads that a new task is available
    pthread_mutex_unlock(&pool.mutex);  // Unlock the mutex
}

// Destroy the thread pool and clean up resources
void thread_pool_destroy() {
    pthread_mutex_lock(&pool.mutex);  // Lock the mutex

    // Set the stop flag and broadcast the condition to wake up all threads
    pool.stop = 1;
    pthread_cond_broadcast(&pool.cond);
    pthread_mutex_unlock(&pool.mutex);  // Unlock the mutex

    // Join all the threads to ensure they finish execution before cleanup
    for (int i = 0; i < pool.num_threads; i++) {
        pthread_join(pool.threads[i], NULL);
    }

    // Clean up the mutex, condition variable, task queue, and thread array
    pthread_mutex_destroy(&pool.mutex);
    pthread_cond_destroy(&pool.cond);
    free(pool.task_queue);  // Free the memory allocated for the task queue
    pool.task_queue = NULL;
    free(pool.threads);  // Free the memory allocated for the threads
    pool.threads = NULL;
}

// Worker function executed by each thread in the pool
void *thread_worker(void *arg) {
    while (1) {
        pthread_mutex_lock(&pool.mutex);  // Lock the mutex to access the shared task queue

        // Wait for a task to be available or for the stop signal
        while (pool.queue_size == 0 && !pool.stop) {
            pthread_cond_wait(&pool.cond, &pool.mutex);  // Wait for condition signal
        }

        // If the stop flag is set, break out of the loop and exit the thread
        if (pool.stop) {
            pthread_mutex_unlock(&pool.mutex);  // Unlock the mutex before exiting
            break;
        }

        // Get the next task from the front of the queue
        Task task = pool.task_queue[pool.queue_front];
        pool.queue_front = (pool.queue_front + 1) % pool.queue_capacity;  // Move front pointer circularly
        pool.queue_size--;  // Decrement the queue size

        pthread_mutex_unlock(&pool.mutex);  // Unlock the mutex to allow other threads access

        // Execute the task
        (*(task.function))(task.argument);
    }
    return NULL;
}
