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

#ifndef C64B_PARSER_H
#define C64B_PARSER_H

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "hid_usage.h"

#include "uni_config.h"
#include "uni_bt.h"
#include "uni_gamepad.h"
#include "uni_keyboard.h"
#include "uni_hid_device.h"
#include "uni_log.h"
#include "uni_platform.h"
#include "uni_gpio.h"

#include "sdkconfig.h"
#include "c64b_keyboard.h"
#include "c64b_macros.h"

#include "c64b_pinout_0v2.h"

//----------------------------------------------------------------------------//
// Bluetooth controller masks

#define BTN_DPAD_UP_MASK 1
#define BTN_DPAD_DN_MASK 2
#define BTN_DPAD_RR_MASK 4
#define BTN_DPAD_LL_MASK 8

#define BTN_A_MASK       1
#define BTN_B_MASK       2
#define BTN_X_MASK       4
#define BTN_Y_MASK       8
#define BTN_L1_MASK      0x0010
#define BTN_L3_MASK      0x0100
#define BTN_R1_MASK      0x0020
#define BTN_R3_MASK      0x0200

#define BTN_HOME_MASK    1
#define BTN_SELECT_MASK  2
#define BTN_START_MASK   4

#define KB_LSHFT_MASK    0x2
#define KB_RSHFT_MASK    0x20
#define KB_LCTRL_MASK    0x1
#define KB_RCTRL_MASK    0x10
#define KB_LALT_MASK     0x4
#define KB_RALT_MASK     0x40
#define KB_START_MASK    0x8

#define ANL_DEADZONE     256

//----------------------------------------------------------------------------//
// keyboard owner must always be protected by a semaphore

typedef enum
{
	KB_OWNER_NONE = -1,
	KB_OWNER_KBRD,
	KB_OWNER_CTL1,
	KB_OWNER_CTL2,
	KB_OWNER_FEED,
	KB_OWNER_COUNT
} t_c64b_kb_owner;

void c64b_parser_init();
void c64b_parser_connect(uni_hid_device_t* d);
void c64b_parser_disconnect(uni_hid_device_t* d);
int  c64b_parser_get_idx(uni_hid_device_t* d);
void c64b_parse(uni_hid_device_t* d);

#endif
