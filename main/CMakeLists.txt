set(SOURCES 
    "main.c"
    "usb-cdc.c"
    "nvs.c"
    "wifi.c"
    "led.c"
    "uart.c"
    "i2c.c"
)

set(INCLUDES 
    "."
)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDES})
