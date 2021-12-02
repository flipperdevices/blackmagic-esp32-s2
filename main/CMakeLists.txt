set(SOURCES 
    "main.c"
    "usb-cdc.c"
    "nvs.c"
    "led.c"
    "cli-uart.c"
    "i2c.c"
    "delay.c"
    "network.c"
    "network-http.c"
    "network-uart.c"
    "network-gdb.c"
    "cli.c"
    "cli-uart.c"
    "cli-commands.c"
    "cli-commands-gpio.c"
    "cli-commands-wifi.c"
    "cli-commands-device-info.c"
    "cli-args.c"
)

set(INCLUDES 
    "."
)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDES})

# Commit
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE FW_GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Branch
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE FW_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Branch number
execute_process(
    COMMAND git rev-list --count HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE FW_GIT_BRANCH_NUM
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Version
execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE FW_GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Build date
string(TIMESTAMP FW_BUILD_DATE "%d-%m-%Y")

message(STATUS "FW date: ${FW_BUILD_DATE}")
message(STATUS "FW commit: ${FW_GIT_COMMIT}")
message(STATUS "FW branch: ${FW_GIT_BRANCH}")
message(STATUS "FW branch num: ${FW_GIT_BRANCH_NUM}")
message(STATUS "FW version: ${FW_GIT_VERSION}")

set_property(SOURCE "cli-commands-device-info.c" APPEND PROPERTY COMPILE_OPTIONS -DFW_BUILD_DATE="${FW_BUILD_DATE}")
set_property(SOURCE "cli-commands-device-info.c" APPEND PROPERTY COMPILE_OPTIONS -DFW_GIT_COMMIT="${FW_GIT_COMMIT}")
set_property(SOURCE "cli-commands-device-info.c" APPEND PROPERTY COMPILE_OPTIONS -DFW_GIT_BRANCH="${FW_GIT_BRANCH}")
set_property(SOURCE "cli-commands-device-info.c" APPEND PROPERTY COMPILE_OPTIONS -DFW_GIT_BRANCH_NUM="${FW_GIT_BRANCH_NUM}")
set_property(SOURCE "cli-commands-device-info.c" APPEND PROPERTY COMPILE_OPTIONS -DFW_GIT_VERSION="${FW_GIT_VERSION}")