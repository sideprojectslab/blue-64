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

#include "c64b_threadsafe.h"

//----------------------------------------------------------------------------//
SemaphoreHandle_t prse_sem_h;
uni_controller_t  ctrl_tmp[3] = {{0}, {0}, {0}};

//----------------------------------------------------------------------------//
SemaphoreHandle_t mcro_sem_h;
SemaphoreHandle_t feed_sem_h;

//----------------------------------------------------------------------------//
SemaphoreHandle_t fbak_sem_h;
uint8_t           dev_fbak[3] = {{0}, {2}, {1}};

//----------------------------------------------------------------------------//
SemaphoreHandle_t kbrd_sem_h;
t_c64b_keyboard   keyboard   = {0};
t_c64b_kb_owner   kb_owner   = KB_OWNER_NONE;
