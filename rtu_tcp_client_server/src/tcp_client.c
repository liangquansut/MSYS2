#include "common.h"

int main() {
    modbus_t *ctx = modbus_new_tcp("127.0.0.1", 1502);
    if (!ctx) return -1;

    if (modbus_connect(ctx) == -1) {
        printf("TCP client connect failed\n");
        modbus_free(ctx);
        return -1;
    }

    uint16_t reg;
    if (modbus_read_registers(ctx, 0, 1, &reg) == 1)
        printf("TCP client read: %d\n", reg);

    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}