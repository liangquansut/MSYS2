// src/rtu_poll.c
#include "polling.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "thread_safe_queue.h"

extern ts_queue_t g_queue;

static void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000 };
    nanosleep(&ts, NULL);
#endif
}

thread_ret_t THREAD_CALL rtu_poll_thread(void *arg) {
    poll_config_t *cfg = (poll_config_t*)arg;

    modbus_t *ctx = modbus_new_rtu(SERIAL_PORT, 9600, 'N', 8, 1);
    if (!ctx) return 0;

    modbus_set_slave(ctx, SLAVE_ID);
    if (modbus_connect(ctx) == -1) {
        printf("RTU connect failed\n");
        modbus_free(ctx);
        return 0;
    }

    uint16_t reg[10];

    while (cfg->running) {
        int rc = modbus_read_registers(ctx, 0, 10, reg);
        if (rc == 10) {
            //printf("[RTU] regs[0]=%d\n", reg[0]);

            // 将数据放入线程安全队列
            data_packet_t pkt;
            memcpy(pkt.regs, reg, sizeof(reg));
            pkt.source = 0; // 0=RTU
            ts_queue_push(&g_queue, &pkt);

        } else {
            //printf("[RTU] read error\n");
        }
        sleep_ms(cfg->interval_ms);
    }

    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}