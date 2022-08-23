set(SOURCES 
    "main.c"
    # "usb-cdc.c"
    "usb-uart.c"
    "nvs.c"
    "nvs-config.c"
    # "led.c"
    "led-dotstar.c"
    "cli-uart.c"
    "i2c.c"
    "delay.c"
    "network.c"
    "network-http.c"
    "network-gdb.c"
    "cli-uart.c"
    "cli/cli.c"
    "cli/cli-commands.c"
    "cli/cli-commands-gpio.c"
    "cli/cli-commands-wifi.c"
    "cli/cli-commands-config.c"
    "cli/cli-commands-device-info.c"
    "cli/cli-args.c"
    "soft-uart-log.c"
    "factory-reset-service.c"
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

set(INFO_FLAGS
    -DFW_BUILD_DATE="${FW_BUILD_DATE}"
    -DFW_GIT_COMMIT="${FW_GIT_COMMIT}"
    -DFW_GIT_BRANCH="${FW_GIT_BRANCH}"
    -DFW_GIT_BRANCH_NUM="${FW_GIT_BRANCH_NUM}"
    -DFW_GIT_VERSION="${FW_GIT_VERSION}"
)

message(STATUS "IDF version: ${IDF_VER}")
message(STATUS "FW date: ${FW_BUILD_DATE}")
message(STATUS "FW commit: ${FW_GIT_COMMIT}")
message(STATUS "FW branch: ${FW_GIT_BRANCH}")
message(STATUS "FW branch num: ${FW_GIT_BRANCH_NUM}")
message(STATUS "FW version: ${FW_GIT_VERSION}")

set_property(SOURCE "cli/cli-commands-device-info.c" APPEND PROPERTY COMPILE_OPTIONS ${INFO_FLAGS})