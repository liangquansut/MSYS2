// include/polling.h
#pragma once
#include "common.h"

#ifdef _WIN32
    #include <windows.h>
    typedef HANDLE thread_t;        // Windows线程句柄
    typedef DWORD thread_ret_t;     // Windows线程返回值类型
    #define THREAD_CALL __stdcall   // Windows线程函数调用约定
#else
    #include <pthread.h>            // POSIX线程库
    typedef pthread_t thread_t;     // POSIX线程类型
    typedef void* thread_ret_t;     // POSIX线程返回值类型
    #define THREAD_CALL             // 无特殊调用约定
#endif

typedef struct {
    int running;        // 线程运行标志，非0表示继续运行，0表示停止
    int interval_ms;    // 轮询间隔（毫秒），RTU/TCP线程使用，处理线程可选择性使用（如统计窗口）
} poll_config_t;        // 线程配置结构体，包含运行标志和轮询间隔

// RTU / TCP 轮询线程入口
thread_ret_t THREAD_CALL rtu_poll_thread(void *arg);
thread_ret_t THREAD_CALL tcp_poll_thread(void *arg);
// 数据处理线程入口
thread_ret_t THREAD_CALL processing_thread(void *arg);

// 启动/停止线程的简单封装
int start_thread(thread_t *t, thread_ret_t (THREAD_CALL *func)(void*), void *arg);
void stop_flag(poll_config_t *cfg);
void join_thread(thread_t t);