set(srcs "main.c"
         "uni_platform_custom.c"
         "c64b_keyboard.c"
         "c64b_platform.c"
         "c64b_macros.c"
         "c64b_parser.c")

set(requires "bluepad32")

# See: https://gitlab.com/ricardoquesada/bluepad32/-/issues/9
if(DEFINED ENV{BLUEPAD32_ARDUINO})
    list(APPEND srcs "sketch.cpp")
    list(APPEND requires "bluepad32_arduino" "arduino")
endif()

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES "${requires}")
