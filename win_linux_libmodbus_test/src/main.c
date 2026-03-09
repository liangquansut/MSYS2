#include <stdio.h>
#include <modbus/modbus.h>

int main() {

#ifdef _WIN32
    const char* port = "COM6";
#else
    const char* port = "/dev/ttyUSB0";
#endif

    modbus_t *ctx = modbus_new_rtu(port, 9600, 'N', 8, 1);
    if (!ctx) {
        printf("Unable to create libmodbus context\n");
        return -1;
    }

    modbus_set_slave(ctx, 1);

    if (modbus_connect(ctx) == -1) {
        printf("Connection failed\n");
        modbus_free(ctx);
        return -1;
    }

    uint16_t reg;
    if (modbus_read_registers(ctx, 0, 1, &reg) == 1) {
        printf("Register[0] = %d\n", reg);
    }

    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}