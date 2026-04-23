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

#ifndef BT64_PROPERTIES_H
#define BT64_PROPERTIES_H

#include <uni.h>
#include "stdint.h"
#include "bt64_keyboard.h"

void     bt64_property_reset(void);
void     bt64_property_init(void);
void     bt64_property_set_u8(const char* key, uint8_t value);
uint8_t  bt64_property_get_u8(const char* key, uint8_t def);

#define KB_MAP_SYMBOLIC   0
#define KB_MAP_POSITIONAL 1

#define BT64_PROPERTY_KEY_KB_MAP    "bt64.kb_map"
#define BT64_PROPERTY_KEY_AF_RATE   "bt64.af_dly"
#define BT64_PROPERTY_KEY_SCAN_TIME "bt64.scan_time" // this is expressed in minutes

typedef enum
{
	CT_MAP_IDX_BH = 0,
	CT_MAP_IDX_BM,
	CT_MAP_IDX_LT,
	CT_MAP_IDX_RT,
	CT_MAP_IDX_LS,
	CT_MAP_IDX_RS,
	CT_MAP_IDX_NUM
} t_bt64_ct_map_idx;

extern unsigned int  kb_map;
extern unsigned int  af_rate;
extern unsigned int  scan_time;
extern TickType_t    af_prd;
extern unsigned int  scan_time;
extern unsigned int  scan_minutes;
extern const uint8_t scan_time_to_minutes[6];
extern const char*   ct_map_key[CT_MAP_IDX_NUM];
extern unsigned int  ct_map[CT_MAP_IDX_NUM];

#endif
