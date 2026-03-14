#include "common.h"
#include <stdlib.h>
#include <string.h>

#include <time.h>

/**
 * sleep_us函数用于在RTU服务器的主循环中控制寄存器值更新的频率。它接受一个整数参数us，表示需要睡眠的微秒数。
 * 如果us小于或等于0，函数直接返回，不进行睡眠。否则，根据操作系统的不同，函数会使用不同的方法来实现微秒级的睡眠：
 * 在Windows系统上，函数会将微秒转换为毫秒（因为Windows的Sleep函数以毫秒为单位），如果转换后的毫秒数小于或等于0，
 * 则设置为1毫秒，以确保至少睡眠1毫秒。然后调用Sleep函数进行睡眠。
 * 在POSIX系统上，函数会创建一个timespec结构体，设置其中的秒数为us除以1000000，纳秒数为us模1000000乘以1000。
 * 然后调用nanosleep函数进行睡眠。
 * 这个函数的设计允许RTU服务器在每次循环中更新寄存器值后，按照指定的频率进行睡眠，从而控制数据更新的速率，
 * 避免过快地更新寄存器值导致通信问题或资源浪费。
 * parse_us_arg函数用于解析命令行参数，获取指定的微秒级时间间隔。它接受命令行参数的数量argc、
 * 参数数组argv、要解析的参数索引index、默认值default_us以及参数名称name。
 * 函数首先检查argc是否小于或等于index，如果是，则返回默认
 */
static void sleep_us(int us) {
    if (us <= 0) {
        return;
    }

#ifdef _WIN32
    int ms = us / 1000;
    if (ms <= 0) {
        ms = 1;
    }
    Sleep(ms);
#else
    struct timespec ts = { us / 1000000, (us % 1000000) * 1000 };
    nanosleep(&ts, NULL);
#endif
}

/**
 * parse_us_arg函数用于解析命令行参数，获取指定的微秒级时间间隔。它接受命令行参数的数量argc、
 * 参数数组argv、要解析的参数索引index、默认值default_us以及参数名称name。函数首先检查argc是否小于或等于index，
 * 如果是，则返回默认值default_us。否则，函数尝试将argv[index]转换为长整数，并检查转换是否成功以及数值是否在合理范围内（0到10000000微秒）。
 * 如果转换失败或数值不合法，函数会打印一条错误消息，并返回默认值default_us。否则，函数返回解析得到的微秒数。
 */
static int parse_us_arg(int argc, char **argv, int index, int default_us, const char *name) {
    if (argc <= index) {
        return default_us;
    }

    char *end = NULL;
    long v = strtol(argv[index], &end, 10);
    if (end == argv[index] || *end != '\0' || v < 0 || v > 10000000) {
        printf("Invalid %s '%s', fallback to %d us\n", name, argv[index], default_us);
        return default_us;
    }

    return (int)v;
}

int main(int argc, char **argv) {
    int write_interval_us = parse_us_arg(argc, argv, 1, 1000, "write_interval_us");
    int indication_timeout_us = parse_us_arg(argc, argv, 2, 1000, "indication_timeout_us");

    modbus_t *ctx = modbus_new_rtu(SERIAL_PORT_SERVER, 9600, 'N', 8, 1);   // 创建一个RTU服务器上下文（容器），使用指定的串口参数
    uint16_t tab_reg[10] = {0}; // 定义一个数组来存储保持寄存器的值，这里我们创建了一个包含10个寄存器的数组，并初始化为0

    // 用随机数设定保持寄存器中的测试值
    srand(time(NULL));
    for (int i = 0; i < 10; i++) {
        tab_reg[i] = rand() % 65536; // 将保持寄存器的值设置为0到65535之间的随机数
    }

    modbus_mapping_t *mb_mapping =
        modbus_mapping_new(0, 0, 10, 0);    // 创建一个Modbus映射，指定寄存器的数量和类型。在这个例子中，我们创建了一个包含10个保持寄存器的映射

    if (!ctx || !mb_mapping) return -1;

    memcpy(mb_mapping->tab_registers, tab_reg, sizeof(tab_reg));

    modbus_set_slave(ctx, SLAVE_ID);    // 设置从站ID

    // 连接到RTU服务器，在 RTU 模式下，modbus_connect() 函数会打开串口并准备好进行通信。如果连接失败，函数将返回 -1
    if (modbus_connect(ctx) == -1) {
        printf("RTU server connect failed\n");
        return -1;
    }

    modbus_set_debug(ctx, TRUE);  // 启用调试模式，这样我们就可以在控制台上看到Modbus通信的详细信息，方便我们调试和验证服务器的行为

    int sec = indication_timeout_us / 1000000;
    int usec = indication_timeout_us % 1000000;
    modbus_set_indication_timeout(ctx, sec, usec);
    printf("RTU server write_interval_us=%d, indication_timeout_us=%d\n", write_interval_us, indication_timeout_us);

    uint16_t base = 0;

    while (1) {
        for (int i = 0; i < 10; i++) {
            mb_mapping->tab_registers[i] = (uint16_t)(base + i);
        }
        base++;

        uint8_t query[MODBUS_RTU_MAX_ADU_LENGTH];
        int rc = modbus_receive(ctx, query);

        if (rc > 0) {
            modbus_reply(ctx, query, rc, mb_mapping);
        } else if (rc == -1 && errno != ETIMEDOUT) {   // ETIMEDOUT时继续刷新寄存器
            printf("RTU server receive failed\n");
            break;
        }

        sleep_us(write_interval_us);
    }

    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}