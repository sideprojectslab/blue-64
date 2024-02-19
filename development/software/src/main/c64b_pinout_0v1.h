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

#ifndef C64B_PINOUT_0V1_H
#define C64B_PINOUT_0V1_H

#define PIN_KCA0 21
#define PIN_KCA1 19
#define PIN_KCA2 18

#define PIN_COL0 26
#define PIN_COL1 22
#define PIN_COL2 23
#define PIN_COL3 33
#define PIN_COL4 32

#define PIN_KRA0 17
#define PIN_KRA1 16
#define PIN_KRA2 4

#define PIN_ROW0 25
#define PIN_ROW1 14
#define PIN_ROW2 27
#define PIN_ROW3 12
#define PIN_ROW4 13

#define PIN_CTRL 2
#define PIN_SHFT 0
#define PIN_CMDR 15
#define PIN_KBEN 5

#define COL_PERM (uint8_t[]){5, 1, 0, 3, 4, 6, 7, 2}
#define ROW_PERM (uint8_t[]){2, 1, 0, 5, 4, 6, 7, 3}

#if (CONFIG_BLUEPAD32_UART_OUTPUT_ENABLE == 1)
	#define PIN_nRST 255
#else
	#define PIN_nRST 1
#endif

#endif
