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

//----------------------------------------------------------------------------//

SemaphoreHandle_t autofire_sem_h[3];
SemaphoreHandle_t afsleep_sem_h[3];
bool              autofire[3] = {0};

//----------------------------------------------------------------------------//
bool c64b_gamepad_trigger_active(int32_t trig)
{
	return (trig > 40);
}

//----------------------------------------------------------------------------//
uint8_t c64b_gamepad_analog_active(int32_t x, int32_t y)
{
	uint8_t ret = 0;

	if((abs(x) > ANL_DEADZONE) || (abs(y) > ANL_DEADZONE))
	{
		unsigned int quadrant = 0;
		if((x >= 0) && (y < 0))
		{
			quadrant = 0;
		}
		else if((x < 0) && (y < 0))
		{
			quadrant = 1;
		}
		else if((x < 0) && (y >= 0))
		{
			quadrant = 2;
		}
		else if((x >= 0) && (y >= 0))
		{
			quadrant = 3;
		}

		if(abs(y) < abs(x) * 2)
		{
			if(quadrant == 0 || quadrant == 3)
				ret |= ANL_RRMASK;
			else
				ret |= ANL_LLMASK;
		}

		if(abs(x) < abs(y * 2))
		{
			if(quadrant == 0 || quadrant == 1)
				ret |= ANL_UPMASK;
			else
				ret |= ANL_DNMASK;
		}
	}

	return ret;
}

//----------------------------------------------------------------------------//

bool c64b_gamepad_interesting(uni_gamepad_t* gp, uni_gamepad_t* gp_old)
{
	if(gp->dpad != gp_old->dpad)
		return true;

	if(gp->buttons != gp_old->buttons)
		return true;

	if(gp->misc_buttons != gp_old->misc_buttons)
		return true;

	if(c64b_gamepad_trigger_active(gp->brake) !=
	   c64b_gamepad_trigger_active(gp_old->brake))
		return true;

	if(c64b_gamepad_trigger_active(gp->throttle) !=
	   c64b_gamepad_trigger_active(gp_old->throttle))
		return true;

	if(c64b_gamepad_analog_active(gp->axis_x, gp->axis_y) !=
	   c64b_gamepad_analog_active(gp_old->axis_x, gp_old->axis_y))
		return true;

	return false;
}

//----------------------------------------------------------------------------//

static void c64b_gamepad_autofire(t_c64b_cport_idx i)
{
	while(1)
	{
		if(xSemaphoreTake(autofire_sem_h[i], (TickType_t)portMAX_DELAY) == pdTRUE)
		{
			bool active = autofire[i];
			xSemaphoreGive(autofire_sem_h[i]);

			if (active)
			{
				// make sure this tpress is smaller than the smallest autofire
				// interval (currently 100ms)
				const TickType_t tpress = 30 / portTICK_PERIOD_MS;

				c64b_keyboard_cport_psh(&keyboard, CPORT_FF, i);
				vTaskDelay(tpress);
				c64b_keyboard_cport_rel(&keyboard, CPORT_FF, i);
				vTaskDelay(tpress);

				xSemaphoreTake(afsleep_sem_h[i], af_prd - tpress * 2);
			}
		}
	}
}

//----------------------------------------------------------------------------//

static void task_c64b_gamepad_autofire_2(void *arg)
{
	c64b_gamepad_autofire(CPORT_2);
}

static void task_c64b_gamepad_autofire_1(void *arg)
{
	c64b_gamepad_autofire(CPORT_1);
}

//----------------------------------------------------------------------------//

static void c64b_gamepad_autofire_start(unsigned int i)
{
	if(xSemaphoreTake(autofire_sem_h[i], (TickType_t)portMAX_DELAY) == pdTRUE)
	{
		if (!autofire[i])
		{
			logi("starting autofire on port %i\n", i + 1);
			autofire[i] = true;
			xSemaphoreGive(afsleep_sem_h[i]); // for instant restart
		}
		xSemaphoreGive(autofire_sem_h[i]);
	}
}

//----------------------------------------------------------------------------//

static void c64b_gamepad_autofire_stop(unsigned int i)
{
	if(xSemaphoreTake(autofire_sem_h[i], (TickType_t)portMAX_DELAY) == pdTRUE)
	{
		if (autofire[i])
		{
			logi("stopping autofire on port %i\n", i + 1);
			autofire[i] = false;
		}
		xSemaphoreGive(autofire_sem_h[i]);
	}
}

//----------------------------------------------------------------------------//

void c64b_parse_gamepad_init()
{
	autofire_sem_h[1] = xSemaphoreCreateBinary();
	autofire_sem_h[2] = xSemaphoreCreateBinary();
	afsleep_sem_h[1] = xSemaphoreCreateBinary();
	afsleep_sem_h[2] = xSemaphoreCreateBinary();
	xSemaphoreGive(autofire_sem_h[1]);
	xSemaphoreGive(autofire_sem_h[2]);
	xSemaphoreGive(afsleep_sem_h[1]);
	xSemaphoreGive(afsleep_sem_h[2]);

	xTaskCreatePinnedToCore(task_c64b_gamepad_autofire_2,
	                        "autofire 2",
	                        4096,
	                        NULL,
	                        1,
	                        NULL,
	                        tskNO_AFFINITY);

	xTaskCreatePinnedToCore(task_c64b_gamepad_autofire_1,
	                        "autofire 1",
	                        4096,
	                        NULL,
	                        1,
	                        NULL,
	                        tskNO_AFFINITY);
}

//----------------------------------------------------------------------------//

bool c64b_parse_gamepad_menu(uni_gamepad_t* gp, uni_gamepad_t* gp_old)
{
	if((gp->buttons & BTN_B_MASK) && !(gp_old->buttons & BTN_B_MASK))
	{
		if(xSemaphoreTake(mcro_sem_h, (TickType_t)0) == pdTRUE)
			menu_fwd();
	}
	else if((gp->buttons & BTN_A_MASK) && !(gp_old->buttons & BTN_A_MASK))
	{
		if(xSemaphoreTake(mcro_sem_h, (TickType_t)0) == pdTRUE)
			menu_bwd();
	}
	else if((gp->buttons & BTN_X_MASK) && !(gp_old->buttons & BTN_X_MASK))
	{
		if(xSemaphoreTake(mcro_sem_h, (TickType_t)0) == pdTRUE)
			menu_ext();
	}
	else if((gp->misc_buttons & BTN_MENU_MASK) && !(gp_old->misc_buttons & BTN_MENU_MASK))
	{
		if(xSemaphoreTake(mcro_sem_h, (TickType_t)0) == pdTRUE)
			menu_act();
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
			kb_owner = KB_OWNER_NONE;
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
		if((kb_owner == cport_idx) || (kb_owner == KB_OWNER_NONE))
		{
			kb_owner = cport_idx;
			kb_nop   = false;

			if(gp->misc_buttons & BTN_MENU_MASK)
				c64b_keyboard_char_psh(&keyboard, c64b_keyboard_idx_to_key(ct_map[CT_MAP_IDX_BM]));
			else if(gp->misc_buttons & BTN_HOME_MASK)
				c64b_keyboard_char_psh(&keyboard, c64b_keyboard_idx_to_key(ct_map[CT_MAP_IDX_BH]));
			else if(gp->buttons & BTN_LS_MASK)
				c64b_keyboard_char_psh(&keyboard, c64b_keyboard_idx_to_key(ct_map[CT_MAP_IDX_LS]));
			else if(gp->buttons & BTN_RS_MASK)
				c64b_keyboard_char_psh(&keyboard, c64b_keyboard_idx_to_key(ct_map[CT_MAP_IDX_RS]));
			else if(c64b_gamepad_trigger_active(gp->brake))
				c64b_keyboard_char_psh(&keyboard, c64b_keyboard_idx_to_key(ct_map[CT_MAP_IDX_LT]));
			else if(c64b_gamepad_trigger_active(gp->throttle))
				c64b_keyboard_char_psh(&keyboard, c64b_keyboard_idx_to_key(ct_map[CT_MAP_IDX_RT]));
			else
				kb_nop = true;

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
	bool af_pressed = false;

	if((gp->dpad & BTN_DPAD_UP_MASK) || (gp->buttons & BTN_B_MASK))
		up_pressed = true;

	if((gp->dpad & BTN_DPAD_DN_MASK) || (gp->buttons & BTN_X_MASK))
		dn_pressed = true;

	if(gp->dpad & BTN_DPAD_RR_MASK)
		rr_pressed = true;

	if(gp->dpad & BTN_DPAD_LL_MASK)
		ll_pressed = true;

	if(gp->buttons & BTN_A_MASK)
		ff_pressed = true;

	if(gp->buttons & BTN_Y_MASK)
		af_pressed = true;

	uint8_t analog = c64b_gamepad_analog_active(gp->axis_x, gp->axis_y);
	if(analog & ANL_RRMASK)
		rr_pressed = true;
	if(analog & ANL_LLMASK)
		ll_pressed = true;
	if(analog & ANL_UPMASK)
		up_pressed = true;
	if(analog & ANL_DNMASK)
		dn_pressed = true;

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

	if(af_pressed)
		c64b_gamepad_autofire_start(cport_idx);
	else
		c64b_gamepad_autofire_stop(cport_idx);

	return true;
}
