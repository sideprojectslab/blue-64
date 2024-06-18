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

bool c64b_parse_keyboard_symbolic(uni_keyboard_t* kb, uni_keyboard_t* kb_old)
{
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

				if(key == HID_USAGE_KB_ESCAPE)
				{
					restore = true;
				}
			}
			shft_lock_old = shft_lock_press;

			//------------------------------------------------------------------------------------//
			// detecting shift
			lshft = (kb->modifiers & KB_LSHFT_MASK) || shft_lock;
			rshft = (kb->modifiers & KB_RSHFT_MASK);
			shft = lshft || rshft;

			//------------------------------------------------------------------------------------//
			// key modifiers
			if((kb->modifiers & KB_LCTRL_MASK) || (kb->modifiers & KB_RCTRL_MASK))
				c64b_keyboard_ctrl_psh(&keyboard);
			else
				c64b_keyboard_ctrl_rel(&keyboard);

			if(kb->modifiers & KB_START_MASK)
				c64b_keyboard_cmdr_psh(&keyboard);
			else
				c64b_keyboard_cmdr_rel(&keyboard);

			if(restore)
				c64b_keyboard_rest_psh(&keyboard);
			else
				c64b_keyboard_rest_rel(&keyboard);

			//------------------------------------------------------------------------------------//
			// regular keys

			c64b_keychain_clear();
			for (int i = 0; i < UNI_KEYBOARD_PRESSED_KEYS_MAX; i++)
			{
				if(c64b_keychain_get_size() >= MAX_KEYPRESS)
					break;

				const uint8_t key = kb->pressed_keys[i];

				// regular keys (only one pshed at a time
				switch(key) {
					// basic letters
					case HID_USAGE_KB_A:
						if(!shft)
							c64b_keychain_add("a");
						else
							c64b_keychain_add("A");
						break;
					case HID_USAGE_KB_B:
						if(!shft)
							c64b_keychain_add("b");
						else
							c64b_keychain_add("B");
						break;
					case HID_USAGE_KB_C:
						if(!shft)
							c64b_keychain_add("c");
						else
							c64b_keychain_add("C");
						break;
					case HID_USAGE_KB_D:
						if(!shft)
							c64b_keychain_add("d");
						else
							c64b_keychain_add("D");
						break;
					case HID_USAGE_KB_E:
						if(!shft)
							c64b_keychain_add("e");
						else
							c64b_keychain_add("E");
						break;
					case HID_USAGE_KB_F:
						if(!shft)
							c64b_keychain_add("f");
						else
							c64b_keychain_add("F");
						break;
					case HID_USAGE_KB_G:
						if(!shft)
							c64b_keychain_add("g");
						else
							c64b_keychain_add("G");
						break;
					case HID_USAGE_KB_H:
						if(!shft)
							c64b_keychain_add("h");
						else
							c64b_keychain_add("H");
						break;
					case HID_USAGE_KB_I:
						if(!shft)
							c64b_keychain_add("i");
						else
							c64b_keychain_add("I");
						break;
					case HID_USAGE_KB_J:
						if(!shft)
							c64b_keychain_add("j");
						else
							c64b_keychain_add("J");
						break;
					case HID_USAGE_KB_K:
						if(!shft)
							c64b_keychain_add("k");
						else
							c64b_keychain_add("K");
						break;
					case HID_USAGE_KB_L:
						if(!shft)
							c64b_keychain_add("l");
						else
							c64b_keychain_add("L");
						break;
					case HID_USAGE_KB_M:
						if(!shft)
							c64b_keychain_add("m");
						else
							c64b_keychain_add("M");
						break;
					case HID_USAGE_KB_N:
						if(!shft)
							c64b_keychain_add("n");
						else
							c64b_keychain_add("N");
						break;
					case HID_USAGE_KB_O:
						if(!shft)
							c64b_keychain_add("o");
						else
							c64b_keychain_add("O");
						break;
					case HID_USAGE_KB_P:
						if(!shft)
							c64b_keychain_add("p");
						else
							c64b_keychain_add("P");
						break;
					case HID_USAGE_KB_Q:
						if(!shft)
							c64b_keychain_add("q");
						else
							c64b_keychain_add("Q");
						break;
					case HID_USAGE_KB_R:
						if(!shft)
							c64b_keychain_add("r");
						else
							c64b_keychain_add("R");
						break;
					case HID_USAGE_KB_S:
						if(!shft)
							c64b_keychain_add("s");
						else
							c64b_keychain_add("S");
						break;
					case HID_USAGE_KB_T:
						if(!shft)
							c64b_keychain_add("t");
						else
							c64b_keychain_add("T");
						break;
					case HID_USAGE_KB_U:
						if(!shft)
							c64b_keychain_add("u");
						else
							c64b_keychain_add("U");
						break;
					case HID_USAGE_KB_V:
						if(!shft)
							c64b_keychain_add("v");
						else
							c64b_keychain_add("V");
						break;
					case HID_USAGE_KB_W:
						if(!shft)
							c64b_keychain_add("w");
						else
							c64b_keychain_add("W");
						break;
					case HID_USAGE_KB_X:
						if(!shft)
							c64b_keychain_add("x");
						else
							c64b_keychain_add("X");
						break;
					case HID_USAGE_KB_Y:
						if(!shft)
							c64b_keychain_add("y");
						else
							c64b_keychain_add("Y");
						break;
					case HID_USAGE_KB_Z:
						if(!shft)
							c64b_keychain_add("z");
						else
							c64b_keychain_add("Z");
						break;

					// numbers

					case HID_USAGE_KB_1_EXCLAMATION_MARK:
						if(!shft)
							c64b_keychain_add("1");
						else
							c64b_keychain_add("!");
						break;
					case HID_USAGE_KB_2_AT:
						if(!shft)
							c64b_keychain_add("2");
						else
							c64b_keychain_add("@");
						break;
					case HID_USAGE_KB_3_NUMBER_SIGN:
						if(!shft)
							c64b_keychain_add("3");
						else
							c64b_keychain_add("#");
						break;
					case HID_USAGE_KB_4_DOLLAR:
						if(!shft)
							c64b_keychain_add("4");
						else
							c64b_keychain_add("$");
						break;
					case HID_USAGE_KB_5_PERCENT:
						if(!shft)
							c64b_keychain_add("5");
						else
							c64b_keychain_add("%");
						break;
					case HID_USAGE_KB_6_CARET:
						if(!shft)
							c64b_keychain_add("6");
						else
							c64b_keychain_add("^"); // replaced the pound symbol
						break;
					case HID_USAGE_KB_7_AMPERSAND:
						if(!shft)
							c64b_keychain_add("7");
						else
							c64b_keychain_add("&");
						break;
					case HID_USAGE_KB_8_ASTERISK:
						if(!shft)
							c64b_keychain_add("8");
						else
							c64b_keychain_add("*");
						break;
					case HID_USAGE_KB_9_OPARENTHESIS:
						if(!shft)
							c64b_keychain_add("9");
						else
							c64b_keychain_add("(");
						break;
					case HID_USAGE_KB_0_CPARENTHESIS:
						if(!shft)
							c64b_keychain_add("0");
						else
							c64b_keychain_add(")");
						break;

					// other ascii keys
					case HID_USAGE_KB_SPACEBAR:
						c64b_keychain_add(" ");
						break;
					case HID_USAGE_KB_ENTER:
						c64b_keychain_add("~ret~");
						break;
					case HID_USAGE_KB_BACKSPACE:
						c64b_keychain_add("~del~");
						break;
					case HID_USAGE_KB_F12:
					case HID_USAGE_KB_DELETE:
						c64b_keychain_add("~clr~");
						break;
					case HID_USAGE_KB_GRAVE_ACCENT_TILDE:
						c64b_keychain_add("~arll~");
						break;
					case HID_USAGE_KB_SINGLE_DOUBLE_QUOTE:
						if(!shft)
							c64b_keychain_add("'");
						else
							c64b_keychain_add("\"");
						break;
					case HID_USAGE_KB_EQUAL_PLUS:
						if(!shft)
							c64b_keychain_add("=");
						else
							c64b_keychain_add("+");
						break;
					case HID_USAGE_KB_MINUS_UNDERSCORE:
						c64b_keychain_add("-");
						break;
					case HID_USAGE_KB_F9:
					case HID_USAGE_KB_HOME:
						if(!shft)
							c64b_keychain_add("~home~");
						break;
					case HID_USAGE_KB_F10:
					case HID_USAGE_KB_INSERT:
						c64b_keychain_add("~inst~");
						break;
					case HID_USAGE_KB_TAB:
						if(!shft)
							c64b_keychain_add("~stop~");
						else
							c64b_keychain_add("~run~");
						break;
					case HID_USAGE_KB_BACKSLASH_VERTICAL_BAR:
						if(!shft)
							c64b_keychain_add("~arup~");
						else
							c64b_keychain_add("~pi~");
						break;
					case HID_USAGE_KB_SEMICOLON_COLON:
						if(!shft)
							c64b_keychain_add(";");
						else
							c64b_keychain_add(":");
						break;
					case HID_USAGE_KB_COMMA_LESS:
						if(!shft)
							c64b_keychain_add(",");
						else
							c64b_keychain_add("<");
						break;
					case HID_USAGE_KB_DOT_GREATER:
						if(!shft)
							c64b_keychain_add(".");
						else
							c64b_keychain_add(">");
						break;
					case HID_USAGE_KB_SLASH_QUESTION:
						if(!shft)
							c64b_keychain_add("/");
						else
							c64b_keychain_add("?");
						break;
					case HID_USAGE_KB_OBRACKET_OBRACE:
						c64b_keychain_add("[");
						break;
					case HID_USAGE_KB_CBRACKET_CBRACE:
						c64b_keychain_add("]");
						break;

					// cursor arrows
					case HID_USAGE_KB_LEFT_ARROW:
						c64b_keychain_add("~ll~");
						break;
					case HID_USAGE_KB_RIGHT_ARROW:
						c64b_keychain_add("~rr~");
						break;
					case HID_USAGE_KB_UP_ARROW:
						c64b_keychain_add("~up~");
						break;
					case HID_USAGE_KB_DOWN_ARROW:
						c64b_keychain_add("~dn~");
						break;

					// F-keys
					case HID_USAGE_KB_F1:
						c64b_keychain_add("~f1~");
						break;
					case HID_USAGE_KB_F2:
						c64b_keychain_add("~f2~");
						break;
					case HID_USAGE_KB_F3:
						c64b_keychain_add("~f3~");
						break;
					case HID_USAGE_KB_F4:
						c64b_keychain_add("~f4~");
						break;
					case HID_USAGE_KB_F5:
						c64b_keychain_add("~f5~");
						break;
					case HID_USAGE_KB_F6:
						c64b_keychain_add("~f6~");
						break;
					case HID_USAGE_KB_F7:
						c64b_keychain_add("~f7~");
						break;
					case HID_USAGE_KB_F8:
						c64b_keychain_add("~f8~");
						break;

					default:
						break;
				}
			}

			//------------------------------------------------------------------------------------//
			// shift-only

			if(c64b_keychain_get_size() == 0)
			{
				if(rshft)
					c64b_keychain_add("~rsh~");
				else if(lshft)
					c64b_keychain_add("~lsh~");
			}

			c64b_keychain_update();

			if(c64b_keychain_get_size() == 0)
			{
				c64b_keyboard_keys_rel(&keyboard, true);
				kb_owner = KB_OWNER_NONE;
			}
			else
			{
				c64b_keychain_press_latest();
			}
		}
		xSemaphoreGive(kbrd_sem_h);
	}

	return (c64b_keychain_get_size() != 0);
}
