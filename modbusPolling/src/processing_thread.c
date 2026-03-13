#include "polling.h"
#include "thread_safe_queue.h"
#include <time.h>

static uint64_t now_mono_ms(void) {
#ifdef _WIN32
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
#endif
}

static uint64_t now_real_ms(void) {
#ifdef _WIN32
    FILETIME ft;
    ULARGE_INTEGER uli;
    GetSystemTimePreciseAsFileTime(&ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (uint64_t)((uli.QuadPart - 116444736000000000ULL) / 10000ULL);
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
#endif
}

extern ts_queue_t g_queue;

thread_ret_t THREAD_CALL processing_thread(void *arg) {
    poll_config_t *cfg = (poll_config_t*)arg;
    FILE *csv = fopen("poll_data.csv", "a+");
    if (!csv) {
        printf("[PROCESS] open poll_data.csv failed\n");
        return 0;
    }

    fseek(csv, 0, SEEK_END);
    long size = ftell(csv);
    if (size == 0) {
        fprintf(csv, "write_time_ms,aligned_sample_time_ms,source,seq,e2e_latency_ms,queue_drop_total,seq_drop_total,reg0,reg1,reg2,reg3,reg4,reg5,reg6,reg7,reg8,reg9\n");
        fflush(csv);
    }
    setvbuf(csv, NULL, _IOFBF, 1 << 20);

    const uint64_t align_mono_base = now_mono_ms();
    const uint64_t align_real_base = now_real_ms();
    const int stats_interval_ms = (cfg->interval_ms > 0) ? cfg->interval_ms : 1000;

    uint64_t last_stats_ms = now_mono_ms();
    uint64_t rows_this_window = 0;
    uint64_t total_seq_drop = 0;
    uint64_t last_seq[2] = {0, 0};
    int has_last_seq[2] = {0, 0};

    data_packet_t pkt;

    while (cfg->running) {
        ts_queue_pop(&g_queue, &pkt);

        uint64_t now_mono = now_mono_ms();
        uint64_t write_real_ms = now_real_ms();

        uint64_t aligned_sample_ms = align_real_base;
        if (pkt.sample_mono_ms >= align_mono_base) {
            aligned_sample_ms += (pkt.sample_mono_ms - align_mono_base);
        }

        uint64_t e2e_latency_ms = 0;
        if (now_mono >= pkt.sample_mono_ms) {
            e2e_latency_ms = now_mono - pkt.sample_mono_ms;
        }

        int src = (pkt.source == 0) ? 0 : 1;
        if (has_last_seq[src] && pkt.seq > last_seq[src] + 1) {
            total_seq_drop += (pkt.seq - last_seq[src] - 1);
        }
        has_last_seq[src] = 1;
        last_seq[src] = pkt.seq;

        if (pkt.source == 0)
            printf("[PROCESS] From RTU: reg0=%d\n", pkt.regs[0]);
        else
            printf("[PROCESS] From TCP: reg0=%d\n", pkt.regs[0]);

        fprintf(csv,
                "%llu,%llu,%s,%llu,%llu,%llu,%llu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
                (unsigned long long)write_real_ms,
                (unsigned long long)aligned_sample_ms,
                (pkt.source == 0) ? "RTU" : "TCP",
                (unsigned long long)pkt.seq,
                (unsigned long long)e2e_latency_ms,
                (unsigned long long)ts_queue_get_drop_count(&g_queue),
                (unsigned long long)total_seq_drop,
                pkt.regs[0], pkt.regs[1], pkt.regs[2], pkt.regs[3], pkt.regs[4],
                pkt.regs[5], pkt.regs[6], pkt.regs[7], pkt.regs[8], pkt.regs[9]);

        rows_this_window++;

        if (now_mono - last_stats_ms >= (uint64_t)stats_interval_ms) {
            uint64_t elapsed = now_mono - last_stats_ms;
            double rows_per_sec = (elapsed > 0) ? ((double)rows_this_window * 1000.0 / (double)elapsed) : 0.0;
            uint64_t queue_drop_total = ts_queue_get_drop_count(&g_queue);

            printf("[STATS] rows_per_sec=%.2f, queue_drop_total=%llu, seq_drop_total=%llu, e2e_latency_ms=%llu\n",
                   rows_per_sec,
                   (unsigned long long)queue_drop_total,
                   (unsigned long long)total_seq_drop,
                   (unsigned long long)e2e_latency_ms);

            rows_this_window = 0;
            last_stats_ms = now_mono;
            fflush(csv);
        }
    }

    fclose(csv);

    return 0;
}