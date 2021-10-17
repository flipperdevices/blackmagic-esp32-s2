set(BM_DIR "blackmagic-fw")
set(PLATFORM_DIR "esp32-platform")

set(BM_SOURCES 
    ${BM_DIR}/src/platforms/common/swdptap.c
    ${BM_DIR}/src/platforms/common/jtagtap.c
    ${BM_DIR}/src/gdb_main.c
    ${BM_DIR}/src/command.c
    ${BM_DIR}/src/crc32.c
    ${BM_DIR}/src/exception.c
    ${BM_DIR}/src/gdb_hostio.c
    ${BM_DIR}/src/gdb_main.c
    ${BM_DIR}/src/gdb_packet.c
    ${BM_DIR}/src/remote.c
    ${BM_DIR}/src/hex_utils.c
    ${BM_DIR}/src/main.c
    ${BM_DIR}/src/morse.c

    #TODO move to project
    ${PLATFORM_DIR}/platform.c
    ${PLATFORM_DIR}/gdb-glue.c
)

set(BM_TARGETS 
    ${BM_DIR}/src/target/adiv5.c
    ${BM_DIR}/src/target/cortexa.c
    ${BM_DIR}/src/target/jtag_devs.c
    ${BM_DIR}/src/target/kinetis.c
    ${BM_DIR}/src/target/lpc15xx.c
    ${BM_DIR}/src/target/lpc546xx.c
    ${BM_DIR}/src/target/nrf51.c
    ${BM_DIR}/src/target/sam3x.c
    ${BM_DIR}/src/target/samx5x.c
    ${BM_DIR}/src/target/stm32g0.c
    ${BM_DIR}/src/target/stm32l4.c
    ${BM_DIR}/src/target/adiv5_jtagdp.c
    ${BM_DIR}/src/target/cortexm.c
    ${BM_DIR}/src/target/jtag_scan.c
    ${BM_DIR}/src/target/lmi.c
    ${BM_DIR}/src/target/lpc17xx.c
    ${BM_DIR}/src/target/lpc_common.c
    ${BM_DIR}/src/target/nxpke04.c
    ${BM_DIR}/src/target/sam4l.c
    ${BM_DIR}/src/target/stm32f1.c
    ${BM_DIR}/src/target/stm32h7.c
    # ${BM_DIR}/src/target/swdptap_generic.c
    ${BM_DIR}/src/target/adiv5_swdp.c
    ${BM_DIR}/src/target/efm32.c
    # ${BM_DIR}/src/target/jtagtap_generic.c
    ${BM_DIR}/src/target/lpc11xx.c
    ${BM_DIR}/src/target/lpc43xx.c
    ${BM_DIR}/src/target/msp432.c
    ${BM_DIR}/src/target/rp.c
    ${BM_DIR}/src/target/samd.c
    ${BM_DIR}/src/target/stm32f4.c
    ${BM_DIR}/src/target/stm32l0.c
    ${BM_DIR}/src/target/target.c
)

set(BM_INCLUDE 
    ${BM_DIR}/src/include 
    ${BM_DIR}/src/platforms/common
    ${BM_DIR}/src/target
    ${BM_DIR}/libopencm3/include
    ${PLATFORM_DIR}
)

# Get the current working branch
execute_process(
    COMMAND git describe --always --dirty --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESC
    OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "FW Version: ${GIT_DESC}")

idf_component_register(SRCS ${BM_SOURCES} ${BM_TARGETS}  
                    INCLUDE_DIRS ${BM_INCLUDE})

target_compile_options(${COMPONENT_LIB} PRIVATE -DPC_HOSTED=0 -DFIRMWARE_VERSION="${GIT_DESC}" -Wno-char-subscripts -Wno-attributes)