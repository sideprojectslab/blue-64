set(srcs
	"main.c"
	"uni_platform_custom.c"
	"bt64_bt.c"
	"bt64_bt_allowlist.c"
	"bt64_threadsafe.c"
	"bt64_keyboard.c"
	"bt64_platform.c"
	"bt64_properties.c"
	"bt64_macros.c"
	"bt64_parser_kbs.c"
	"bt64_parser_kbp.c"
	"bt64_parser_kb.c"
	"bt64_parser_gp.c"
	"bt64_parser.c"
	"bt64_update.c"
	"bt64_threadsafe.c")

set(requires "bluepad32" "btstack" "fatfs")

idf_component_register(
	SRCS "${srcs}"
	INCLUDE_DIRS
		"."
		"../../../../ext/bluepad32/src/components/bluepad32"
	REQUIRES "${requires}")

target_link_libraries(${COMPONENT_LIB} PUBLIC __idf_bluepad32)
