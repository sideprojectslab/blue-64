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

#include "bt64_parser_kb.h"

bool bt64_parse_keyboard_positional(uni_keyboard_t* kb, uni_keyboard_t* kb_old)
{
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
						t_bt64_parse_fbak fbak;
						fbak.cport_idx  = KPORT;
						fbak.shift_lock = shft_lock;
						xQueueOverwrite(queue_ctl_fbak, (void *)(&fbak));
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
				bt64_keyboard_ctrl_psh(&keyboard);
			else
				bt64_keyboard_ctrl_rel(&keyboard);

			if(kb->modifiers & KB_LCTRL_MASK)
				bt64_keyboard_cmdr_psh(&keyboard);
			else
				bt64_keyboard_cmdr_rel(&keyboard);

			if(restore)
				bt64_keyboard_rest_psh(&keyboard);
			else
				bt64_keyboard_rest_rel(&keyboard);

			//------------------------------------------------------------------------------------//
			// regular keys

			bt64_keychain_clear();
			for (int i = 0; i < UNI_KEYBOARD_PRESSED_KEYS_MAX; i++)
			{
				if(bt64_keychain_get_size() >= MAX_KEYPRESS)
					break;

				const uint8_t key = kb->pressed_keys[i];

				// regular keys (only one pshed at a time
				switch(key) {
					// basic letters
					case HID_USAGE_KB_A:
						if(!shft)
							bt64_keychain_add("a");
						else
							bt64_keychain_add("A");
						break;
					case HID_USAGE_KB_B:
						if(!shft)
							bt64_keychain_add("b");
						else
							bt64_keychain_add("B");
						break;
					case HID_USAGE_KB_C:
						if(!shft)
							bt64_keychain_add("c");
						else
							bt64_keychain_add("C");
						break;
					case HID_USAGE_KB_D:
						if(!shft)
							bt64_keychain_add("d");
						else
							bt64_keychain_add("D");
						break;
					case HID_USAGE_KB_E:
						if(!shft)
							bt64_keychain_add("e");
						else
							bt64_keychain_add("E");
						break;
					case HID_USAGE_KB_F:
						if(!shft)
							bt64_keychain_add("f");
						else
							bt64_keychain_add("F");
						break;
					case HID_USAGE_KB_G:
						if(!shft)
							bt64_keychain_add("g");
						else
							bt64_keychain_add("G");
						break;
					case HID_USAGE_KB_H:
						if(!shft)
							bt64_keychain_add("h");
						else
							bt64_keychain_add("H");
						break;
					case HID_USAGE_KB_I:
						if(!shft)
							bt64_keychain_add("i");
						else
							bt64_keychain_add("I");
						break;
					case HID_USAGE_KB_J:
						if(!shft)
							bt64_keychain_add("j");
						else
							bt64_keychain_add("J");
						break;
					case HID_USAGE_KB_K:
						if(!shft)
							bt64_keychain_add("k");
						else
							bt64_keychain_add("K");
						break;
					case HID_USAGE_KB_L:
						if(!shft)
							bt64_keychain_add("l");
						else
							bt64_keychain_add("L");
						break;
					case HID_USAGE_KB_M:
						if(!shft)
							bt64_keychain_add("m");
						else
							bt64_keychain_add("M");
						break;
					case HID_USAGE_KB_N:
						if(!shft)
							bt64_keychain_add("n");
						else
							bt64_keychain_add("N");
						break;
					case HID_USAGE_KB_O:
						if(!shft)
							bt64_keychain_add("o");
						else
							bt64_keychain_add("O");
						break;
					case HID_USAGE_KB_P:
						if(!shft)
							bt64_keychain_add("p");
						else
							bt64_keychain_add("P");
						break;
					case HID_USAGE_KB_Q:
						if(!shft)
							bt64_keychain_add("q");
						else
							bt64_keychain_add("Q");
						break;
					case HID_USAGE_KB_R:
						if(!shft)
							bt64_keychain_add("r");
						else
							bt64_keychain_add("R");
						break;
					case HID_USAGE_KB_S:
						if(!shft)
							bt64_keychain_add("s");
						else
							bt64_keychain_add("S");
						break;
					case HID_USAGE_KB_T:
						if(!shft)
							bt64_keychain_add("t");
						else
							bt64_keychain_add("T");
						break;
					case HID_USAGE_KB_U:
						if(!shft)
							bt64_keychain_add("u");
						else
							bt64_keychain_add("U");
						break;
					case HID_USAGE_KB_V:
						if(!shft)
							bt64_keychain_add("v");
						else
							bt64_keychain_add("V");
						break;
					case HID_USAGE_KB_W:
						if(!shft)
							bt64_keychain_add("w");
						else
							bt64_keychain_add("W");
						break;
					case HID_USAGE_KB_X:
						if(!shft)
							bt64_keychain_add("x");
						else
							bt64_keychain_add("X");
						break;
					case HID_USAGE_KB_Y:
						if(!shft)
							bt64_keychain_add("y");
						else
							bt64_keychain_add("Y");
						break;
					case HID_USAGE_KB_Z:
						if(!shft)
							bt64_keychain_add("z");
						else
							bt64_keychain_add("Z");
						break;

					// numbers

					case HID_USAGE_KB_1_EXCLAMATION_MARK:
						if(!shft)
							bt64_keychain_add("1");
						else
							bt64_keychain_add("!");
						break;
					case HID_USAGE_KB_2_AT:
						if(!shft)
							bt64_keychain_add("2");
						else
							bt64_keychain_add("\"");
						break;
					case HID_USAGE_KB_3_NUMBER_SIGN:
						if(!shft)
							bt64_keychain_add("3");
						else
							bt64_keychain_add("#");
						break;
					case HID_USAGE_KB_4_DOLLAR:
						if(!shft)
							bt64_keychain_add("4");
						else
							bt64_keychain_add("$");
						break;
					case HID_USAGE_KB_5_PERCENT:
						if(!shft)
							bt64_keychain_add("5");
						else
							bt64_keychain_add("%");
						break;
					case HID_USAGE_KB_6_CARET:
						if(!shft)
							bt64_keychain_add("6");
						else
							bt64_keychain_add("&");
						break;
					case HID_USAGE_KB_7_AMPERSAND:
						if(!shft)
							bt64_keychain_add("7");
						else
							bt64_keychain_add("'");
						break;
					case HID_USAGE_KB_8_ASTERISK:
						if(!shft)
							bt64_keychain_add("8");
						else
							bt64_keychain_add("(");
						break;
					case HID_USAGE_KB_9_OPARENTHESIS:
						if(!shft)
							bt64_keychain_add("9");
						else
							bt64_keychain_add(")");
						break;
					case HID_USAGE_KB_0_CPARENTHESIS:
						if(!shft)
							bt64_keychain_add("0");
						else
							bt64_keychain_add("0");
						break;

					// other ascii keys
					case HID_USAGE_KB_SPACEBAR:
						bt64_keychain_add(" ");
						break;
					case HID_USAGE_KB_ENTER:
						bt64_keychain_add("~ret~");
						break;
					case HID_USAGE_KB_BACKSPACE:
						if(!shft)
							bt64_keychain_add("~del~");
						else
							bt64_keychain_add("~inst~");
						break;
					case HID_USAGE_KB_DELETE:
						bt64_keychain_add("~del~");
						break;
					case HID_USAGE_KB_GRAVE_ACCENT_TILDE:
						bt64_keychain_add("~arll~");
						break;
					case HID_USAGE_KB_SINGLE_DOUBLE_QUOTE:
						if(!shft)
							bt64_keychain_add(";");
						else
							bt64_keychain_add("]");
						break;
					case HID_USAGE_KB_EQUAL_PLUS:
						bt64_keychain_add("-");
						break;
					case HID_USAGE_KB_MINUS_UNDERSCORE:
						bt64_keychain_add("+");
						break;
					case HID_USAGE_KB_F9:
					case HID_USAGE_KB_HOME:
						if(!shft)
							bt64_keychain_add("~home~");
						else
							bt64_keychain_add("~clr~");
						break;
					case HID_USAGE_KB_ESCAPE:
						if(!shft)
							bt64_keychain_add("~stop~");
						else
							bt64_keychain_add("~run~");
						break;
					case HID_USAGE_KB_F10:
					case HID_USAGE_KB_INSERT:
						bt64_keychain_add("~inst~");
						break;
					case HID_USAGE_KB_END:
						bt64_keychain_add("^"); // replaced the pound symbol
						break;
					case HID_USAGE_KB_PAGE_DOWN:
						bt64_keychain_add("=");
						break;
					case HID_USAGE_KB_BACKSLASH_VERTICAL_BAR:
						if(!shft)
							bt64_keychain_add("~pi~");
						else
							bt64_keychain_add("~arup~");
						break;
					case HID_USAGE_KB_SEMICOLON_COLON:
						if(!shft)
							bt64_keychain_add(":");
						else
							bt64_keychain_add("[");
						break;
					case HID_USAGE_KB_COMMA_LESS:
						if(!shft)
							bt64_keychain_add(",");
						else
							bt64_keychain_add("<");
						break;
					case HID_USAGE_KB_DOT_GREATER:
						if(!shft)
							bt64_keychain_add(".");
						else
							bt64_keychain_add(">");
						break;
					case HID_USAGE_KB_SLASH_QUESTION:
						if(!shft)
							bt64_keychain_add("/");
						else
							bt64_keychain_add("?");
						break;
					case HID_USAGE_KB_OBRACKET_OBRACE:
						bt64_keychain_add("@");
						break;
					case HID_USAGE_KB_CBRACKET_CBRACE:
						bt64_keychain_add("*");
						break;

					// cursor arrows
					case HID_USAGE_KB_LEFT_ARROW:
						bt64_keychain_add("~ll~");
						break;
					case HID_USAGE_KB_RIGHT_ARROW:
						bt64_keychain_add("~rr~");
						break;
					case HID_USAGE_KB_UP_ARROW:
						bt64_keychain_add("~up~");
						break;
					case HID_USAGE_KB_DOWN_ARROW:
						bt64_keychain_add("~dn~");
						break;

					// F-keys
					case HID_USAGE_KB_F1:
						if(!shft)
							bt64_keychain_add("~f1~");
						else
							bt64_keychain_add("~f2~");
						break;
					case HID_USAGE_KB_F2:
						bt64_keychain_add("~f2~");
						break;
					case HID_USAGE_KB_F3:
						if(!shft)
							bt64_keychain_add("~f3~");
						else
							bt64_keychain_add("~f4~");
						break;
					case HID_USAGE_KB_F4:
						bt64_keychain_add("~f4~");
						break;
					case HID_USAGE_KB_F5:
						if(!shft)
							bt64_keychain_add("~f5~");
						else
							bt64_keychain_add("~f6~");
						break;
					case HID_USAGE_KB_F6:
						bt64_keychain_add("~f6~");
						break;
					case HID_USAGE_KB_F7:
						if(!shft)
							bt64_keychain_add("~f7~");
						else
							bt64_keychain_add("~f8~");
						break;
					case HID_USAGE_KB_F8:
						bt64_keychain_add("~f8~");
						break;
					default:
						break;
				}
			}

			//------------------------------------------------------------------------------------//
			// shift-only

			if(bt64_keychain_get_size() == 0)
			{
				if(rshft)
					bt64_keychain_add("~rsh~");
				else if(lshft)
					bt64_keychain_add("~lsh~");
			}

			bt64_keychain_update();

			if(bt64_keychain_get_size() == 0)
			{
				bt64_keyboard_keys_rel(&keyboard, true);
				kb_owner = KB_OWNER_NONE;
			}
			else
			{
				bt64_keychain_press_latest();
			}
		}
		xSemaphoreGive(kbrd_sem_h);
	}

	return (bt64_keychain_get_size() != 0);
}
