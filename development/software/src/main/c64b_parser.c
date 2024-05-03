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
#include "c64b_update.h"
#include "esp_task_wdt.h"

extern bool c64b_parse_keyboard_menu (uni_keyboard_t* kb, uni_keyboard_t* kb_old);
extern bool c64b_parse_keyboard_keys (uni_keyboard_t* kb, uni_keyboard_t* kb_old);
extern bool c64b_parse_gamepad_menu  (uni_gamepad_t*  gp, uni_gamepad_t*  gp_old);
extern bool c64b_parse_gamepad_swap  (uni_gamepad_t*  gp, uni_gamepad_t*  gp_old);
extern bool c64b_parse_gamepad_kbemu (uni_gamepad_t*  gp, uni_gamepad_t*  gp_old, t_c64b_cport_idx cport_idx);
extern bool c64b_parse_gamepad_ctrl  (uni_gamepad_t*  gp, uni_gamepad_t*  gp_old, t_c64b_cport_idx cport_idx);
extern void c64b_parse_gamepad_init  ();

//----------------------------------------------------------------------------//
// Static Variables

static uni_hid_device_t* dev_ptr[3]  = {NULL, NULL, NULL};
static uni_controller_t* ctrl_ptr[3] = {NULL, NULL, NULL};
static uni_controller_t  ctrl_tmp[3] = {{0}, {0}, {0}};
static uni_controller_t  ctrl_new[3] = {{0}, {0}, {0}};
static uni_controller_t  ctrl_old[3] = {{0}, {0}, {0}};

static bool              swap_ports = false;
static const uint8_t     col_perm[] = COL_PERM;
static const uint8_t     row_perm[] = ROW_PERM;

//----------------------------------------------------------------------------//
// C64-Blue functions

//----------------------------------------------------------------------------//

int c64b_parser_get_dev_idx(uni_hid_device_t* d)
{
	for(unsigned int i = 0; i < 3; ++i)
	{
		if(ctrl_ptr[i] == &(d->controller))
			return (i);
	}
	return -1;
}


int c64b_parser_get_ctl_idx(uni_controller_t* ctl)
{
	for(unsigned int i = 0; i < 3; ++i)
	{
		if(ctrl_ptr[i] == ctl)
			return (i);
	}
	return -1;
}


void c64b_parser_set_gp_seat(uni_controller_t* ctl, unsigned int seat)
{
	if(ctl == NULL)
		return;

	unsigned int i = c64b_parser_get_ctl_idx(ctl);

	if(i < 0)
		return;

	uni_hid_device_t* d = dev_ptr[i];

	if(d->report_parser.play_dual_rumble != NULL)
	{
		d->report_parser.play_dual_rumble(d, 0, seat == 2 ? 200 : 50, 128, 128);
		logi("parser: setting rumble for seat %d\n", seat);
	}

	if(d->report_parser.set_player_leds != NULL)
	{
		d->report_parser.set_player_leds(d, 1 << (seat - 1));
		logi("parser: setting leds for seat %d\n", seat);
	}

	if(d->report_parser.set_lightbar_color != NULL)
	{
		uint8_t r = seat == 2 ? 255 : 0;
		uint8_t g = seat == 2 ? 0   : 255;
		d->report_parser.set_lightbar_color(d, r, g, 0);
		logi("parser: setting lightbar for seat %d\n", seat);
	}
}


void c64b_parser_connect(uni_hid_device_t* d)
{
	// keyboard ID always sits on index 0
	if(uni_hid_device_is_keyboard(d))
	{
		logi("parser: keyboard connected: %p\n", d);
		if(ctrl_ptr[0] == NULL)
		{
			ctrl_ptr[0] = &(d->controller);
			dev_ptr[0]  = d;
		}
	}
	// inserting controller ID in the first free location after 0
	else if(uni_hid_device_is_gamepad(d))
	{
		logi("parser: gamepad connected: %p\n", d);
		if(ctrl_ptr[2] == NULL)
		{
			ctrl_ptr[2] = &(d->controller);
			dev_ptr[2]  = d;
			c64b_parser_set_gp_seat(ctrl_ptr[2], 2);
		}
		else if(ctrl_ptr[1] == NULL)
		{
			ctrl_ptr[1] = &(d->controller);
			dev_ptr[1]  = d;
			c64b_parser_set_gp_seat(ctrl_ptr[1], 1);
		}
	}
	else
	{
		logi("parser: device class not supported: %d\n", d->controller.klass);
	}
}

//----------------------------------------------------------------------------//

static void task_keyboard_macro_feed(void *arg)
{
	logi("Starting Keyboard Feed\n");
	if(xSemaphoreTake(kbrd_sem_h, (TickType_t)10) == pdTRUE)
	{
		if((kb_owner == KB_OWNER_FEED) || (kb_owner == KB_OWNER_NONE))
		{
			kb_owner = KB_OWNER_FEED;
			c64b_keyboard_feed_str(&keyboard, (char *)arg);
			kb_owner = KB_OWNER_NONE;
		}
		xSemaphoreGive(kbrd_sem_h);
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

void c64b_parse_keyboard(uni_controller_t* ctl)
{
	if(ctl == NULL)
		return;

	// this function only supports keyboard
	if(ctl->klass != UNI_CONTROLLER_CLASS_KEYBOARD)
		return;

	uni_keyboard_t* kb = &(ctl->keyboard);
	uni_keyboard_t* kb_old;

	kb_old = &(ctrl_old[0].keyboard);

	if (memcmp(kb_old, kb, sizeof(uni_keyboard_t)) == 0)
		return;

	#if (CONFIG_BLUEPAD32_UART_OUTPUT_ENABLE == 1)
		uni_controller_dump(ctl);
	#endif

	if(kb->modifiers & KB_RALT_MASK)
		c64b_parse_keyboard_menu(kb, kb_old);
	else
		c64b_parse_keyboard_keys(kb, kb_old);

	*kb_old = *kb;
}

//----------------------------------------------------------------------------//

void c64b_parse_gamepad(uni_controller_t* ctl)
{
	if(ctl == NULL)
		return;

	// this function only supports gamepads
	if(ctl->klass != UNI_CONTROLLER_CLASS_GAMEPAD)
		return;

	t_c64b_cport_idx cport_idx;
	uni_gamepad_t*   gp = &(ctl->gamepad);
	uni_gamepad_t*   gp_old;
	bool             kb_nop = true;

	if(ctl == &(ctrl_new[1]))
	{
		gp_old    = &(ctrl_old[1].gamepad);
		cport_idx = swap_ports ? CPORT_2 : CPORT_1;
	}
	else if(ctl == &(ctrl_new[2]))
	{
		gp_old    = &(ctrl_old[2].gamepad);
		cport_idx = swap_ports ? CPORT_1 : CPORT_2;
	}
	else
	{
		logi("parser: gamepad unregistered\n");
		return;
	}

	if (memcmp(gp_old, gp, sizeof(uni_gamepad_t)) == 0)
		return;

	#if (CONFIG_BLUEPAD32_UART_OUTPUT_ENABLE == 1)
		uni_controller_dump(ctl);
	#endif

	//------------------------------------------------------------------------//

	if(gp->misc_buttons & BTN_SELECT_MASK)
	{
		if(!(gp_old->misc_buttons & BTN_SELECT_MASK))
			if(xSemaphoreTake(kbrd_sem_h, portMAX_DELAY) == pdTRUE)
			{
				kb_owner = KB_OWNER_NONE;
				xSemaphoreGive(kbrd_sem_h);
			}

		// processing special keys, all controller lines are disabled
		c64b_keyboard_cport_rel(&keyboard, CPORT_UP, cport_idx);
		c64b_keyboard_cport_rel(&keyboard, CPORT_DN, cport_idx);
		c64b_keyboard_cport_rel(&keyboard, CPORT_LL, cport_idx);
		c64b_keyboard_cport_rel(&keyboard, CPORT_RR, cport_idx);
		c64b_keyboard_cport_rel(&keyboard, CPORT_FF, cport_idx);

		//--------------------------------------------------------------------//
		// Manu
		kb_nop &= c64b_parse_gamepad_menu(gp, gp_old);

		//--------------------------------------------------------------------//
		// swap ports
		if((cport_idx == CPORT_2) || (ctrl_ptr[1] == NULL) || (ctrl_ptr[2] == NULL))
		{
			if(c64b_parse_gamepad_swap(gp, gp_old))
			{
				swap_ports = !swap_ports;
				c64b_parser_set_gp_seat(ctrl_ptr[1], swap_ports ? 2 : 1);
				c64b_parser_set_gp_seat(ctrl_ptr[2], swap_ports ? 1 : 2);
			}
		}
	}
	else
	{
		//--------------------------------------------------------------------//
		// direct keyboard control
		kb_nop &= c64b_parse_gamepad_kbemu(gp, gp_old, cport_idx);

		//--------------------------------------------------------------------//
		// controller ports override characters
		c64b_parse_gamepad_ctrl(gp, gp_old, cport_idx);
	}

	*gp_old = *gp;
}

//----------------------------------------------------------------------------//

static void task_c64b_parse(void *arg)
{
	while(1)
	{
		if(xSemaphoreTake(prse_sem_h, portMAX_DELAY) == pdTRUE)
		{
			// latching controller data in a thread-safe manner
			ctrl_new[0] = ctrl_tmp[0];
			ctrl_new[1] = ctrl_tmp[1];
			ctrl_new[2] = ctrl_tmp[2];
			xSemaphoreGive(prse_sem_h);

			c64b_parse_keyboard(&(ctrl_new[0]));
			c64b_parse_gamepad(&(ctrl_new[1]));
			c64b_parse_gamepad(&(ctrl_new[2]));
		}
	}
}

//----------------------------------------------------------------------------//

void c64b_parse(uni_hid_device_t* d)
{
	static bool first_parse = true;
	if(first_parse)
	{
		// this task always runs in the background so it needs to have
		// very low priority
		xTaskCreatePinnedToCore(task_c64b_parse,
		                        "parse task",
		                        4096 * 2,
		                        NULL,
		                        1,
		                        NULL,
		                        tskNO_AFFINITY);
	}
	first_parse = false;

	if(xSemaphoreTake(prse_sem_h, portMAX_DELAY) == pdTRUE)
	{
		if(ctrl_ptr[0] == &(d->controller))
		{
			ctrl_tmp[0] = d->controller;
		}
		else if(ctrl_ptr[1] == &(d->controller))
		{
			ctrl_tmp[1] = d->controller;
		}
		else if(ctrl_ptr[2] == &(d->controller))
		{
			ctrl_tmp[2] = d->controller;
		}
		xSemaphoreGive(prse_sem_h);
	}
}

//----------------------------------------------------------------------------//

void c64b_parser_disconnect(uni_hid_device_t* d)
{
	if(xSemaphoreTake(prse_sem_h, portMAX_DELAY) == pdTRUE)
	{
		unsigned int idx;
		if(ctrl_ptr[0] == &(d->controller))
			idx = 0;
		else if(ctrl_ptr[1] == &(d->controller))
			idx = 1;
		else if(ctrl_ptr[2] == &(d->controller))
			idx = 2;
		else
			return;

		logi("Device Disconnected: %d\n", idx);

		ctrl_ptr[idx] = NULL;
		dev_ptr[idx] = NULL;
		memset(&(ctrl_tmp[0]), 0, sizeof(uni_controller_t));
		if(idx == 0)
			ctrl_tmp[0].klass = UNI_CONTROLLER_CLASS_KEYBOARD;
		else
			ctrl_tmp[0].klass = UNI_CONTROLLER_CLASS_GAMEPAD;

		xSemaphoreGive(prse_sem_h);
	}

	if(xSemaphoreTake(kbrd_sem_h, portMAX_DELAY) == pdTRUE)
	{
		c64b_keyboard_reset(&keyboard);
		xSemaphoreGive(kbrd_sem_h);
	}
}

//----------------------------------------------------------------------------//

void c64b_parser_init()
{
	prse_sem_h = xSemaphoreCreateBinary();
	kbrd_sem_h = xSemaphoreCreateBinary();
	feed_sem_h = xSemaphoreCreateBinary();
	xSemaphoreGive(prse_sem_h);
	xSemaphoreGive(kbrd_sem_h);
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

	keyboard.col_perm  = col_perm;
	keyboard.row_perm  = row_perm;

	c64b_property_init();
	c64b_parse_gamepad_init();

	if(c64b_update_init(true) == UPDATE_OK)
	{
		const char update_started[] =
			"~clr~0 firmware update started!";

		const char update_successful[] =
			"~clr~0 successfully updated firmware!~ret~"
			"~ret~"
			"0 please switch off the computer and~ret~"
			"0 remove the sd-card";

		const char update_failed[] =
			"~clr~0 firmware updated failed!~ret~"
			"~ret~"
			"0 please switch off the computer and~ret~"
			"0 remove the sd-card";

		vTaskDelay(3000 / portTICK_PERIOD_MS);

		c64b_keyboard_init(&keyboard);
		c64b_keyboard_feed_str(&keyboard, update_started);

		if(c64b_update() == UPDATE_OK)
		{
			c64b_keyboard_init(&keyboard);
			c64b_keyboard_feed_str(&keyboard, update_successful);
		}
		else
		{
			c64b_keyboard_init(&keyboard);
			c64b_keyboard_feed_str(&keyboard, update_failed);
		}

		while(1){}
	}

	c64b_keyboard_init(&keyboard);
}
