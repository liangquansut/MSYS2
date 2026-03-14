// src/main.c
#include "polling.h"
#include "thread_safe_queue.h"
#include <stdlib.h>

/**
 * 获取当前单调时钟的毫秒数，用于计算采样时间和端到端延迟
 * 单调时钟不会受到系统时间调整的影响，适合用于测量时间间隔和计算延迟
 * 在Windows上，使用GetTickCount64()获取系统启动后的毫秒数；
 * 在POSIX系统上，使用clock_gettime()获取单调时钟的时间，并转换为毫秒数返回
 * 注意：单调时钟的基准点是系统启动时间，因此返回值不
 * 具有实际的日期时间意义，但适合用于计算时间差和延迟
 * 获取当前实时时钟的毫秒数，用于记录数据写入CSV文件的时间戳
 * 实时时钟表示当前的日期和时间，可能会受到系统时间调整的影响，但适合用于记录事件发生的实际时间
 * 在Windows上，使用GetSystemTimePreciseAsFileTime()获取当前系统时间的FILETIME结构，
 * 并转换为自1970年1月1日以来的毫秒数；在POSIX系统上，使用clock_gettime()获取实时时钟的时间，
 * 并转换为毫秒数返回
 */
static int parse_interval_ms(int argc, char **argv, int default_ms) {
    if (argc < 2) {
        return default_ms;
    }

    char *end = NULL;
    long v = strtol(argv[1], &end, 10);
    if (end == argv[1] || *end != '\0' || v < 0 || v > 60000) {
        printf("Invalid poll interval '%s', fallback to %d ms\n", argv[1], default_ms);
        return default_ms;
    }

    return (int)v;
}

ts_queue_t g_queue;

int main(int argc, char **argv) {
    ts_queue_init(&g_queue);

    int poll_interval_ms = parse_interval_ms(argc, argv, 1);

    poll_config_t rtu_cfg = { .running = 1, .interval_ms = poll_interval_ms };  // RTU线程配置，包含运行标志和轮询间隔
    poll_config_t proc_cfg = { .running = 1, .interval_ms = 1000 }; // 统计窗口(ms)
    
    thread_t rtu_thread;
    thread_t proc_thread;

    // 启动RTU轮询线程和处理线程，并传入相应的配置参数
    if (start_thread(&rtu_thread, rtu_poll_thread, &rtu_cfg) != 0) {
        printf("Failed to start RTU thread\n");
        return -1;
    }
    
    // 启动处理线程，并传入处理线程的配置参数（包含运行标志和统计窗口的轮询间隔）
    if (start_thread(&proc_thread, processing_thread, &proc_cfg) != 0) {
        printf("Failed to start processing thread\n");
        stop_flag(&rtu_cfg);
        join_thread(rtu_thread);
        return -1;
    }

    printf("RTU poll interval: %d ms\n", poll_interval_ms);
    printf("Press ENTER to stop...\n");
    getchar();

    stop_flag(&rtu_cfg);
    stop_flag(&proc_cfg);

    join_thread(rtu_thread);
    join_thread(proc_thread);

    return 0;
}