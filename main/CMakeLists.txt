set(SOURCES 
    "main.c"
    "usb-cdc.c"
    "nvs.c"
    "led.c"
    "uart.c"
    "i2c.c"
    "delay.c"
    "network.c"
    "network-http.c"
    "network-uart.c"
)

set(INCLUDES 
    "."
)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDES})
