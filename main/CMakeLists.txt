set(SOURCES 
    "main.c"
    "usb-cdc.c"
    "wifi.c"
)

set(INCLUDES 
    "."
)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDES})
