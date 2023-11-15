
#ifndef KEYBOARD_MACROS_H
#define KEYBOARD_MACROS_H

typedef enum
{
	KB_MACRO_NONE = -1,
	KB_MACRO_TAPE,
	KB_MACRO_DISK_SEARCH,
	KB_MACRO_DISK_AUTO,
	KB_MACRO_COUNT
} t_c64b_macro_id;

extern char* feed_cmd_gui[KB_MACRO_COUNT];
extern char* feed_cmd_str[KB_MACRO_COUNT];

extern char* solitaire_prg[];

#endif
