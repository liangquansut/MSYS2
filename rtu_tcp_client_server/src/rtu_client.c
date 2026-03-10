#include "common.h"

int main() {
    modbus_t *ctx = modbus_new_rtu(SERIAL_PORT, 9600, 'N', 8, 1);
    if (!ctx) return -1;

    modbus_set_slave(ctx, SLAVE_ID);

    if (modbus_connect(ctx) == -1) {
        printf("RTU client connect failed\n");
        modbus_free(ctx);
        return -1;
    }

    uint16_t reg;
    if (modbus_read_registers(ctx, 0, 1, &reg) == 1)
        printf("RTU client read: %d\n", reg);

    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}