// src/main.c
#include "polling.h"
#include "thread_safe_queue.h"
#include <stdlib.h>

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

    poll_config_t rtu_cfg = { .running = 1, .interval_ms = poll_interval_ms };
    poll_config_t proc_cfg = { .running = 1, .interval_ms = 1000 }; // 统计窗口(ms)
    
    thread_t rtu_thread;
    thread_t proc_thread;

    if (start_thread(&rtu_thread, rtu_poll_thread, &rtu_cfg) != 0) {
        printf("Failed to start RTU thread\n");
        return -1;
    }
    
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