add_executable(cn
    common/main.c
    stm32/startup.c
)

set_target_properties(cn PROPERTIES
    LINKER_LANGUAGE C
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
)
target_include_directories(cn PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
