#include "common.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int main() {
    modbus_t *ctx = modbus_new_rtu(SERIAL_PORT_SERVER, 9600, 'N', 8, 1);    // 创建RTU服务器上下文（容器）
    uint16_t tab_reg[10] = {0}; // 用于存储保持寄存器的数组

    modbus_mapping_t *mb_mapping =
        modbus_mapping_new(0, 0, 10, 0);    // 创建Modbus映射，指定寄存器数量（0个线圈、0个离散输入、10个保持寄存器、0个输入寄存器）

    if (!ctx || !mb_mapping) return -1; // 检查上下文和映射是否创建成功

    modbus_set_slave(ctx, SLAVE_ID);    // 设置从站ID

    // 连接RTU服务器，如果连接失败则输出错误信息并释放资源
    if (modbus_connect(ctx) == -1) {
        printf("RTU server connect failed\n");
        modbus_mapping_free(mb_mapping);
        modbus_free(ctx);
        return -1;
    }

    srand((unsigned int)time(NULL));    // 初始化随机数生成器
    modbus_set_indication_timeout(ctx, 0, 200000);  // 设置Modbus响应超时时间为200毫秒
    printf("RTU server started, writing holding registers...\n");

    while (1) {
        for (int i = 0; i < 10; i++) {
            tab_reg[i] = (uint16_t)(rand() % 10000);
        }
        memcpy(mb_mapping->tab_registers, tab_reg, sizeof(tab_reg));    // 将生成的随机数写入Modbus映射的保持寄存器

        printf("Server HR[0..9]:");
        for (int i = 0; i < 10; i++) {
            printf(" %u", tab_reg[i]);
        }
        printf("\n");

        uint8_t query[MODBUS_RTU_MAX_ADU_LENGTH];   // 用于存储接收到的Modbus请求的缓冲区
        int rc = modbus_receive(ctx, query);    // 接收Modbus请求，如果接收成功则返回请求长度，否则返回-1

        if (rc > 0) {
            modbus_reply(ctx, query, rc, mb_mapping);   // 处理接收到的请求并发送响应，使用Modbus映射中的数据
        } else if (rc == -1 && errno != ETIMEDOUT) {
            printf("modbus_receive failed\n");
        }

#ifdef _WIN32
        Sleep(500);
#else
        usleep(500000);
#endif
    }

    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}