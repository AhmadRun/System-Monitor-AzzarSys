cmake_minimum_required(VERSION 3.10)
project(sysmon VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/cpu_monitor.cpp
    src/memory_monitor.cpp
    src/disk_monitor.cpp
    src/process_monitor.cpp
    src/network_monitor.cpp
    src/system_info.cpp
    src/ui.cpp
)

add_executable(sysmon ${SOURCES})

install(TARGETS sysmon DESTINATION /usr/local/bin)
