#include "polling.h"
#include "thread_safe_queue.h"
#include <time.h>

/**
 * 获取当前单调时钟的毫秒数，用于计算采样时间和端到端延迟
 * 单调时钟不会受到系统时间调整的影响，适合用于测量时间间隔和计算延迟
 * 在Windows上，使用GetTickCount64()获取系统启动后的毫秒数；
 * 在POSIX系统上，使用clock_gettime()获取单调时钟的时间，并转换为毫秒数返回
 * 注意：单调时钟的基准点是系统启动时间，因此返回值不具有实际的日期时间意义，但适合用于计算时间差和延迟
 * 获取当前实时时钟的毫秒数，用于记录数据写入CSV文件的时间戳
 * 实时时钟表示当前的日期和时间，可能会受到系统时间调整的影响，但适合用于记录事件发生的实际时间
 * 在Windows上，使用GetSystemTimePreciseAsFileTime()获取当前系统时间的FILETIME结构，
 * 并转换为自1970年1月1日以来的毫秒数；在POSIX系统上，使用clock_gettime()获取实时时钟的时间，
 * 并转换为毫秒数返回
 * 注意：实时时钟的基准点是1970年1月1日，因此返回值表示自该时间以来的毫秒数，适合用于记录事件发生的实际时间戳
 * 处理线程的主循环中，持续从线程安全队列中弹出数据包，并计算相关的统计信息，如端到端延迟、丢包率等。
 * 每当处理一个数据包时，都会将相关信息写入CSV文件，并定期输出统计信息到控制台。处理线程会一直运行，
 * 直到收到停止信号（running标志被设置为0）。
 * 处理线程的统计信息包括：
 * - rows_per_sec：每秒处理的数据包数量，计算方法是统计窗口内处理的数据包数量除以窗口持续的秒数
 * - queue_drop_total：线程安全队列中由于满而丢弃的数据包总数，通过调用ts_queue_get_drop_count()获取
 * - seq_drop_total：根据数据包的序列号计算的丢包总数，如果当前数据包的序列号比上一个数据包的序列号大超过1，
 * 则认为中间有丢包，丢包数量为两者之差减1
 * e2e_latency_ms：端到端延迟，计算方法是当前单调时钟时间减去数据包采样时的单调时钟时间
 * 处理线程还会将每个数据包的信息写入CSV文件，
 * 包括写入时间、对齐后的采样时间、数据来源、序列号、端到端延迟、丢包统计信息以及寄存器的值等。
 * 这些信息可以用于后续的数据分析和性能评估。
 * 处理线程的设计考虑了性能和统计需求，使用单调时钟来计算延迟和对齐采样时间，
 * 使用实时时钟来记录事件发生的实际时间，并定期输出统计信息以监控系统的运行状态和性能表现。
 */
static uint64_t now_mono_ms(void) {
#ifdef _WIN32
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
#endif
}

/**
 * 获取当前实时时钟的毫秒数，用于记录数据写入CSV文件的时间戳
 * 实时时钟表示当前的日期和时间，可能会受到系统时间调整的影响，但适合用于记录事件发生的实际时间
 * 在Windows上，使用GetSystemTimePreciseAsFileTime()获取当前系统时间的FILETIME结构，
 * 并转换为自1970年1月1日以来的毫秒数；在POSIX系统上，使用clock_gettime()获取实时时钟的时间，
 * 并转换为毫秒数返回
 * 注意：实时时钟的基准点是1970年1月1日，因此返回值表示自该时间以来的毫秒数，适合用于记录事件发生的实际时间戳
 * 处理线程的主循环中，持续从线程安全队列中弹出数据包，并计算相关的统计信息，如端到端延迟、丢包率等。
 * 每当处理一个数据包时，都会将相关信息写入CSV文件，并定期输出统计信息到控制台。处理线程会一直运行，
 * 直到收到停止信号（running标志被设置为0）。
 * 处理线程的统计信息包括：
 * - rows_per_sec：每秒处理的数据包数量，计算方法是统计窗口内处理的数据包数量除以窗口持续的秒数
 * - queue_drop_total：线程安全队列中由于满而丢弃的数据包总数，通过调用ts_queue_get_drop_count()获取
 * - seq_drop_total：根据数据包的序列号计算的丢包总数，如果当前数据包的序列号比上一个数据包的序列号大超过1，
 * 则认为中间有丢包，丢包数量为两者之差减1
 * e2e_latency_ms：端到端延迟，计算方法是当前单调时钟时间减去数据包采样时的单调时钟时间
 * 处理线程还会将每个数据包的信息写入CSV文件，
 * 包括写入时间、对齐后的采样时间、数据来源、序列号、端到端延迟、丢包统计信息以及寄存器的值等。
 * 这些信息可以用于后续的数据分析和性能评估。
 * 处理线程的设计考虑了性能和统计需求，使用单调时钟来计算延迟和对齐采样时间，
 * 使用实时时钟来记录事件发生的实际时间，并定期输出统计信息以监控系统的运行状态和性能表现。
 */
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

/**
 * 处理线程的主循环中，持续从线程安全队列中弹出数据包，并计算相关的统计信息，如端到端延迟、丢包率等。
 * 每当处理一个数据包时，都会将相关信息写入CSV文件，并定期输出统计信息到控制台。处理线程会一直运行，
 * 直到收到停止信号（running标志被设置为0）。
 * 处理线程的统计信息包括：
 * - rows_per_sec：每秒处理的数据包数量，计算方法是统计窗口内处理的数据包数量除以窗口持续的秒数
 * - queue_drop_total：线程安全队列中由于满而丢弃的数据包总数，通过调用ts_queue_get_drop_count()获取
 * - seq_drop_total：根据数据包的序列号计算的丢包总数，如果当前数据包的序列号比上一个数据包的序列号大超过1，
 * 则认为中间有丢包，丢包数量为两者之差减1
 * e2e_latency_ms：端到端延迟，计算方法是当前单调时钟时间减去数据包采样时的单调时钟时间
 * 处理线程还会将每个数据包的信息写入CSV文件，
 * 包括写入时间、对齐后的采样时间、数据来源、序列号、端到端延迟、丢包统计信息以及寄存器的值等。
 * 这些信息可以用于后续的数据分析和性能评估。
 * 处理线程的设计考虑了性能和统计需求，使用单调时钟来计算延迟和对齐采样时间，
 * 使用实时时钟来记录事件发生的实际时间，并定期输出统计信息以监控系统的运行状态和性能表现。
 */
thread_ret_t THREAD_CALL processing_thread(void *arg) {
    poll_config_t *cfg = (poll_config_t*)arg;   // 处理线程的配置参数，包含运行标志和统计窗口的轮询间隔
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
    setvbuf(csv, NULL, _IOFBF, 1 << 20);    // 1MB缓冲区，减少磁盘IO次数

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
                (unsigned long long)write_real_ms,  // 数据写入CSV文件的时间戳，使用实时时钟表示
                (unsigned long long)aligned_sample_ms,  // 对齐后的采样时间，基于单调时钟计算并对齐到处理线程的统计窗口
                (pkt.source == 0) ? "RTU" : "TCP",  // 数据来源，0表示RTU，1表示TCP
                (unsigned long long)pkt.seq,    // 数据包的序列号，每个来源独立递增
                (unsigned long long)e2e_latency_ms, // 端到端延迟，当前单调时钟时间减去数据包采样时的单调时钟时间
                (unsigned long long)ts_queue_get_drop_count(&g_queue), // 线程安全队列中由于满而丢弃的数据包总数
                (unsigned long long)total_seq_drop, // 根据序列号计算的丢包总数
                pkt.regs[0], pkt.regs[1], pkt.regs[2], pkt.regs[3], pkt.regs[4],
                pkt.regs[5], pkt.regs[6], pkt.regs[7], pkt.regs[8], pkt.regs[9]); // 数据包中寄存器的值

        rows_this_window++;

        if (now_mono - last_stats_ms >= (uint64_t)stats_interval_ms) {
            uint64_t elapsed = now_mono - last_stats_ms;
            double rows_per_sec = (elapsed > 0) ? ((double)rows_this_window * 1000.0 / (double)elapsed) : 0.0;
            uint64_t queue_drop_total = ts_queue_get_drop_count(&g_queue);

            printf("[STATS] rows_per_sec=%.2f, queue_drop_total=%llu, seq_drop_total=%llu, e2e_latency_ms=%llu\n",
                   rows_per_sec,    // 每秒处理的数据包数量，计算方法是统计窗口内处理的数据包数量除以窗口持续的秒数
                   (unsigned long long)queue_drop_total, // 线程安全队列中由于满而丢弃的数据包总数，通过调用ts_queue_get_drop_count()获取
                   (unsigned long long)total_seq_drop, // 根据数据包的序列号计算的丢包总数，如果当前数据包的序列号比上一个数据包的序列号大超过1，则认为中间有丢包，丢包数量为两者之差减1
                   (unsigned long long)e2e_latency_ms); // 端到端延迟，计算方法是当前单调时钟时间减去数据包采样时的单调时钟时间

            rows_this_window = 0;
            last_stats_ms = now_mono;
            fflush(csv);
        }
    }

    fclose(csv);

    return 0;
}