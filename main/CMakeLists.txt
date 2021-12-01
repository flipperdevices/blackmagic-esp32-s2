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
)

set(INCLUDES 
    "."
)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDES})
