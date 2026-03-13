#pragma once
#include <stdio.h>
#include <modbus/modbus.h>

#ifdef _WIN32
    #define SERIAL_PORT_CLIENT "COM5"
    #define SERIAL_PORT_SERVER "COM6"
#else
    #define SERIAL_PORT_CLIENT "/dev/ttyUSB0"
    #define SERIAL_PORT_SERVER "/dev/ttyUSB1"
#endif

#define SLAVE_ID 1