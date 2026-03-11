// src/main.c
#include "polling.h"

int main() {
    poll_config_t rtu_cfg = { .running = 1, .interval_ms = 500 };

    thread_t rtu_thread;

    if (start_thread(&rtu_thread, rtu_poll_thread, &rtu_cfg) != 0) {
        printf("Failed to start RTU thread\n");
        return -1;
    }

    printf("Press ENTER to stop...\n");
    getchar();

    stop_flag(&rtu_cfg);

    join_thread(rtu_thread);

    return 0;
}