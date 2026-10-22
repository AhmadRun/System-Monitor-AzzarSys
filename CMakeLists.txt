cmake_minimum_required(VERSION 3.16...4.4)
project(sysmon VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

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

include(GNUInstallDirs)
install(TARGETS sysmon DESTINATION ${CMAKE_INSTALL_BINDIR})
