cmake_minimum_required(VERSION 3.10)
project(CrossPlatformModbus C)

set(CMAKE_C_STANDARD 11)

# Detect platform
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")

# -----------------------------
# Compiler selection (Windows)
# -----------------------------
if (WIN32)
    # MSYS2 MinGW64
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

# -----------------------------
# Build target
# -----------------------------
add_executable(app src/main.c)

# -----------------------------
# Link libmodbus
# -----------------------------
if (WIN32)
    target_link_libraries(app modbus)
else()
    target_link_libraries(app ${LIBMODBUS_LIBRARIES})
endif()