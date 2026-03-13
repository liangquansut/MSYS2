#include "common.h"
#include <stdlib.h>
#include <string.h>

// 这是一个简单的Modbus RTU服务器示例，使用libmodbus库实现。服务器会持续更新保持寄存器的值，并响应来自客户端的请求
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

    // 设定保持寄存器中的测试值
    for (int i = 0; i < 10; i++) {
        tab_reg[i] = i + 1; // 将保持寄存器的值设置为1到10
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