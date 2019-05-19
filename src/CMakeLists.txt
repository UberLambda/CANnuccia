add_executable(cn
    common/isr.c
)

set_target_properties(cn PROPERTIES
    LINKER_LANGUAGE C
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
)
