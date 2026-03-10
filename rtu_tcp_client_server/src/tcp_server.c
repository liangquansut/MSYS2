#include "common.h"

int main() {
    modbus_t *ctx = modbus_new_tcp("0.0.0.0", 1502);
    int server_socket = modbus_tcp_listen(ctx, 1);

    uint16_t tab_reg[10] = {0};
    modbus_mapping_t *mb_mapping =
        modbus_mapping_new(0, 0, 10, 0);

    while (1) {
        modbus_tcp_accept(ctx, &server_socket);

        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
        int rc = modbus_receive(ctx, query);

        if (rc > 0)
            modbus_reply(ctx, query, rc, mb_mapping);
    }

    return 0;
}