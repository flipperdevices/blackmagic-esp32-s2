idf_component_register(REQUIRES freertos esp_system driver)

target_include_directories(${COMPONENT_TARGET} INTERFACE
    "${COMPONENT_DIR}/include/"
    "${COMPONENT_DIR}/src/"
)

target_sources(${COMPONENT_TARGET} INTERFACE
    "${COMPONENT_DIR}/src/dotstar.c"
)