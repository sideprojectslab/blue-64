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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either expsh or implied.     //
// See the License for the specific language governing permissions and        //
// limitations under the License.                                             //
//----------------------------------------------------------------------------//

#include "c64b_parser.h"

static t_c64b_keyboard   keyboard;
static uni_hid_device_t* ctrl_id [3] = {NULL, NULL, NULL};
static uni_controller_t  ctrl_dat[3] = {{0}, {0}, {0}};
static bool              swap_ports  = false;
static SemaphoreHandle_t kb_sem_h;
static SemaphoreHandle_t feed_sem_h;
static t_c64b_kb_owner   kb_owner = KB_OWNER_NONE;
static t_c64b_macro_id   kb_macro_id = 0;
static bool              kb_macro_sel = false;

//----------------------------------------------------------------------------//
// C64-Blue functions

void c64b_parser_init()
{
	kb_sem_h   = xSemaphoreCreateBinary();
	feed_sem_h = xSemaphoreCreateBinary();
	xSemaphoreGive(kb_sem_h);
	xSemaphoreGive(feed_sem_h);

	kb_owner = KB_OWNER_NONE;

	keyboard.pin_col[0] = PIN_COL0;
	keyboard.pin_col[1] = PIN_COL1;
	keyboard.pin_col[2] = PIN_COL2;
	keyboard.pin_col[3] = PIN_COL3;
	keyboard.pin_col[4] = PIN_COL4;

	keyboard.pin_kca[0] = PIN_KCA0;
	keyboard.pin_kca[1] = PIN_KCA1;
	keyboard.pin_kca[2] = PIN_KCA2;

	keyboard.pin_row[0] = PIN_ROW0;
	keyboard.pin_row[1] = PIN_ROW1;
	keyboard.pin_row[2] = PIN_ROW2;
	keyboard.pin_row[3] = PIN_ROW3;
	keyboard.pin_row[4] = PIN_ROW4;

	keyboard.pin_kra[0] = PIN_KRA0;
	keyboard.pin_kra[1] = PIN_KRA1;
	keyboard.pin_kra[2] = PIN_KRA2;

	keyboard.pin_kben  = PIN_KBEN;
	keyboard.pin_nrst  = PIN_nRST;
	keyboard.pin_ctrl  = PIN_CTRL;
	keyboard.pin_shft  = PIN_SHFT;
	keyboard.pin_cmdr  = PIN_CMDR;

	keyboard.feed_psh_ms = 30;
	keyboard.feed_rel_ms = 30;

	c64b_keyboard_init(&keyboard);
}

//----------------------------------------------------------------------------//

void c64b_parser_connect(uni_hid_device_t* d)
{
	// keyboard ID always sits on index 0
	if(d->controller.klass == UNI_CONTROLLER_CLASS_KEYBOARD)
	{
		logi("parser: keyboard connected: %p\n", d);
		ctrl_id[0] = NULL;
	}
	// inserting controller ID in the first free location after 0
	else if(d->controller.klass == UNI_CONTROLLER_CLASS_GAMEPAD)
	{
		logi("parser: gamepad connected: %p\n", d);
		if(ctrl_id[1] == NULL)
			ctrl_id[1] = d;
		else if(ctrl_id[2] == NULL)
			ctrl_id[2] = d;
	}
	else
	{
		logi("parser: device class not supported: %d\n", d->controller.klass);
	}
}

//----------------------------------------------------------------------------//

void c64b_parser_disconnect(uni_hid_device_t* d)
{
	unsigned int idx;
	if(ctrl_id[0] == d)
		idx = 0;
	else if(ctrl_id[1] == d)
		idx = 1;
	else if(ctrl_id[2] == d)
		idx = 2;
	else
		return;

	ctrl_id[idx] = NULL;
	memset(&(ctrl_dat[idx]), 0, sizeof(uni_controller_t));
}

//----------------------------------------------------------------------------//

int c64b_parser_get_idx(uni_hid_device_t* d)
{
	for(unsigned int i = 0; i < 3; ++i)
	{
		if(ctrl_id[i] == d)
			return (i);
	}
	return -1;
}

//----------------------------------------------------------------------------//

static void task_keyboard_macro_feed(void *arg)
{
	logi("Starting Keyboard Feed\n");
	if(xSemaphoreTake(kb_sem_h, (TickType_t)10) == pdTRUE)
	{
		if((kb_owner == KB_OWNER_FEED) || (kb_owner == KB_OWNER_NONE))
		{
			kb_owner = KB_OWNER_FEED;
			c64b_keyboard_feed_str(&keyboard, (char *)arg);
			kb_owner = KB_OWNER_NONE;
		}
		xSemaphoreGive(kb_sem_h);
	}
	xSemaphoreGive(feed_sem_h);
	vTaskDelete(NULL);
}

//----------------------------------------------------------------------------//
void keyboard_macro_feed(const char* str)
{
	xTaskCreatePinnedToCore(task_keyboard_macro_feed,
	                        "keyboard-macro-feed",
	                        2048,
	                        (void * const)str,
	                        3,
	                        NULL,
	                        tskNO_AFFINITY);
}

//----------------------------------------------------------------------------//

void c64b_parse_keyboard(uni_hid_device_t* d)
{
	if(d == NULL)
		return;

	uni_controller_t* ctl = &(d->controller);

	// this function only supports keyboard
	if(ctl->klass != UNI_CONTROLLER_CLASS_KEYBOARD)
		return;

	uni_keyboard_t* kb = &(ctl->keyboard);
	uni_keyboard_t* kb_old;
	bool            kb_nop;
	bool            shft;

	if(ctrl_id[0] == d)
	{
		kb_old = &(ctrl_dat[0].keyboard);
	}
	else
	{
		logi("parser: keyboard unregistered\n");
		return;
	}

	if (memcmp(kb_old, kb, sizeof(uni_keyboard_t)) == 0)
		return;

	//uni_controller_dump(ctl);

	if(xSemaphoreTake(kb_sem_h, (TickType_t)0) == pdTRUE)
	{
		if((kb_owner == KB_OWNER_KBRD) || (kb_owner == KB_OWNER_NONE))
		{
			kb_owner = KB_OWNER_KBRD;

			// key modifiers
			for (int i = 0; i < UNI_KEYBOARD_PRESSED_KEYS_MAX; i++)
			{
				const uint8_t key = kb->pressed_keys[i];

				// modifiers
				if((key == HID_USAGE_KB_LEFT_SHIFT ) || (key == HID_USAGE_KB_RIGHT_SHIFT))
				{
					c64b_keyboard_shft_psh(&keyboard);
					shft = true;
				}
				else
				{
					c64b_keyboard_shft_rel(&keyboard);
					shft = false;
				}

				if((key == HID_USAGE_KB_LEFT_CONTROL) || (key == HID_USAGE_KB_RIGHT_CONTROL))
					c64b_keyboard_ctrl_psh(&keyboard);
				else
					c64b_keyboard_ctrl_rel(&keyboard);

				if((key == HID_USAGE_KB_LEFT_GUI) || (key == HID_USAGE_KB_RIGHT_GUI))
					c64b_keyboard_cmdr_psh(&keyboard);
				else
					c64b_keyboard_cmdr_rel(&keyboard);

				// rest key
				if(key == HID_USAGE_KB_ESCAPE)
					c64b_keyboard_rest_psh(&keyboard);
				else
					c64b_keyboard_rest_rel(&keyboard);
			}

			// regular keys
			for (int i = 0; i < UNI_KEYBOARD_PRESSED_KEYS_MAX; i++)
			{
				const uint8_t key = kb->pressed_keys[i];
				kb_nop = false;

				// regular keys (only one pshed at a time
				switch(key) {
					// basic letters
					case HID_USAGE_KB_A:
						c64b_keyboard_char_psh(&keyboard, "a");
						break;
					case HID_USAGE_KB_B:
						c64b_keyboard_char_psh(&keyboard, "b");
						break;
					case HID_USAGE_KB_C:
						c64b_keyboard_char_psh(&keyboard, "c");
						break;
					case HID_USAGE_KB_D:
						c64b_keyboard_char_psh(&keyboard, "d");
						break;
					case HID_USAGE_KB_E:
						c64b_keyboard_char_psh(&keyboard, "e");
						break;
					case HID_USAGE_KB_F:
						c64b_keyboard_char_psh(&keyboard, "f");
						break;
					case HID_USAGE_KB_G:
						c64b_keyboard_char_psh(&keyboard, "g");
						break;
					case HID_USAGE_KB_H:
						c64b_keyboard_char_psh(&keyboard, "h");
						break;
					case HID_USAGE_KB_I:
						c64b_keyboard_char_psh(&keyboard, "i");
						break;
					case HID_USAGE_KB_J:
						c64b_keyboard_char_psh(&keyboard, "j");
						break;
					case HID_USAGE_KB_K:
						c64b_keyboard_char_psh(&keyboard, "k");
						break;
					case HID_USAGE_KB_L:
						c64b_keyboard_char_psh(&keyboard, "l");
						break;
					case HID_USAGE_KB_M:
						c64b_keyboard_char_psh(&keyboard, "m");
						break;
					case HID_USAGE_KB_N:
						c64b_keyboard_char_psh(&keyboard, "n");
						break;
					case HID_USAGE_KB_O:
						c64b_keyboard_char_psh(&keyboard, "o");
						break;
					case HID_USAGE_KB_P:
						c64b_keyboard_char_psh(&keyboard, "p");
						break;
					case HID_USAGE_KB_Q:
						c64b_keyboard_char_psh(&keyboard, "q");
						break;
					case HID_USAGE_KB_R:
						c64b_keyboard_char_psh(&keyboard, "r");
						break;
					case HID_USAGE_KB_S:
						c64b_keyboard_char_psh(&keyboard, "s");
						break;
					case HID_USAGE_KB_T:
						c64b_keyboard_char_psh(&keyboard, "t");
						break;
					case HID_USAGE_KB_U:
						c64b_keyboard_char_psh(&keyboard, "u");
						break;
					case HID_USAGE_KB_V:
						c64b_keyboard_char_psh(&keyboard, "v");
						break;
					case HID_USAGE_KB_W:
						c64b_keyboard_char_psh(&keyboard, "w");
						break;
					case HID_USAGE_KB_X:
						c64b_keyboard_char_psh(&keyboard, "x");
						break;
					case HID_USAGE_KB_Y:
						c64b_keyboard_char_psh(&keyboard, "y");
						break;
					case HID_USAGE_KB_Z:
						c64b_keyboard_char_psh(&keyboard, "z");
						break;

					// numbers

					// other ascii keys
					case HID_USAGE_KB_SPACEBAR:
						c64b_keyboard_char_psh(&keyboard, " ");
						break;
					case HID_USAGE_KB_RETURN:
						c64b_keyboard_char_psh(&keyboard, "~ret~");
						break;
					case HID_USAGE_KB_BACKSPACE:
						c64b_keyboard_char_psh(&keyboard, "~del~");
						break;
					case HID_USAGE_KB_DELETE:
						c64b_keyboard_char_psh(&keyboard, "~rel~");
						break;

					// arrows
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

					default:
						kb_nop = true;
						break;
				}

				// only the first key pshed (other than the modifiers) is registered
				if(!kb_nop)
					break;
			}

			if(kb_nop)
			{
				c64b_keyboard_keys_rel(&keyboard, !shft);
				kb_owner = KB_OWNER_NONE;
			}
		}
		xSemaphoreGive(kb_sem_h);
	}
	*kb_old = *kb;
}

//----------------------------------------------------------------------------//

void c64b_parse_gamepad(uni_hid_device_t* d)
{
	if(d == NULL)
		return;

	uni_controller_t* ctl    = &(d->controller);

	// this function only supports gamepads
	if(ctl->klass != UNI_CONTROLLER_CLASS_GAMEPAD)
		return;

	t_c64b_cport_idx cport_idx;
	uni_gamepad_t*   gp = &(ctl->gamepad);
	uni_gamepad_t*   gp_old;
	bool             kb_nop = true;
	bool             cport_inhibit = false;

	if(ctrl_id[1] == d)
	{
		gp_old    = &(ctrl_dat[1].gamepad);
		cport_idx = swap_ports ? CPORT_1 : CPORT_2;
	}
	else if(ctrl_id[2] == d)
	{
		gp_old    = &(ctrl_dat[2].gamepad);
		cport_idx = swap_ports ? CPORT_2 : CPORT_1;
	}
	else
	{
		logi("parser: gamepad unregistered\n");
		return;
	}

	if (memcmp(gp_old, gp, sizeof(uni_gamepad_t)) == 0)
		return;

	//uni_controller_dump(ctl);

	//------------------------------------------------------------------------//

	if(gp->misc_buttons & BTN_SELECT_MASK)
	{
		cport_inhibit = true;
		//--------------------------------------------------------------------//
		// keyboard macros

		if(xSemaphoreTake(feed_sem_h, (TickType_t)0) == pdTRUE)
		{
			if((gp->buttons & BTN_B_MASK) && !(gp_old->buttons & BTN_B_MASK))
			{
				if(kb_macro_sel)
					kb_macro_id = kb_macro_id == KB_MACRO_COUNT - 1 ? 0 : kb_macro_id + 1;
				kb_macro_sel = true;
				keyboard_macro_feed(feed_cmd_gui[kb_macro_id]);
			}

			else if((gp->buttons & BTN_A_MASK) && !(gp_old->buttons & BTN_A_MASK))
			{
				if(kb_macro_sel)
					kb_macro_id = kb_macro_id == 0 ? KB_MACRO_COUNT - 1 : kb_macro_id - 1;
				kb_macro_sel = true;
				keyboard_macro_feed(feed_cmd_gui[kb_macro_id]);
			}

			else if((gp->misc_buttons & BTN_START_MASK) && !(gp_old->misc_buttons & BTN_START_MASK))
			{
				if(kb_macro_sel)
				{
					kb_macro_sel = false;
					keyboard_macro_feed(feed_cmd_str[kb_macro_id]);
				}
			}

			else
			{
				xSemaphoreGive(feed_sem_h);
			}
		}
	}

	//---------------------------------------------------------------------//
	// direct keyboard control
	if(xSemaphoreTake(kb_sem_h, (TickType_t)0) == pdTRUE)
	{
		if((kb_owner == cport_idx + 1) || (kb_owner == KB_OWNER_NONE))
		{
			kb_owner = cport_idx + 1;

			// space
			if(!cport_inhibit)
				if(gp->misc_buttons & BTN_START_MASK)
				{
					kb_nop = false;
					if(!(gp->misc_buttons & BTN_SELECT_MASK))
					{
						c64b_keyboard_char_psh(&keyboard, " ");
						c64b_keyboard_cmdr_psh(&keyboard);
					}
				}

			if(kb_nop)
			{
				c64b_keyboard_keys_rel(&keyboard, true);
				c64b_keyboard_cmdr_rel(&keyboard);
				kb_owner = KB_OWNER_NONE;
			}

			// swap ports
			if((gp->misc_buttons & BTN_HOME_MASK) && !(gp_old->misc_buttons & BTN_HOME_MASK))
			{
				logi("Swapping Ports\n");
				swap_ports ^= true;
				c64b_keyboard_reset(&keyboard);
			}
		}
		xSemaphoreGive(kb_sem_h);
	}

	//------------------------------------------------------------------------//
	// controller ports override characters
	if(!cport_inhibit)
	{
		if((gp->dpad & BTN_DPAD_UP_MASK) || (gp->buttons & BTN_A_MASK))
			c64b_keyboard_cport_psh(&keyboard, CPORT_UP, cport_idx);
		else
			c64b_keyboard_cport_rel(&keyboard, CPORT_UP, cport_idx);

		if((gp->dpad & BTN_DPAD_DN_MASK) || (gp->throttle != 0))
			c64b_keyboard_cport_psh(&keyboard, CPORT_DN, cport_idx);
		else
			c64b_keyboard_cport_rel(&keyboard, CPORT_DN, cport_idx);

		if(gp->dpad & BTN_DPAD_RR_MASK)
			c64b_keyboard_cport_psh(&keyboard, CPORT_RR, cport_idx);
		else
			c64b_keyboard_cport_rel(&keyboard, CPORT_RR, cport_idx);

		if(gp->dpad & BTN_DPAD_LL_MASK)
			c64b_keyboard_cport_psh(&keyboard, CPORT_LL, cport_idx);
		else
			c64b_keyboard_cport_rel(&keyboard, CPORT_LL, cport_idx);

		if(gp->buttons & BTN_B_MASK)
			c64b_keyboard_cport_psh(&keyboard, CPORT_FF, cport_idx);
		else
			c64b_keyboard_cport_rel(&keyboard, CPORT_FF, cport_idx);
	}
	else
	{
		c64b_keyboard_cport_rel(&keyboard, CPORT_UP, cport_idx);
		c64b_keyboard_cport_rel(&keyboard, CPORT_DN, cport_idx);
		c64b_keyboard_cport_rel(&keyboard, CPORT_LL, cport_idx);
		c64b_keyboard_cport_rel(&keyboard, CPORT_RR, cport_idx);
		c64b_keyboard_cport_rel(&keyboard, CPORT_FF, cport_idx);
	}

	*gp_old = *gp;
}

//----------------------------------------------------------------------------//

void c64b_parse(uni_hid_device_t* d)
{
	if(ctrl_id[0] == d)
		c64b_parse_keyboard(d);
	else
		c64b_parse_gamepad(d);
}
