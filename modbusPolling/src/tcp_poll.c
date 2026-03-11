// src/tcp_poll.c
#include "polling.h"
#include <time.h>

static void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000 };
    nanosleep(&ts, NULL);
#endif
}

thread_ret_t THREAD_CALL tcp_poll_thread(void *arg) {
    poll_config_t *cfg = (poll_config_t*)arg;

    modbus_t *ctx = modbus_new_tcp("127.0.0.1", 1502);
    if (!ctx) return 0;

    if (modbus_connect(ctx) == -1) {
        printf("TCP connect failed\n");
        modbus_free(ctx);
        return 0;
    }

    uint16_t reg[10];

    while (cfg->running) {
        int rc = modbus_read_registers(ctx, 0, 10, reg);
        if (rc == 10) {
            printf("[TCP] regs[0]=%d\n", reg[0]);
        } else {
            printf("[TCP] read error\n");
        }
        sleep_ms(cfg->interval_ms);
    }

    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}