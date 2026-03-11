// include/polling.h
#pragma once
#include "common.h"

#ifdef _WIN32
    #include <windows.h>
    typedef HANDLE thread_t;
    typedef DWORD thread_ret_t;
    #define THREAD_CALL __stdcall
#else
    #include <pthread.h>
    typedef pthread_t thread_t;
    typedef void* thread_ret_t;
    #define THREAD_CALL
#endif

typedef struct {
    int running;
    int interval_ms;
} poll_config_t;

// RTU / TCP 轮询线程入口
thread_ret_t THREAD_CALL rtu_poll_thread(void *arg);
thread_ret_t THREAD_CALL tcp_poll_thread(void *arg);
// 数据处理线程入口
thread_ret_t THREAD_CALL processing_thread(void *arg);

// 启动/停止线程的简单封装
int start_thread(thread_t *t, thread_ret_t (THREAD_CALL *func)(void*), void *arg);
void stop_flag(poll_config_t *cfg);
void join_thread(thread_t t);