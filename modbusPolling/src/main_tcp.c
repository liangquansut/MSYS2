// src/main.c
#include "polling.h"

int main() {
    poll_config_t tcp_cfg = { .running = 1, .interval_ms = 1000 };

    thread_t tcp_thread;

    if (start_thread(&tcp_thread, tcp_poll_thread, &tcp_cfg) != 0) {
        printf("Failed to start TCP thread\n");
        stop_flag(&tcp_cfg);
        join_thread(tcp_thread);
        return -1;
    }

    printf("Press ENTER to stop...\n");
    getchar();

    stop_flag(&tcp_cfg);

    join_thread(tcp_thread);
    
    return 0;
}