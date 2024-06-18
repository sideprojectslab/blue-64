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

#include "c64b_parser_kb.h"

//------------------------------------------------------------------------------
// functions to implement key precedence logic

static const t_c64b_key_id* old_keys[MAX_KEYPRESS] = {0};
static const t_c64b_key_id* new_keys[MAX_KEYPRESS] = {0};
       uint8_t new_pressed = 0;
static uint8_t old_pressed = 0;

void c64b_keychain_clear()
{
	new_pressed = 0;
}

uint8_t c64b_keychain_get_size()
{
	return new_pressed;
}


bool c64b_keychain_add(const char* s)
{
	unsigned int   idx = c64b_keyboard_key_to_idx(s);
	const t_c64b_key_id* h   = &(KEY_IDS[idx]);

	if(new_pressed >= MAX_KEYPRESS)
		return false;

	new_keys[new_pressed++] = h;
	return true;
}

void c64b_keychain_update()
{
	// first we check if the old keys are still there in the new keys.
	// if an old key is not pressed any more is removed from the list of old
	// keys. If an old key is still pressed it is removed from the list of new
	// keys

	for(unsigned int i = 0; i < old_pressed; ++i)
	{
		if(old_keys[i] == NULL)
			break;

		bool still_pressed = false;
		for(unsigned int j = 0; j < new_pressed; ++j)
		{
			if(old_keys[i] == new_keys[j])
			{
				still_pressed = true;
				new_keys[j] = NULL;
				break;
			}
		}

		if(still_pressed == false)
			old_keys[i] = NULL;
	}

	// next, the old keys are compacted to remove holes
	unsigned int missing = 0;
	for(unsigned int i = 0; i < old_pressed; ++i)
	{
		if(old_keys[i] == NULL)
			missing += 1;
		else
			old_keys[i - missing] = old_keys[i];
	}

	// then we add the new keys to the buffer in no particular order
	old_pressed = old_pressed - missing;
	for(unsigned int i = 0; i < new_pressed; ++i)
		if(new_keys[i] != NULL)
			old_keys[old_pressed++] = new_keys[i];
}

bool c64b_keychain_press_latest()
{
	if(old_pressed == 0)
		return false;
	return c64b_keyboard_key_psh(&keyboard, old_keys[old_pressed - 1]);
}


//------------------------------------------------------------------------------
// main parser functions

bool c64b_parse_keyboard_menu(uni_keyboard_t* kb, uni_keyboard_t* kb_old)
{
	bool kb_nop = true;

	for (int i = 0; i < UNI_KEYBOARD_PRESSED_KEYS_MAX; i++)
	{
		const uint8_t key = kb->pressed_keys[i];

		switch(key)
		{
			case HID_USAGE_KB_RIGHT_ARROW:
			case HID_USAGE_KB_DOWN_ARROW:
				kb_nop = false;
				if(xSemaphoreTake(mcro_sem_h, (TickType_t)0) == pdTRUE)
					menu_fwd();
				break;
			case HID_USAGE_KB_LEFT_ARROW:
			case HID_USAGE_KB_UP_ARROW:
				kb_nop = false;
				if(xSemaphoreTake(mcro_sem_h, (TickType_t)0) == pdTRUE)
					menu_bwd();
				break;
			case HID_USAGE_KB_BACKSPACE:
				kb_nop = false;
				if(xSemaphoreTake(mcro_sem_h, (TickType_t)0) == pdTRUE)
					menu_ext();
				break;
			case HID_USAGE_KB_ENTER:
				kb_nop = false;
				if(xSemaphoreTake(mcro_sem_h, (TickType_t)0) == pdTRUE)
					menu_act();
				break;
			default:
		}

		if(!kb_nop)
			break;
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
