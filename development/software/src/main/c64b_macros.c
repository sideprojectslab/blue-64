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
// Software licensed under the licensed under CC BY-NC-SA 4.0. To view a copy //
//  of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/  //
//----------------------------------------------------------------------------//

#include "c64b_macros.h"

//----------------------------------------------------------------------------//
// startup commands

const char* feed_cmd_gui[KB_MACRO_COUNT] =
{
	"~clr~load tape",
	"~clr~load disk",
	"~clr~run disk",
	"~clr~device info",
	"~clr~color-change"
};

const char device_info[] =
	"~clr~"
	"0 device info:~ret~"
	"0 blue-64 by side-projects-lab~ret~"
	"0 firmware version: 0.1 (beta)";


const char color_change[] =
	"~clr~"
	"poke 53280,0~ret~"
	"poke 53281,0~ret~"
	"~ctrl-psh~6~ctrl-rel~"
	"~clr~new color scheme active";

const char* feed_cmd_str[KB_MACRO_COUNT] =
{
	"~clr~load~ret~",
	"~clr~load \"$\",8~ret~",
	"~clr~load \"*\",8~ret~",
	device_info,
	color_change,
	NULL,
	NULL,
	NULL,
	NULL
};
