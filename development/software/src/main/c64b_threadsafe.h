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

#ifndef C64B_THREADSAFE_H
#define C64B_THREADSAFE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "c64b_keyboard.h"
#include "c64b_properties.h"

typedef enum
{
	KB_OWNER_NONE = -1,
	KB_OWNER_KBRD,
	KB_OWNER_CTL1,
	KB_OWNER_CTL2,
	KB_OWNER_FEED,
	KB_OWNER_COUNT
} t_c64b_kb_owner;

//----------------------------------------------------------------------------//
// unprotected

//----------------------------------------------------------------------------//
extern SemaphoreHandle_t kbrd_sem_h; // protects access to keyboard keystrokes
extern t_c64b_keyboard   keyboard;
extern t_c64b_kb_owner   kb_owner;

//----------------------------------------------------------------------------//
extern SemaphoreHandle_t mcro_sem_h; // protects access to keyboard macro
extern SemaphoreHandle_t feed_sem_h; // starts keyboaard feed thread

#endif
