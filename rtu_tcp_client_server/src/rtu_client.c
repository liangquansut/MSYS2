#include "common.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int main() {
    modbus_t *ctx = modbus_new_rtu(SERIAL_PORT_CLIENT, 9600, 'N', 8, 1);
    if (!ctx) return -1;

    modbus_set_slave(ctx, SLAVE_ID);

    if (modbus_connect(ctx) == -1) {
        printf("RTU client connect failed\n");
        modbus_free(ctx);
        return -1;
    }

    uint16_t regs[10] = {0};
    printf("RTU client started, reading holding registers...\n");

    while (1) {
        int rc = modbus_read_registers(ctx, 0, 10, regs);
        if (rc == 10) {
            printf("Client HR[0..9]:");
            for (int i = 0; i < 10; i++) {
                printf(" %u", regs[i]);
            }
            printf("\n");
        } else {
            printf("RTU client read failed\n");
        }

#ifdef _WIN32
        Sleep(500);
#else
        usleep(500000);
#endif
    }

    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}