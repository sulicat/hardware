idf_component_register(SRCS "ReMapper.c" 
                            "gamepad_hid.c"
                            "hid_parsing.c"
                            INCLUDE_DIRS "." "../../../SHARED/include/"
                            REQUIRES tinyusb)
