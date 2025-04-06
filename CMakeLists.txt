# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(wifi_comm C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

file(GLOB_RECURSE LIBS "libs/*.c")
message(STATUS "LIBS contains the following files:")
foreach(file ${LIBS})
    message(STATUS "${file}")
endforeach()

# Add executable. Default name is the project name, version 0.1
add_executable(wifi_comm main.c ${LIBS})

pico_set_program_name(wifi_comm "wifi_comm")
pico_set_program_version(wifi_comm "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(wifi_comm 0)
pico_enable_stdio_usb(wifi_comm 1)

# Add the standard library to the build
target_link_libraries(wifi_comm
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(wifi_comm PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/libs
        
)

# Add any user requested libraries
target_link_libraries(wifi_comm 
        pico_cyw43_arch_none
        hardware_i2c
        hardware_adc
        )

pico_add_extra_outputs(wifi_comm)

