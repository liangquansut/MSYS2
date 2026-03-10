#include "common.h"

int main() {
    modbus_t *ctx = modbus_new_rtu(SERIAL_PORT, 9600, 'N', 8, 1);
    uint16_t tab_reg[10] = {0};

    modbus_mapping_t *mb_mapping =
        modbus_mapping_new(0, 0, 10, 0);

    if (!ctx || !mb_mapping) return -1;

    modbus_set_slave(ctx, SLAVE_ID);

    if (modbus_connect(ctx) == -1) {
        printf("RTU server connect failed\n");
        return -1;
    }

    while (1) {
        uint8_t query[MODBUS_RTU_MAX_ADU_LENGTH];
        int rc = modbus_receive(ctx, query);

        if (rc > 0)
            modbus_reply(ctx, query, rc, mb_mapping);
    }

    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}