idf_component_register(SRCS "ReMapper.c" 
                            "gamepad_hid.c"
                            INCLUDE_DIRS "." "../../../SHARED/include/"
                            REQUIRES tinyusb)
