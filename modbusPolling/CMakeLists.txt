cmake_minimum_required(VERSION 3.10)
project(ModbusPolling C)

set(CMAKE_C_STANDARD 11)

message(STATUS "System: ${CMAKE_SYSTEM_NAME}")

# -----------------------------
# Windows (MSYS2 MinGW64)
# -----------------------------
if (WIN32)
    set(CMAKE_C_COMPILER "C:/msys64/mingw64/bin/gcc.exe")
    include_directories(C:/msys64/mingw64/include)
    link_directories(C:/msys64/mingw64/lib)
endif()

# -----------------------------
# Linux (Ubuntu / Debian / Arch)
# -----------------------------
if (UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBMODBUS REQUIRED libmodbus)
    include_directories(${LIBMODBUS_INCLUDE_DIRS})
    link_directories(${LIBMODBUS_LIBRARY_DIRS})
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

# -----------------------------
# Build executables
# -----------------------------
add_executable(rtu_poll 
                src/main_rtu.c
                src/polling.c
                src/rtu_poll.c)
add_executable(rtu_server src/rtu_server.c)
add_executable(tcp_poll 
                src/main_tcp.c
                src/polling.c
                src/tcp_poll.c)
add_executable(tcp_server src/tcp_server.c)

# -----------------------------
# Link libmodbus
# -----------------------------
if (WIN32)
    target_link_libraries(rtu_poll modbus)
    target_link_libraries(rtu_server modbus)
    target_link_libraries(tcp_poll modbus)
    target_link_libraries(tcp_server modbus)
else()
    target_link_libraries(rtu_poll ${LIBMODBUS_LIBRARIES})
    target_link_libraries(rtu_server ${LIBMODBUS_LIBRARIES})
    target_link_libraries(tcp_poll ${LIBMODBUS_LIBRARIES})
    target_link_libraries(tcp_server ${LIBMODBUS_LIBRARIES})
endif()