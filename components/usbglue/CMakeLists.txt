idf_component_register(REQUIRES esp_rom app_update spi_flash freertos soc driver)

idf_component_get_property(FREERTOS_ORIG_INCLUDE_PATH freertos ORIG_INCLUDE_PATH)

idf_build_get_property(idf_target IDF_TARGET)

if(${idf_target} STREQUAL "esp32s2")
    target_compile_options(${COMPONENT_TARGET} INTERFACE
        "-DCFG_TUSB_MCU=OPT_MCU_ESP32S2"
    )
endif()

if(${idf_target} STREQUAL "esp32s3")
    target_compile_options(${COMPONENT_TARGET} INTERFACE
        "-DCFG_TUSB_MCU=OPT_MCU_ESP32S3"
    )
endif()

idf_component_get_property(tusb_lib espressif__tinyusb COMPONENT_LIB)
target_include_directories(${tusb_lib} PRIVATE ./config)

target_include_directories(${COMPONENT_TARGET} INTERFACE
    "${FREERTOS_ORIG_INCLUDE_PATH}"
    "${COMPONENT_DIR}/config/"
    "${COMPONENT_DIR}/drivers/"
    "${COMPONENT_DIR}/drivers/dual-cdc/"
    "${COMPONENT_DIR}/tinyusb/hw/bsp/"
    "${COMPONENT_DIR}/tinyusb/src/"
    "${COMPONENT_DIR}/tinyusb/src/device"
    "${COMPONENT_DIR}/tinyusb/src/class"
)

target_sources(${COMPONENT_TARGET} INTERFACE
    "${COMPONENT_DIR}/drivers/usb-glue.c"

    # "${COMPONENT_DIR}/drivers/dual-cdc/dual-cdc-driver.c"
    "${COMPONENT_DIR}/drivers/dual-cdc/dual-cdc-descriptors.c"

    # "${COMPONENT_DIR}/drivers/dap-link/vendor_device.c"
    "${COMPONENT_DIR}/drivers/dap-link/dap-link-descriptors.c"
)
