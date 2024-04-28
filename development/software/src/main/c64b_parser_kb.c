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

#include "c64b_parser.h"

extern bool c64b_parse_keyboard_symbolic  (uni_keyboard_t* kb, uni_keyboard_t* kb_old);
extern bool c64b_parse_keyboard_positional(uni_keyboard_t* kb, uni_keyboard_t* kb_old);

bool c64b_parse_keyboard_menu(uni_keyboard_t* kb, uni_keyboard_t* kb_old)
{
	bool kb_nop = true;

	if(xSemaphoreTake(feed_sem_h, (TickType_t)0) == pdTRUE)
	{
		for (int i = 0; i < UNI_KEYBOARD_PRESSED_KEYS_MAX; i++)
		{
			const uint8_t key = kb->pressed_keys[i];

			switch(key)
			{
				case HID_USAGE_KB_RIGHT_ARROW:
				case HID_USAGE_KB_DOWN_ARROW:
					kb_nop = false;
					menu_fwd();
					break;
				case HID_USAGE_KB_LEFT_ARROW:
				case HID_USAGE_KB_UP_ARROW:
					kb_nop = false;
					menu_bwd();
					break;
				case HID_USAGE_KB_ENTER:
					kb_nop = false;
					menu_act();
					break;
				default:
			}

			if(!kb_nop)
				break;
		}
	}

	if(kb_nop)
	{
		xSemaphoreGive(feed_sem_h);
	}

	return kb_nop;
}

bool c64b_parse_keyboard_keys(uni_keyboard_t* kb, uni_keyboard_t* kb_old)
{
	if(kb_map == KB_MAP_SYMBOLIC)
		return c64b_parse_keyboard_symbolic(kb, kb_old);
	else
		return c64b_parse_keyboard_positional(kb, kb_old);
}
