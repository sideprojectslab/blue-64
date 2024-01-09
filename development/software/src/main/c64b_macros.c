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
//                                                                            //
// Licensed under the Apache License, Version 2.0 (the "License");            //
// you may not use this file except in compliance with the License.           //
// You may obtain a copy of the License at                                    //
//                                                                            //
//     http://www.apache.org/licenses/LICENSE-2.0                             //
//                                                                            //
// Unless required by applicable law or agreed to in writing, software        //
// distributed under the License is distributed on an "AS IS" BASIS,          //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   //
// See the License for the specific language governing permissions and        //
// limitations under the License.                                             //
//----------------------------------------------------------------------------//

#include "c64b_macros.h"

//----------------------------------------------------------------------------//
// startup commands

const char* feed_cmd_gui[KB_MACRO_COUNT] =
{
	"~clr~load tape",
	"~clr~load disk",
	"~clr~run disk",
	"~clr~device info"
};

const char device_info[] =
	"~clr~"
	"0 device info:~ret~"
	"0 blue-64 by side-projects-lab~ret~"
	"0 firmware version: 0.5 (beta)";

/*
const char color_change[] =
	"~clr~"
	"poke 53280,1~ret~"
	"poke 53281,5~ret~"
	"~ctrl-psh~6~ctrl-rel~"
	"~clr~new color scheme active";
*/

const char* feed_cmd_str[KB_MACRO_COUNT] =
{
	"~clr~load~ret~",
	"~clr~load \"$\",8~ret~",
	"~clr~load \"*\",8~ret~",
	device_info
};
