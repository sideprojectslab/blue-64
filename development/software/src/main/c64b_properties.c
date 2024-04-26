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

#include <nvs.h>
#include <nvs_flash.h>

#include "c64b_properties.h"

//----------------------------------------------------------------------------//
// properties

unsigned int kb_map = KB_MAP_SYMBOLIC;
unsigned int ct_map[CT_MAP_IDX_NUM] = {0};

const char* ct_map_key[CT_MAP_IDX_NUM] =
{
	"c64b.ct_by",
	"c64b.ct_bh",
	"c64b.ct_bm",
	"c64b.ct_lt",
	"c64b.ct_rt",
	"c64b.ct_ls",
	"c64b.ct_rs"
};

//----------------------------------------------------------------------------//
// functions

void c64b_property_reset(void)
{
	ESP_ERROR_CHECK(nvs_flash_erase());
	uni_property_init();
//	ESP_ERROR_CHECK(nvs_flash_init());
}


void c64b_property_set_u8(const char* key, uint8_t value)
{
	uni_property_value_t prop;
	prop.u8 = value;
	uni_property_set(key, UNI_PROPERTY_TYPE_U8, prop);
}

uint8_t c64b_property_get_u8(const char* key, uint8_t def)
{
	uni_property_value_t prop;
	prop.u8 = def;
	prop = uni_property_get(key, UNI_PROPERTY_TYPE_U8, prop);
	return prop.u8;
}

void c64b_property_init(void)
{
//	uni_property_init();
	kb_map = c64b_property_get_u8(C64B_PROPERTY_KEY_KB_MAP, KB_MAP_SYMBOLIC);

	ct_map[CT_MAP_IDX_BY] = c64b_property_get_u8(ct_map_key[CT_MAP_IDX_BY], c64b_keyboard_key_to_idx("~f1~"));
	ct_map[CT_MAP_IDX_BH] = c64b_property_get_u8(ct_map_key[CT_MAP_IDX_BH], C64B_KB_IDX_NONE);
	ct_map[CT_MAP_IDX_BM] = c64b_property_get_u8(ct_map_key[CT_MAP_IDX_BM], c64b_keyboard_key_to_idx(" "));
	ct_map[CT_MAP_IDX_LT] = c64b_property_get_u8(ct_map_key[CT_MAP_IDX_LT], C64B_KB_IDX_NONE);
	ct_map[CT_MAP_IDX_RT] = c64b_property_get_u8(ct_map_key[CT_MAP_IDX_RT], C64B_KB_IDX_NONE);
	ct_map[CT_MAP_IDX_LS] = c64b_property_get_u8(ct_map_key[CT_MAP_IDX_LS], C64B_KB_IDX_NONE);
	ct_map[CT_MAP_IDX_RS] = c64b_property_get_u8(ct_map_key[CT_MAP_IDX_RS], C64B_KB_IDX_NONE);
}
