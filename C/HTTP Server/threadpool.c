#include "threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define TRUE 1

// This method craetes a thread pool
threadpool* create_threadpool(int num_threads_in_pool) {
    if(num_threads_in_pool <= 0 || num_threads_in_pool > MAXT_IN_POOL)
        return NULL;

    threadpool *pool = (threadpool*)malloc(sizeof(threadpool));
    if(pool == NULL) {
        perror("malloc failed");
        exit(1);
    }

    pool->num_threads = num_threads_in_pool;
    pool->qsize = 0;
    pool->shutdown = 0;
    pool->dont_accept = 0;
    pool->qhead = NULL;
    pool->qtail = NULL;

    pthread_mutex_init(&(pool->qlock), NULL);
    pthread_cond_init(&(pool->q_empty), NULL);
    pthread_cond_init(&(pool->q_not_empty), NULL);

    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * pool->num_threads);
    for(int i = 0; i < num_threads_in_pool; i++) {
        if(pthread_create(&(pool->threads[i]), NULL, do_work, pool) < 0) {
            perror("pthread_create() failed");
            exit(1);
        }
    }
    return pool;
}

// This method recieves a new work, adding it to the pool's Q, signaling there is a new work.
void dispatch(threadpool* from_me, dispatch_fn dispatch_to_here, void *arg) {
    if(from_me == NULL) return;

    pthread_mutex_lock(&(from_me->qlock));
    if(from_me->dont_accept == 1) {
        pthread_mutex_unlock(&(from_me->qlock));
        return;
    }

    // creating a new work
    work_t *work = (work_t*)malloc(sizeof(work_t));
    if(work == NULL) {
        pthread_mutex_unlock(&(from_me->qlock));
        perror("malloc() failed");
        exit(1);
    }
    work->routine = dispatch_to_here;
    work->arg = arg;
    work->next = NULL;

    // enqueue
    if(from_me->qsize == 0) {
        from_me->qhead = work;
    }
    else {
        from_me->qtail->next = work;
    }
    from_me->qtail = work;
    from_me->qsize++;

    // signal the pool there is a new work to do
    pthread_cond_signal(&(from_me->q_not_empty));
    pthread_mutex_unlock(&(from_me->qlock));
}

// This method is the life cycle of all therads in the pool, waiting for work to be sent
// if a work is inserted a thread executes the function listed in the work.
void* do_work(void* p) {
    threadpool *pool = (threadpool*)p;

    while(TRUE) {
        pthread_mutex_lock(&(pool->qlock));
        if(pool->shutdown == 1) {
            pthread_mutex_unlock(&(pool->qlock));
            return NULL;
        }
        if(pool->qsize == 0) {
            pthread_cond_wait(&(pool->q_not_empty), &(pool->qlock));
        }
        
        if(pool->shutdown == 1) {
            pthread_mutex_unlock(&(pool->qlock));
            return NULL;
        }

        // dequeue
        work_t *work = pool->qhead;
        if(work == NULL) {
            pthread_mutex_unlock(&(pool->qlock));
            continue;
        }
        pool->qhead = pool->qhead->next;
        pool->qsize--;
        if(pool->dont_accept == 1 && pool->qsize == 0) {
            pthread_cond_signal(&(pool->q_empty));
        }
        pthread_mutex_unlock(&(pool->qlock));

        (*(work->routine))(work->arg);

        pthread_mutex_lock(&(pool->qlock));
        free(work);
        pthread_mutex_unlock(&(pool->qlock));
    }
}

// This method destroys a thread pool
void destroy_threadpool(threadpool* destroyme) {
    pthread_mutex_lock(&(destroyme->qlock));
    destroyme->dont_accept = 1;
    if(destroyme->qsize > 0) {
        pthread_cond_wait(&(destroyme->q_empty), &(destroyme->qlock));
    }
    destroyme->shutdown = 1;
    
    // "wake" all remaining non working threads so they terminate because shutdown = 1
    pthread_cond_broadcast(&(destroyme->q_not_empty)); 
    pthread_mutex_unlock(&(destroyme->qlock));

    for(int i = 0; i < destroyme->num_threads; i++) {
        pthread_join(destroyme->threads[i], NULL);
    }

    free(destroyme->threads);
    pthread_cond_destroy(&(destroyme->q_empty));
    pthread_cond_destroy(&(destroyme->q_not_empty));
    pthread_mutex_destroy(&(destroyme->qlock));
    free(destroyme);
}