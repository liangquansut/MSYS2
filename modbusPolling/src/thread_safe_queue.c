#include "thread_safe_queue.h"
#include <string.h>

void ts_queue_init(ts_queue_t *q) {
    q->head = q->tail = q->count = 0;
    q->dropped_count = 0;

#ifdef _WIN32
    InitializeCriticalSection(&q->lock);
    q->sem = CreateSemaphore(NULL, 0, 128, NULL);
#else
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->cond, NULL);
#endif
}

int ts_queue_push(ts_queue_t *q, const data_packet_t *pkt) {
#ifdef _WIN32
    EnterCriticalSection(&q->lock);
#else
    pthread_mutex_lock(&q->lock);
#endif

    if (q->count >= TS_QUEUE_CAPACITY) {
        q->dropped_count++;
#ifdef _WIN32
        LeaveCriticalSection(&q->lock);
#else
        pthread_mutex_unlock(&q->lock);
#endif
        return 0;
    }

    q->buffer[q->tail] = *pkt;
    q->tail = (q->tail + 1) % TS_QUEUE_CAPACITY;
    q->count++;

#ifdef _WIN32
    LeaveCriticalSection(&q->lock);
    ReleaseSemaphore(q->sem, 1, NULL);
#else
    pthread_mutex_unlock(&q->lock);
    pthread_cond_signal(&q->cond);
#endif

    return 1;
}

int ts_queue_pop(ts_queue_t *q, data_packet_t *out) {
#ifdef _WIN32
    WaitForSingleObject(q->sem, INFINITE);
    EnterCriticalSection(&q->lock);
#else
    pthread_mutex_lock(&q->lock);
    while (q->count == 0)
        pthread_cond_wait(&q->cond, &q->lock);
#endif

    *out = q->buffer[q->head];
    q->head = (q->head + 1) % TS_QUEUE_CAPACITY;
    q->count--;

#ifdef _WIN32
    LeaveCriticalSection(&q->lock);
#else
    pthread_mutex_unlock(&q->lock);
#endif

    return 1;
}

uint64_t ts_queue_get_drop_count(ts_queue_t *q) {
    uint64_t dropped;

#ifdef _WIN32
    EnterCriticalSection(&q->lock);
    dropped = q->dropped_count;
    LeaveCriticalSection(&q->lock);
#else
    pthread_mutex_lock(&q->lock);
    dropped = q->dropped_count;
    pthread_mutex_unlock(&q->lock);
#endif

    return dropped;
}