// src/main.c
#include "polling.h"
#include "thread_safe_queue.h"

ts_queue_t g_queue;

int main() {
    ts_queue_init(&g_queue);

    poll_config_t tcp_cfg = { .running = 1, .interval_ms = 1000 };
    poll_config_t proc_cfg = { .running = 1, .interval_ms = 0 }; // 处理线程不需要睡眠

    thread_t tcp_thread;
    thread_t proc_thread;

    if (start_thread(&tcp_thread, tcp_poll_thread, &tcp_cfg) != 0) {
        printf("Failed to start TCP thread\n");
        stop_flag(&tcp_cfg);
        join_thread(tcp_thread);
        return -1;
    }
    if (start_thread(&proc_thread, processing_thread, &proc_cfg) != 0) {
        printf("Failed to start processing thread\n");
        stop_flag(&tcp_cfg);
        join_thread(tcp_thread);
        return -1;
    }

    printf("Press ENTER to stop...\n");
    getchar();

    stop_flag(&tcp_cfg);
    stop_flag(&proc_cfg);

    join_thread(tcp_thread);
    join_thread(proc_thread);

    return 0;
}