set(srcs "main.c"
         "uni_platform_custom.c"
         "c64b_threadsafe.c"
         "c64b_keyboard.c"
         "c64b_platform.c"
         "c64b_properties.c"
         "c64b_macros.c"
         "c64b_parser_kbs.c"
         "c64b_parser_kbp.c"
         "c64b_parser_kb.c"
         "c64b_parser_gp.c"
         "c64b_parser.c"
         "c64b_update.c"
         "c64b_threadsafe.c")

set(requires "bluepad32" "fatfs")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES "${requires}")
