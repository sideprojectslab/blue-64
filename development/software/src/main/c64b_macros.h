//----------------------------------------------------------------------------//
//         .XXXXXXXXXXXXXXXX.  .XXXXXXXXXXXXXXXX.  .XX.                       //
//         XXXXXXXXXXXXXXXXX'  XXXXXXXXXXXXXXXXXX  XXXX                       //
//         XXXX                XXXX          XXXX  XXXX                       //
//         XXXXXXXXXXXXXXXXX.  XXXXXXXXXXXXXXXXXX  XXXX                       //
//         'XXXXXXXXXXXXXXXXX  XXXXXXXXXXXXXXXXX'  XXXX                       //
//                       XXXX  XXXX                XXXX                       //
//         .XXXXXXXXXXXXXXXXX  XXXX                XXXXXXXXXXXXXXXXX.         //
//         'XXXXXXXXXXXXXXXX'  'XX'                'XXXXXXXXXXXXXXXX'         //
//----------------------------------------------------------------------------//
//             Copyright 2023 Vittorio Pascucci (SideProjectsLab)             //
//                   Software licensed under CC BY-NC-SA 4.0.                 //
//                    To view a copy of this license, visit                   //
//               http://creativecommons.org/licenses/by-nc-sa/4.0/            //
//----------------------------------------------------------------------------//

#ifndef KEYBOARD_MACROS_H
#define KEYBOARD_MACROS_H

#include <stddef.h>

typedef enum
{
	KB_MACRO_NONE = -1,
	KB_MACRO_TAPE,
	KB_MACRO_DISK_SEARCH,
	KB_MACRO_DISK_AUTO,
	KB_MACRO_SYS_INFO,
	KB_MACRO_COLOR_CHANGE,
	KB_MACRO_COUNT
} t_c64b_macro_id;

extern const char* feed_cmd_gui[KB_MACRO_COUNT];
extern const char* feed_cmd_str[KB_MACRO_COUNT];

#endif
