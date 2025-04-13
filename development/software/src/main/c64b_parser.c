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
extern bool c64b_gamepad_interesting (uni_gamepad_t* gp, uni_gamepad_t* gp_old);

extern bool uni_bt_allowlist_add_addr_force(bd_addr_t addr);
extern void uni_bt_enable_pairing_safe(bool enabled);
extern void uni_bt_forget_devices_safe();

//----------------------------------------------------------------------------//
// Static Variables

static uni_hid_device_t* dev_ptr[3]  = {NULL, NULL, NULL};
static uni_controller_t  ctrl_new[3] = {{0}, {0}, {0}};
static uni_controller_t  ctrl_old[3] = {{0}, {0}, {0}};

static bool              swap_ports = false;
static const uint8_t     col_perm[] = COL_PERM;
static const uint8_t     row_perm[] = ROW_PERM;

//----------------------------------------------------------------------------//
// C64-Blue functions
//----------------------------------------------------------------------------//

uni_controller_t* get_ctl(uni_hid_device_t* d)
{
	return &(d->controller);
}

//----------------------------------------------------------------------------//

void c64b_parser_set_kb_leds(uint8_t mask)
{
	if(dev_ptr[0] == NULL)
		return;

	uni_hid_parser_keyboard_set_leds(dev_ptr[0], mask);
	logi("parser: setting keyboard leds: %x\n", mask);
}

//----------------------------------------------------------------------------//

void c64b_parser_set_gp_seat(unsigned int idx, unsigned int seat)
{
	if((idx != CPORT_1) && (idx != CPORT_2))
		return;

	if((seat != CPORT_1) && (seat != CPORT_2))
		return;

	uni_hid_device_t* d = dev_ptr[idx];

	if(d == NULL)
		return;

	if(d->report_parser.set_player_leds != NULL)
	{
		d->report_parser.set_player_leds(d, 1 << (seat - 1));
		logi("parser: setting leds for seat %d\n", seat);
	}
	else if(d->report_parser.set_lightbar_color != NULL)
	{
		uint8_t r = seat == 2 ? 255 : 0;
		uint8_t g = seat == 2 ? 0   : 255;
		d->report_parser.set_lightbar_color(d, r, g, 0);
		logi("parser: setting lightbar for seat %d\n", seat);
	}
	else if(d->report_parser.play_dual_rumble != NULL)
	{
		d->report_parser.play_dual_rumble(d, 0, seat == 2 ? 400 : 150, 255, 0);
		logi("parser: setting rumble for seat %d\n", seat);
	}
}

//----------------------------------------------------------------------------//

static void task_keyboard_macro_feed(void *arg)
{
	while(1)
	{
		if(xSemaphoreTake(feed_sem_h, portMAX_DELAY) == pdTRUE)
		{
			const char * str = *(char **)arg;
			logi("Starting Keyboard Feed\n");
			if(xSemaphoreTake(kbrd_sem_h, (TickType_t)10) == pdTRUE)
			{
				if((kb_owner == KB_OWNER_FEED) || (kb_owner == KB_OWNER_NONE))
				{
					kb_owner = KB_OWNER_FEED;
					c64b_keyboard_feed_str(&keyboard, str);
					c64b_keyboard_trace_reset(&keyboard);
					kb_owner = KB_OWNER_NONE;
				}
				xSemaphoreGive(kbrd_sem_h);
			}
			xSemaphoreGive(mcro_sem_h);
			logi("Completed Keyboard Feed\n");
		}
	}
}

//----------------------------------------------------------------------------//

void keyboard_macro_feed(const char* str)
{
	static bool first_feed = true;
	static char* str_h;

	str_h = str;

	if (first_feed)
	{
		logi("parser: Creating macro feed thread\n");
		xTaskCreatePinnedToCore(task_keyboard_macro_feed,
								"keyboard-macro-feed",
								1024*6,
								(void * const)&str_h,
								TASK_PRIO_MACRO,
								NULL,
								CORE_AFFINITY);

		first_feed = false;
	}
	else
	{
		xSemaphoreGive(feed_sem_h);
	}
}

//----------------------------------------------------------------------------//
uni_error_t c64b_parser_discover(bd_addr_t addr, const char* name, uint16_t cod, uint8_t rssi)
{
	cod &= UNI_BT_COD_MINOR_MASK;
	if ((cod & UNI_BT_COD_MINOR_KEYBOARD) ||
		(cod & UNI_BT_COD_MINOR_REMOTE_CONTROL)) // gamepad / joystick
	{
		// adding the new device to the allowlist
		if(uni_bt_allowlist_is_enabled() == false)
		{
			logi("Device Detected, Adding to allowlist\n");
			uni_bt_allowlist_add_addr_force(addr);
		}
		else if(uni_bt_allowlist_is_allowed_addr(addr) == false)
		{
			logi("Unknown Device Detected, Pairing Disabled, Ignoring...\n");
			return UNI_ERROR_IGNORE_DEVICE;
		}

		// we basically try to connect with any RSSI
		return UNI_ERROR_SUCCESS;
	}

	logi("Ignoring keyboard\n");
	return UNI_ERROR_IGNORE_DEVICE;
}

//----------------------------------------------------------------------------//

void c64b_parser_connect(uni_hid_device_t* d)
{
	// keyboard ID always sits on index 0
	if(uni_hid_device_is_keyboard(d))
	{
		logi("parser: keyboard connected: %p\n", d);
		if(dev_ptr[0] == NULL)
		{
			dev_ptr[0]  = d;
		}
	}
	// inserting controller ID in the first free location after 0
	else if(uni_hid_device_is_gamepad(d))
	{
		logi("parser: gamepad connected: %p\n", d);
		if(dev_ptr[2] == NULL)
		{
			dev_ptr[2] = d;
			c64b_parser_set_gp_seat(2, swap_ports ? 1 : 2);
		}
		else if(dev_ptr[1] == NULL)
		{
			dev_ptr[1] = d;
			c64b_parser_set_gp_seat(1, swap_ports ? 2 : 1);
		}
	}
	else
	{
		logi("parser: device class not supported: %d\n", d->controller.klass);
		return;
	}
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

	#ifndef CONFIG_ESP_CONSOLE_NONE
		uni_controller_dump(ctl);
	#endif

	if(kb->modifiers & (KB_RALT_MASK | KB_LALT_MASK))
		c64b_parse_keyboard_menu(kb, kb_old);
	else
		c64b_parse_keyboard_keys(kb, kb_old);

	*kb_old = *kb;
}

//----------------------------------------------------------------------------//

void c64b_parse_gamepad(uni_controller_t* ctl, t_c64b_cport_idx cport_idx)
{
	if(ctl == NULL)
		return;

	// this function only supports gamepads
	if(ctl->klass != UNI_CONTROLLER_CLASS_GAMEPAD)
		return;

	t_c64b_parse_fbak fbak;

	uni_gamepad_t*   gp = &(ctl->gamepad);
	uni_gamepad_t*   gp_old;

	if(cport_idx == CPORT_1)
	{
		gp_old    = &(ctrl_old[1].gamepad);
		cport_idx = swap_ports ? CPORT_2 : CPORT_1;
	}
	else if(cport_idx == CPORT_2)
	{
		gp_old    = &(ctrl_old[2].gamepad);
		cport_idx = swap_ports ? CPORT_1 : CPORT_2;
	}
	else
	{
		logi("parser: gamepad unregistered\n");
		return;
	}

	if (!c64b_gamepad_interesting(gp, gp_old))
		return;

	#ifndef CONFIG_ESP_CONSOLE_NONE
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
		c64b_parse_gamepad_menu(gp, gp_old);

		//--------------------------------------------------------------------//
		// swap ports

		if(c64b_parse_gamepad_swap(gp, gp_old))
		{
			swap_ports = !swap_ports;
			fbak.cport_idx  = cport_idx;
			fbak.swap_ports = swap_ports;

			// here we post only if there are any "news" to report
			xQueueOverwrite(queue_ctl_fbak, (void *)(&fbak));
		}
	}
	else
	{
		//--------------------------------------------------------------------//
		// direct keyboard control
		c64b_parse_gamepad_kbemu(gp, gp_old, cport_idx);

		//--------------------------------------------------------------------//
		// controller ports override characters
		c64b_parse_gamepad_ctrl(gp, gp_old, cport_idx);
	}

	*gp_old = *gp;
}

//----------------------------------------------------------------------------//

void task_c64b_parse(void *arg)
{
	while(1)
	{
		if(xQueueReceive(queue_ctl_data[0], &ctrl_new[0], 0))
			c64b_parse_keyboard(&ctrl_new[0]);

		if(xQueueReceive(queue_ctl_data[1], &ctrl_new[1], 0))
			c64b_parse_gamepad(&ctrl_new[1], CPORT_1);

		if(xQueueReceive(queue_ctl_data[2], &ctrl_new[2], 0))
			c64b_parse_gamepad(&ctrl_new[2], CPORT_2);
	}
}

//----------------------------------------------------------------------------//

void task_c64b_disable_pairing(void * arg)
{
	size_t time = (size_t)arg;
	if(time != 0)
	{
		vTaskDelay(60 * configTICK_RATE_HZ * time);
		uni_bt_enable_pairing_safe(false);
		logi("Pairing disabled\n");
	}
	vTaskDelete(NULL);
}

//----------------------------------------------------------------------------//

void c64b_parse(uni_hid_device_t* d)
{
	if(dev_ptr[0] == d)
		xQueueOverwrite(queue_ctl_data[0], (void *)get_ctl(dev_ptr[0]));
	else if(dev_ptr[1] == d)
		xQueueOverwrite(queue_ctl_data[1], (void *)get_ctl(dev_ptr[1]));
	else if(dev_ptr[2] == d)
		xQueueOverwrite(queue_ctl_data[2], (void *)get_ctl(dev_ptr[2]));

	// it's fine to check here because we can catch feedback on
	// button release events

	t_c64b_parse_fbak fbak;
	while(1)
	{
		if(xQueueReceive(queue_ctl_fbak, &fbak, 0))
		{
			if((fbak.cport_idx == CPORT_2) || (dev_ptr[1] == NULL) || (dev_ptr[2] == NULL))
			{
				c64b_parser_set_gp_seat(CPORT_1, fbak.swap_ports ? 2 : 1);
				c64b_parser_set_gp_seat(CPORT_2, fbak.swap_ports ? 1 : 2);
			}
		}
		else
			break;
	}
}

//----------------------------------------------------------------------------//

void c64b_parser_disconnect(uni_hid_device_t* d)
{
	unsigned int idx;
	if(dev_ptr[0] == d)
		idx = 0;
	else if(dev_ptr[1] == d)
		idx = 1;
	else if(dev_ptr[2] == d)
		idx = 2;
	else
		return;

	logi("Device Disconnected: %d\n", idx);

	dev_ptr[idx] = NULL;

	if(xSemaphoreTake(kbrd_sem_h, portMAX_DELAY) == pdTRUE)
	{
		c64b_keyboard_reset(&keyboard);
		xSemaphoreGive(kbrd_sem_h);
	}
}

//----------------------------------------------------------------------------//

void c64b_parser_init()
{
	queue_ctl_data[0] = xQueueCreate(1, sizeof(uni_controller_t));
	queue_ctl_data[1] = xQueueCreate(1, sizeof(uni_controller_t));
	queue_ctl_data[2] = xQueueCreate(1, sizeof(uni_controller_t));
	queue_ctl_fbak    = xQueueCreate(1, sizeof(t_c64b_parse_fbak));

	kbrd_sem_h = xSemaphoreCreateBinary();
	mcro_sem_h = xSemaphoreCreateBinary();
	feed_sem_h = xSemaphoreCreateBinary();

	xSemaphoreGive(kbrd_sem_h);
	xSemaphoreGive(mcro_sem_h);
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
			"~clr~0 firmware update started!"
			"~ret~0 should take about a minute"
			"~ret~0 text might change color and/or size"
			"~ret~";

		const char update_successful[] =
			"~ret~~clr~"
			"~cmdr-psh~7~cmdr-rel~"
			"~ctrl-psh~0~ctrl-rel~"
			"0 successfully updated firmware!~ret~"
			"~ret~"
			"0 please switch off the computer and~ret~"
			"0 remove the sd-card"
			"~ret~";

		const char update_failed[] =
			"~ret~~clr~"
			"~cmdr-psh~7~cmdr-rel~"
			"~ctrl-psh~0~ctrl-rel~"
			"0 firmware updated failed!~ret~"
			"~ret~"
			"0 please switch off the computer and~ret~"
			"0 remove the sd-card"
			"~ret~";

		vTaskDelay(3000 / portTICK_PERIOD_MS);

		c64b_keyboard_init(&keyboard);
		c64b_keyboard_feed_str(&keyboard, update_started);

		if(c64b_update() == UPDATE_OK)
		{
			vTaskDelay(500 / portTICK_PERIOD_MS);
			c64b_keyboard_init(&keyboard);
			c64b_keyboard_feed_str(&keyboard, update_successful);
		}
		else
		{
			vTaskDelay(500 / portTICK_PERIOD_MS);
			c64b_keyboard_init(&keyboard);
			c64b_keyboard_feed_str(&keyboard, update_failed);
		}

		while(1){}
	}

	logi("parser: Creating parser thread\n");
	xTaskCreatePinnedToCore(task_c64b_parse,
							"ctrl_parser",
							1024*16,
							NULL,
							TASK_PRIO_PARSE,
							NULL,
							CORE_AFFINITY);

	if(scan_time_to_minutes[scan_time] != 0)
	{
		logi("parser: Creating pairing-disabler thread\n");
		xTaskCreatePinnedToCore(task_c64b_disable_pairing,
								"disable_pairing",
								1024*4,
								(void *)scan_time_to_minutes[scan_time],
								TASK_PRIO_PARSE,
								NULL,
								CORE_AFFINITY);
	}

	c64b_keyboard_init(&keyboard);
}
