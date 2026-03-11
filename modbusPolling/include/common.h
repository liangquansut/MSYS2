#pragma once
#include <stdio.h>
#include <modbus/modbus.h>

#ifdef _WIN32
    #define SERIAL_PORT "COM3"
#else
    #define SERIAL_PORT "/dev/ttyUSB0"
#endif

#define SLAVE_ID 1