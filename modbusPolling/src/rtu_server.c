#include "common.h"

int main() {
    modbus_t *ctx = modbus_new_rtu(SERIAL_PORT, 9600, 'N', 8, 1);   // 创建一个RTU服务器上下文（容器），使用指定的串口参数
    uint16_t tab_reg[10] = {0}; // 定义一个数组来存储保持寄存器的值，这里我们创建了一个包含10个寄存器的数组，并初始化为0

    // 设定保持寄存器中的测试值
    for (int i = 0; i < 10; i++) {
        tab_reg[i] = i + 1; // 将保持寄存器的值设置为1到10
    }

    modbus_mapping_t *mb_mapping =
        modbus_mapping_new(0, 0, 10, 0);    // 创建一个Modbus映射，指定寄存器的数量和类型。在这个例子中，我们创建了一个包含10个保持寄存器的映射

    if (!ctx || !mb_mapping) return -1;

    modbus_set_slave(ctx, SLAVE_ID);    // 设置从站ID

    // 连接到RTU服务器，在 RTU 模式下，modbus_connect() 函数会打开串口并准备好进行通信。如果连接失败，函数将返回 -1
    if (modbus_connect(ctx) == -1) {
        printf("RTU server connect failed\n");
        return -1;
    }

    modbus_set_debug(ctx, TRUE);  // 启用调试模式，这样我们就可以在控制台上看到Modbus通信的详细信息，方便我们调试和验证服务器的行为

    while (1) {
        uint8_t query[MODBUS_RTU_MAX_ADU_LENGTH];
        int rc = modbus_receive(ctx, query);
        
        if (rc > 0)
            modbus_reply(ctx, query, rc, mb_mapping);
        else if (rc == -1) {   // 如果接收数据失败，modbus_receive() 函数将返回 -1，此时我们可以检查错误并决定是否继续监听
            printf("RTU server receive failed\n");
            break;
        }
    }

    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}