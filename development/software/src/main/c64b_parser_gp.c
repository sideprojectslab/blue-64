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

extern t_c64b_keyboard   keyboard;
extern t_c64b_kb_owner   kb_owner;
extern SemaphoreHandle_t kbrd_sem_h;
extern SemaphoreHandle_t feed_sem_h;
extern bool              swap_ports;
static t_c64b_macro_id   kb_macro_id  = 0;
static bool              kb_macro_sel = false; // allows going back to same macro after selection

//----------------------------------------------------------------------------//

bool c64b_parse_gamepad_menu(uni_gamepad_t* gp, uni_gamepad_t* gp_old)
{
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
			else
			{
				xSemaphoreGive(feed_sem_h);
			}
		}
		else
		{
			xSemaphoreGive(feed_sem_h);
		}
	}

	return true; // placeholder for now
}

//----------------------------------------------------------------------------//

bool c64b_parse_gamepad_swap(uni_gamepad_t* gp, uni_gamepad_t* gp_old)
{
	bool swap_ports = false;

	if((gp->buttons & BTN_Y_MASK) && !(gp_old->buttons & BTN_Y_MASK))
	{
		logi("Swapping Ports\n");
		swap_ports = true;
		if(xSemaphoreTake(kbrd_sem_h, portMAX_DELAY) == pdTRUE)
		{
			c64b_keyboard_reset(&keyboard);
			xSemaphoreGive(kbrd_sem_h);
		}
	}

	return swap_ports;
}

//----------------------------------------------------------------------------//

bool c64b_parse_gamepad_kbemu(uni_gamepad_t* gp, uni_gamepad_t* gp_old, t_c64b_cport_idx cport_idx)
{
	bool kb_nop = true;

	if(xSemaphoreTake(kbrd_sem_h, (TickType_t)0) == pdTRUE)
	{
		if((kb_owner == cport_idx + 1) || (kb_owner == KB_OWNER_NONE))
		{
			kb_owner = cport_idx + 1;

			// space
			if(gp->misc_buttons & BTN_START_MASK)
			{
				if(!(gp->misc_buttons & BTN_SELECT_MASK))
				{
					kb_nop = false;
					c64b_keyboard_char_psh(&keyboard, " ");
				}
			}

			if(gp->buttons & BTN_Y_MASK)
			{
				if(!(gp->misc_buttons & BTN_SELECT_MASK))
				{
					kb_nop = false;
					c64b_keyboard_char_psh(&keyboard, "~f1~");
				}
			}

			if(kb_nop)
			{
				c64b_keyboard_keys_rel(&keyboard, true);
				kb_owner = KB_OWNER_NONE;
			}
		}
		xSemaphoreGive(kbrd_sem_h);
	}

	return kb_nop;
}

//----------------------------------------------------------------------------//

bool c64b_parse_gamepad_ctrl(uni_gamepad_t* gp, uni_gamepad_t* gp_old, t_c64b_cport_idx cport_idx)
{
	bool rr_pressed = false;
	bool ll_pressed = false;
	bool up_pressed = false;
	bool dn_pressed = false;
	bool ff_pressed = false;

	if((gp->dpad & BTN_DPAD_UP_MASK) || (gp->buttons & BTN_A_MASK))
		up_pressed = true;

	if((gp->dpad & BTN_DPAD_DN_MASK) || (gp->throttle != 0))
		dn_pressed = true;

	if(gp->dpad & BTN_DPAD_RR_MASK)
		rr_pressed = true;

	if(gp->dpad & BTN_DPAD_LL_MASK)
		ll_pressed = true;

	if(gp->buttons & BTN_B_MASK)
		ff_pressed = true;

	// if left analog stick is outside the dead zone it overrides the dpad
	if((abs(gp->axis_x) > ANL_DEADZONE) || (abs(gp->axis_y) > ANL_DEADZONE))
	{
		unsigned int quadrant = 0;
		if((gp->axis_x >= 0) && (gp->axis_y < 0))
		{
			quadrant = 0;
		}
		else if((gp->axis_x < 0) && (gp->axis_y < 0))
		{
			quadrant = 1;
		}
		else if((gp->axis_x < 0) && (gp->axis_y >= 0))
		{
			quadrant = 2;
		}
		else if((gp->axis_x >= 0) && (gp->axis_y >= 0))
		{
			quadrant = 3;
		}

		if(abs(gp->axis_y) < abs(gp->axis_x) * 2)
		{
			if(quadrant == 0 || quadrant == 3)
				rr_pressed = true;
			else
				ll_pressed = true;
		}

		if(abs(gp->axis_x) < abs(gp->axis_y * 2))
		{
			if(quadrant == 0 || quadrant == 1)
				up_pressed = true;
			else
				dn_pressed = true;
		}
	}

	// these GPIO accesses are all thread-safe on the ESP32
	if(rr_pressed)
		c64b_keyboard_cport_psh(&keyboard, CPORT_RR, cport_idx);
	else
		c64b_keyboard_cport_rel(&keyboard, CPORT_RR, cport_idx);

	if(ll_pressed)
		c64b_keyboard_cport_psh(&keyboard, CPORT_LL, cport_idx);
	else
		c64b_keyboard_cport_rel(&keyboard, CPORT_LL, cport_idx);

	if(up_pressed)
		c64b_keyboard_cport_psh(&keyboard, CPORT_UP, cport_idx);
	else
		c64b_keyboard_cport_rel(&keyboard, CPORT_UP, cport_idx);

	if(dn_pressed)
		c64b_keyboard_cport_psh(&keyboard, CPORT_DN, cport_idx);
	else
		c64b_keyboard_cport_rel(&keyboard, CPORT_DN, cport_idx);

	if(ff_pressed)
		c64b_keyboard_cport_psh(&keyboard, CPORT_FF, cport_idx);
	else
		c64b_keyboard_cport_rel(&keyboard, CPORT_FF, cport_idx);

	return true;
}
