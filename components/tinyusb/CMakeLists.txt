idf_component_register(REQUIRES esp_rom app_update spi_flash freertos soc driver)

idf_component_get_property( FREERTOS_ORIG_INCLUDE_PATH freertos ORIG_INCLUDE_PATH)

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

target_include_directories(${COMPONENT_TARGET} INTERFACE
    "${FREERTOS_ORIG_INCLUDE_PATH}"
    "${COMPONENT_DIR}/config/"
    "${COMPONENT_DIR}/dual-cdc/"
    "${COMPONENT_DIR}/tinyusb/hw/bsp/"
    "${COMPONENT_DIR}/tinyusb/src/"
    "${COMPONENT_DIR}/tinyusb/src/device"
    "${COMPONENT_DIR}/tinyusb/src/class"
)

target_sources(${COMPONENT_TARGET} INTERFACE
    "${COMPONENT_DIR}/tinyusb/src/tusb.c"
    "${COMPONENT_DIR}/tinyusb/src/common/tusb_fifo.c"
    "${COMPONENT_DIR}/tinyusb/src/device/usbd.c"
    "${COMPONENT_DIR}/tinyusb/src/device/usbd_control.c"
    "${COMPONENT_DIR}/tinyusb/src/class/audio/audio_device.c"
    "${COMPONENT_DIR}/tinyusb/src/class/bth/bth_device.c"
    "${COMPONENT_DIR}/tinyusb/src/class/cdc/cdc_device.c"
    "${COMPONENT_DIR}/tinyusb/src/class/cdc/cdc_host.c"
    "${COMPONENT_DIR}/tinyusb/src/class/cdc/cdc_rndis_host.c"
    "${COMPONENT_DIR}/tinyusb/src/class/dfu/dfu_rt_device.c"
    "${COMPONENT_DIR}/tinyusb/src/class/dfu/dfu_device.c"
    "${COMPONENT_DIR}/tinyusb/src/class/hid/hid_device.c"
    "${COMPONENT_DIR}/tinyusb/src/class/hid/hid_host.c"
    "${COMPONENT_DIR}/tinyusb/src/class/midi/midi_device.c"
    "${COMPONENT_DIR}/tinyusb/src/class/msc/msc_device.c"
    "${COMPONENT_DIR}/tinyusb/src/class/msc/msc_host.c"
    "${COMPONENT_DIR}/tinyusb/src/class/net/ncm_device.c"
    "${COMPONENT_DIR}/tinyusb/src/class/net/ecm_rndis_device.c"
    "${COMPONENT_DIR}/tinyusb/src/class/usbtmc/usbtmc_device.c"
    "${COMPONENT_DIR}/tinyusb/src/class/vendor/vendor_device.c"
    "${COMPONENT_DIR}/tinyusb/src/class/vendor/vendor_host.c"
    "${COMPONENT_DIR}/tinyusb/src/class/video/video_device.c"
    "${COMPONENT_DIR}/tinyusb/src/portable/espressif/esp32sx/dcd_esp32sx.c"
    "${COMPONENT_DIR}/tinyusb/src/host/hub.c"
    "${COMPONENT_DIR}/tinyusb/src/host/usbh.c"
    "${COMPONENT_DIR}/tinyusb/src/host/usbh_control.c"

    "${COMPONENT_DIR}/dual-cdc/dual-cdc-driver.c"
    "${COMPONENT_DIR}/dual-cdc/usb_descriptors.c"
)
