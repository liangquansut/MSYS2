#include <stdio.h>
#include <modbus/modbus.h>

int main() {
    modbus_t *ctx = modbus_new_rtu("\\\\.\\COM6", 9600, 'N', 8, 1);
    if (!ctx) {
        printf("Unable to create libmodbus context\n");
        return -1;
    }

    if (modbus_connect(ctx) == -1) {
        printf("Connection failed\n");
        modbus_free(ctx);
        return -1;
    }
    
    // 打开调试模式
    modbus_set_debug(ctx, TRUE);
    // 设置响应超时时间为 1 秒
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    modbus_set_response_timeout(ctx, timeout.tv_sec, timeout.tv_usec);
    
    // 设置从站地址为 1
    modbus_set_slave(ctx, 1);
    uint16_t reg;
    if (modbus_read_registers(ctx, 0x00, 0x0A, &reg) == 1) {
        printf("Register[0] = %d\n", reg);
    }

    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}