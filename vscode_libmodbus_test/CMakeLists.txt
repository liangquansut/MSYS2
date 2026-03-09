cmake_minimum_required(VERSION 3.10)
project(libmodbusTest C)

set(CMAKE_C_STANDARD 11)

# MSYS2 MinGW64 默认安装路径
set(CMAKE_C_COMPILER "C:/msys64/mingw64/bin/gcc.exe")

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    C:/msys64/mingw64/include
)

link_directories(
    C:/msys64/mingw64/lib
)

add_executable(app src/main.c)

# 链接 libmodbus
target_link_libraries(app modbus)