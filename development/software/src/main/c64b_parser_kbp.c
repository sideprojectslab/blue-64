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

bool c64b_parse_keyboard_positional(uni_keyboard_t* kb, uni_keyboard_t* kb_old)
{
	bool        kb_nop  = true;
	bool        ctrl    = false;
	bool        lshft   = false;
	bool        rshft   = false;
	bool        shft    = false;
	bool        restore = false;

	bool        shft_lock_press = false;
	static bool shft_lock       = false;
	static bool shft_lock_old   = false;


	if(xSemaphoreTake(kbrd_sem_h, (TickType_t)0) == pdTRUE)
	{
		if((kb_owner == KB_OWNER_KBRD) || (kb_owner == KB_OWNER_NONE))
		{
			kb_owner = KB_OWNER_KBRD;

			//------------------------------------------------------------------------------------//
			// caps lock
			shft_lock_press = false;
			for (int i = 0; i < UNI_KEYBOARD_PRESSED_KEYS_MAX; i++)
			{
				const uint8_t key = kb->pressed_keys[i];

				if(key == HID_USAGE_KB_CAPS_LOCK)
				{
					if(!shft_lock_old)
					{
						shft_lock = !shft_lock;
						c64b_parser_set_kb_leds(shft_lock ? 0x2 : 0);
					}
					shft_lock_press = true;
				}

				if(key == HID_USAGE_KB_TAB)
					ctrl = true;

				if((key == HID_USAGE_KB_PAGE_UP) || (key == HID_USAGE_KB_F12))
					restore = true;
			}
			shft_lock_old = shft_lock_press;

			//------------------------------------------------------------------------------------//
			// detecting shift
			lshft = (kb->modifiers & KB_LSHFT_MASK) || shft_lock;
			rshft = (kb->modifiers & KB_RSHFT_MASK);
			shft = lshft || rshft;

			//------------------------------------------------------------------------------------//
			// key modifiers

			if(ctrl)
				c64b_keyboard_ctrl_psh(&keyboard);
			else
				c64b_keyboard_ctrl_rel(&keyboard);

			if(kb->modifiers & KB_LCTRL_MASK)
				c64b_keyboard_cmdr_psh(&keyboard);
			else
				c64b_keyboard_cmdr_rel(&keyboard);

			if(restore)
				c64b_keyboard_rest_psh(&keyboard);
			else
				c64b_keyboard_rest_rel(&keyboard);

			//------------------------------------------------------------------------------------//
			// regular keys

			kb_nop = false;
			for (int i = 0; i < UNI_KEYBOARD_PRESSED_KEYS_MAX; i++)
			{
				const uint8_t key = kb->pressed_keys[i];

				// regular keys (only one pshed at a time
				switch(key) {
					// basic letters
					case HID_USAGE_KB_A:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "a");
						else
							c64b_keyboard_char_psh(&keyboard, "A");
						break;
					case HID_USAGE_KB_B:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "b");
						else
							c64b_keyboard_char_psh(&keyboard, "B");
						break;
					case HID_USAGE_KB_C:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "c");
						else
							c64b_keyboard_char_psh(&keyboard, "C");
						break;
					case HID_USAGE_KB_D:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "d");
						else
							c64b_keyboard_char_psh(&keyboard, "D");
						break;
					case HID_USAGE_KB_E:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "e");
						else
							c64b_keyboard_char_psh(&keyboard, "E");
						break;
					case HID_USAGE_KB_F:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "f");
						else
							c64b_keyboard_char_psh(&keyboard, "F");
						break;
					case HID_USAGE_KB_G:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "g");
						else
							c64b_keyboard_char_psh(&keyboard, "G");
						break;
					case HID_USAGE_KB_H:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "h");
						else
							c64b_keyboard_char_psh(&keyboard, "H");
						break;
					case HID_USAGE_KB_I:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "i");
						else
							c64b_keyboard_char_psh(&keyboard, "I");
						break;
					case HID_USAGE_KB_J:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "j");
						else
							c64b_keyboard_char_psh(&keyboard, "J");
						break;
					case HID_USAGE_KB_K:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "k");
						else
							c64b_keyboard_char_psh(&keyboard, "K");
						break;
					case HID_USAGE_KB_L:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "l");
						else
							c64b_keyboard_char_psh(&keyboard, "L");
						break;
					case HID_USAGE_KB_M:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "m");
						else
							c64b_keyboard_char_psh(&keyboard, "M");
						break;
					case HID_USAGE_KB_N:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "n");
						else
							c64b_keyboard_char_psh(&keyboard, "N");
						break;
					case HID_USAGE_KB_O:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "o");
						else
							c64b_keyboard_char_psh(&keyboard, "O");
						break;
					case HID_USAGE_KB_P:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "p");
						else
							c64b_keyboard_char_psh(&keyboard, "P");
						break;
					case HID_USAGE_KB_Q:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "q");
						else
							c64b_keyboard_char_psh(&keyboard, "Q");
						break;
					case HID_USAGE_KB_R:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "r");
						else
							c64b_keyboard_char_psh(&keyboard, "R");
						break;
					case HID_USAGE_KB_S:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "s");
						else
							c64b_keyboard_char_psh(&keyboard, "S");
						break;
					case HID_USAGE_KB_T:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "t");
						else
							c64b_keyboard_char_psh(&keyboard, "T");
						break;
					case HID_USAGE_KB_U:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "u");
						else
							c64b_keyboard_char_psh(&keyboard, "U");
						break;
					case HID_USAGE_KB_V:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "v");
						else
							c64b_keyboard_char_psh(&keyboard, "V");
						break;
					case HID_USAGE_KB_W:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "w");
						else
							c64b_keyboard_char_psh(&keyboard, "W");
						break;
					case HID_USAGE_KB_X:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "x");
						else
							c64b_keyboard_char_psh(&keyboard, "X");
						break;
					case HID_USAGE_KB_Y:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "y");
						else
							c64b_keyboard_char_psh(&keyboard, "Y");
						break;
					case HID_USAGE_KB_Z:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "z");
						else
							c64b_keyboard_char_psh(&keyboard, "Z");
						break;

					// numbers

					case HID_USAGE_KB_1_EXCLAMATION_MARK:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "1");
						else
							c64b_keyboard_char_psh(&keyboard, "!");
						break;
					case HID_USAGE_KB_2_AT:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "2");
						else
							c64b_keyboard_char_psh(&keyboard, "\"");
						break;
					case HID_USAGE_KB_3_NUMBER_SIGN:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "3");
						else
							c64b_keyboard_char_psh(&keyboard, "#");
						break;
					case HID_USAGE_KB_4_DOLLAR:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "4");
						else
							c64b_keyboard_char_psh(&keyboard, "$");
						break;
					case HID_USAGE_KB_5_PERCENT:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "5");
						else
							c64b_keyboard_char_psh(&keyboard, "%%");
						break;
					case HID_USAGE_KB_6_CARET:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "6");
						else
							c64b_keyboard_char_psh(&keyboard, "&");
						break;
					case HID_USAGE_KB_7_AMPERSAND:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "7");
						else
							c64b_keyboard_char_psh(&keyboard, "'");
						break;
					case HID_USAGE_KB_8_ASTERISK:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "8");
						else
							c64b_keyboard_char_psh(&keyboard, "(");
						break;
					case HID_USAGE_KB_9_OPARENTHESIS:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "9");
						else
							c64b_keyboard_char_psh(&keyboard, ")");
						break;
					case HID_USAGE_KB_0_CPARENTHESIS:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "0");
						else
							c64b_keyboard_char_psh(&keyboard, "0");
						break;

					// other ascii keys
					case HID_USAGE_KB_SPACEBAR:
						c64b_keyboard_char_psh(&keyboard, " ");
						break;
					case HID_USAGE_KB_ENTER:
						c64b_keyboard_char_psh(&keyboard, "~ret~");
						break;
					case HID_USAGE_KB_BACKSPACE:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "~del~");
						else
							c64b_keyboard_char_psh(&keyboard, "~inst~");
						break;
					case HID_USAGE_KB_DELETE:
						c64b_keyboard_char_psh(&keyboard, "~del~");
						break;
					case HID_USAGE_KB_GRAVE_ACCENT_TILDE:
						c64b_keyboard_char_psh(&keyboard, "~arll~");
						break;
					case HID_USAGE_KB_SINGLE_DOUBLE_QUOTE:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, ";");
						else
							c64b_keyboard_char_psh(&keyboard, "]");
						break;
					case HID_USAGE_KB_EQUAL_PLUS:
						c64b_keyboard_char_psh(&keyboard, "-");
						break;
					case HID_USAGE_KB_MINUS_UNDERSCORE:
						c64b_keyboard_char_psh(&keyboard, "+");
						break;
					case HID_USAGE_KB_F9:
					case HID_USAGE_KB_HOME:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "~home~");
						else
							c64b_keyboard_char_psh(&keyboard, "~clr~");
						break;
					case HID_USAGE_KB_ESCAPE:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "~stop~");
						else
							c64b_keyboard_char_psh(&keyboard, "~run~");
						break;
					case HID_USAGE_KB_F10:
					case HID_USAGE_KB_INSERT:
						c64b_keyboard_char_psh(&keyboard, "~inst~");
						break;
					case HID_USAGE_KB_END:
						c64b_keyboard_char_psh(&keyboard, "£");
						break;
					case HID_USAGE_KB_PAGE_DOWN:
						c64b_keyboard_char_psh(&keyboard, "=");
						break;
					case HID_USAGE_KB_BACKSLASH_VERTICAL_BAR:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "~pi~");
						else
							c64b_keyboard_char_psh(&keyboard, "~arup~");
						break;
					case HID_USAGE_KB_SEMICOLON_COLON:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, ":");
						else
							c64b_keyboard_char_psh(&keyboard, "[");
						break;
					case HID_USAGE_KB_COMMA_LESS:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, ",");
						else
							c64b_keyboard_char_psh(&keyboard, "<");
						break;
					case HID_USAGE_KB_DOT_GREATER:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, ".");
						else
							c64b_keyboard_char_psh(&keyboard, ">");
						break;
					case HID_USAGE_KB_SLASH_QUESTION:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "/");
						else
							c64b_keyboard_char_psh(&keyboard, "?");
						break;
					case HID_USAGE_KB_OBRACKET_OBRACE:
						c64b_keyboard_char_psh(&keyboard, "@");
						break;
					case HID_USAGE_KB_CBRACKET_CBRACE:
						c64b_keyboard_char_psh(&keyboard, "*");
						break;

					// cursor arrows
					case HID_USAGE_KB_LEFT_ARROW:
						c64b_keyboard_char_psh(&keyboard, "~ll~");
						break;
					case HID_USAGE_KB_RIGHT_ARROW:
						c64b_keyboard_char_psh(&keyboard, "~rr~");
						break;
					case HID_USAGE_KB_UP_ARROW:
						c64b_keyboard_char_psh(&keyboard, "~up~");
						break;
					case HID_USAGE_KB_DOWN_ARROW:
						c64b_keyboard_char_psh(&keyboard, "~dn~");
						break;

					// F-keys
					case HID_USAGE_KB_F1:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "~f1~");
						else
							c64b_keyboard_char_psh(&keyboard, "~f2~");
						break;
					case HID_USAGE_KB_F2:
						c64b_keyboard_char_psh(&keyboard, "~f2~");
						break;
					case HID_USAGE_KB_F3:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "~f3~");
						else
							c64b_keyboard_char_psh(&keyboard, "~f4~");
						break;
					case HID_USAGE_KB_F4:
						c64b_keyboard_char_psh(&keyboard, "~f4~");
						break;
					case HID_USAGE_KB_F5:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "~f5~");
						else
							c64b_keyboard_char_psh(&keyboard, "~f6~");
						break;
					case HID_USAGE_KB_F6:
						c64b_keyboard_char_psh(&keyboard, "~f6~");
						break;
					case HID_USAGE_KB_F7:
						if(!shft)
							c64b_keyboard_char_psh(&keyboard, "~f7~");
						else
							c64b_keyboard_char_psh(&keyboard, "~f8~");
						break;
					case HID_USAGE_KB_F8:
						c64b_keyboard_char_psh(&keyboard, "~f8~");
						break;
					default:
						kb_nop = true;
						break;
				}

				// only the first key pshed (other than the modifiers) is registered
				if(!kb_nop)
					break;
			}

			//------------------------------------------------------------------------------------//
			// shift-only
			if(kb_nop)
			{
				if(rshft)
				{
					c64b_keyboard_char_psh(&keyboard, "~rsh~");
					kb_nop = false;
				}
				else if(lshft)
				{
					c64b_keyboard_char_psh(&keyboard, "~lsh~");
					kb_nop = false;
				}
				else
				{
					c64b_keyboard_keys_rel(&keyboard, true);
				}
			}

			if(kb_nop)
				kb_owner = KB_OWNER_NONE;
		}
		xSemaphoreGive(kbrd_sem_h);
	}

	return kb_nop;
}
