#pragma once
#include <stdint.h>

#define TS_QUEUE_CAPACITY 128

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

// 线程安全队列定义
typedef struct {
    uint16_t regs[10];   // 示例：10个寄存器
    int source;          // 0=RTU, 1=TCP
    uint64_t seq;        // 每个来源独立递增序号
    uint64_t sample_mono_ms; // 采样时刻（单调时钟）
} data_packet_t;

// 环形缓冲区实现的线程安全队列
typedef struct {
    data_packet_t buffer[TS_QUEUE_CAPACITY];
    int head;
    int tail;
    int count;
    uint64_t dropped_count;

#ifdef _WIN32
    CRITICAL_SECTION lock;
    HANDLE sem;
#else
    pthread_mutex_t lock;
    pthread_cond_t cond;
#endif

} ts_queue_t;

void ts_queue_init(ts_queue_t *q);
int  ts_queue_push(ts_queue_t *q, const data_packet_t *pkt);
int  ts_queue_pop(ts_queue_t *q, data_packet_t *out);
uint64_t ts_queue_get_drop_count(ts_queue_t *q);