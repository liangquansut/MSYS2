#pragma once
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

// 线程安全队列定义
typedef struct {
    uint16_t regs[10];   // 示例：10个寄存器
    int source;          // 0=RTU, 1=TCP
} data_packet_t;

// 环形缓冲区实现的线程安全队列
typedef struct {
    data_packet_t buffer[128];
    int head;
    int tail;
    int count;

#ifdef _WIN32
    CRITICAL_SECTION lock;
    HANDLE sem;
#else
    pthread_mutex_t lock;
    pthread_cond_t cond;
#endif

} ts_queue_t;

void ts_queue_init(ts_queue_t *q);
void ts_queue_push(ts_queue_t *q, data_packet_t *pkt);
int  ts_queue_pop(ts_queue_t *q, data_packet_t *out);