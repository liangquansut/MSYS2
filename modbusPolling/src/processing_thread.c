#include "polling.h"
#include "thread_safe_queue.h"

extern ts_queue_t g_queue;

thread_ret_t THREAD_CALL processing_thread(void *arg) {
    poll_config_t *cfg = (poll_config_t*)arg;

    data_packet_t pkt;

    while (cfg->running) {
        ts_queue_pop(&g_queue, &pkt);

        if (pkt.source == 0)
            printf("[PROCESS] From RTU: reg0=%d\n", pkt.regs[0]);
        else
            printf("[PROCESS] From TCP: reg0=%d\n", pkt.regs[0]);

        // 这里你可以：
        // - 写数据库
        // - 写 CSV
        // - 推送 MQTT
        // - 做滤波/统计
        // - 触发控制逻辑
    }

    return 0;
}