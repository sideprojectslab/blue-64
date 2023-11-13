/****************************************************************************
http://retro.moe/unijoysticle2

Copyright 2019 Ricardo Quesada

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "hid_usage.h"

#include "uni_config.h"
#include "uni_bt.h"
#include "uni_gamepad.h"
#include "uni_keyboard.h"
#include "uni_hid_device.h"
#include "uni_log.h"
#include "uni_platform.h"
#include "uni_gpio.h"

#include "sdkconfig.h"
#include "c64b_keyboard.h"

//
// Globals
//
static int g_delete_keys = 0;

// Hardware pin assignments

#define PIN_KCA0 21
#define PIN_KCA1 19
#define PIN_KCA2 18

#define PIN_COL0 26
#define PIN_COL1 22
#define PIN_COL2 23
#define PIN_COL3 33
#define PIN_COL4 32

#define PIN_KRA0 17
#define PIN_KRA1 16
#define PIN_KRA2 4

#define PIN_ROW0 25
#define PIN_ROW1 14
#define PIN_ROW2 27
#define PIN_ROW3 12
#define PIN_ROW4 13


#define PIN_CTRL  2
#define PIN_SHIFT 0
#define PIN_CMDR  15
#define PIN_KEN   5

#if (CONFIG_BLUEPAD32_UART_OUTPUT_ENABLE == 1)
	#define PIN_nRST  1
#else
	#define PIN_nRST  255
#endif

// Bluetooth controller masks

#define BTN_DPAD_UP_MASK 1
#define BTN_DPAD_DN_MASK 2
#define BTN_DPAD_RR_MASK 4
#define BTN_DPAD_LL_MASK 8

#define BTN_A_MASK       1
#define BTN_B_MASK       2
#define BTN_X_MASK       4
#define BTN_Y_MASK       8
#define BTN_L1_MASK      0x0010
#define BTN_L3_MASK      0x0100
#define BTN_R1_MASK      0x0020
#define BTN_R3_MASK      0x0200

#define BTN_HOME_MASK    1
#define BTN_SELECT_MASK  2
#define BTN_START_MASK   4

// c64-blue "instance"
typedef struct c64_blue_instance_s {
    uni_gamepad_seat_t gamepad_seat;  // which "seat" is being used
} c64_blue_instance_t;

// Declarations
static void trigger_event_on_gamepad(uni_hid_device_t* d);
static c64_blue_instance_t* get_c64_blue_instance(uni_hid_device_t* d);

static t_c64b_keyboard   keyboard;
static uni_hid_device_t* ctrl_id [3] = {NULL, NULL, NULL};
static uni_controller_t  ctrl_dat[3] = {{0}, {0}, {0}};
static bool              swap_ports  = false;
static SemaphoreHandle_t keyboard_sem_h;

// keyboard string feeds
static char feed_hello_world[] = "hello world!";


static void task_keyboard_macro_feed(void *arg)
{
	logi("Starting Keyboard Feed\n");
	xSemaphoreTake(keyboard_sem_h, (TickType_t)10);
	c64b_keyboard_feed_string(&keyboard, (char *)arg);
	xSemaphoreGive(keyboard_sem_h);
	vTaskDelete(NULL);
}


static void process_keyboard(uni_hid_device_t* d)
{
	if(d == NULL)
		return;

	uni_controller_t* ctl = &(d->controller);

	// this function only supports keyboard
	if(ctl->klass != UNI_CONTROLLER_CLASS_KEYBOARD)
		return;

	uni_keyboard_t*   kb = &(ctl->keyboard);
	uni_keyboard_t*   kb_old;
	bool              kb_nop;
	bool              shift;

	if(ctrl_id[0] == d)
	{
		kb_old = &(ctrl_dat[0].keyboard);
	}
	else
	{
		logi("custom: keyboard unregistered\n");
		return;
	}

	if (memcmp(kb_old, kb, sizeof(uni_keyboard_t)) == 0)
		return;

	uni_controller_dump(ctl);

	if (xSemaphoreTake(keyboard_sem_h, (TickType_t)0) == pdTRUE)
	{
		// key modifiers
		for (int i = 0; i < UNI_KEYBOARD_PRESSED_KEYS_MAX; i++)
		{
			const uint8_t key = kb->pressed_keys[i];

			// modifiers
			if((key == HID_USAGE_KB_LEFT_SHIFT ) || (key == HID_USAGE_KB_RIGHT_SHIFT))
			{
				c64b_keyboard_shift_press(&keyboard);
				shift = true;
			}
			else
			{
				c64b_keyboard_shift_release(&keyboard);
				shift = false;
			}

			if((key == HID_USAGE_KB_LEFT_CONTROL) || (key == HID_USAGE_KB_RIGHT_CONTROL))
				c64b_keyboard_ctrl_press(&keyboard);
			else
				c64b_keyboard_ctrl_release(&keyboard);

			if((key == HID_USAGE_KB_LEFT_GUI) || (key == HID_USAGE_KB_RIGHT_GUI))
				c64b_keyboard_cmdr_press(&keyboard);
			else
				c64b_keyboard_cmdr_release(&keyboard);

			// restore key
			if(key == HID_USAGE_KB_ESCAPE)
				c64b_keyboard_restore_press(&keyboard);
			else
				c64b_keyboard_restore_release(&keyboard);
		}

		// regular keys
		for (int i = 0; i < UNI_KEYBOARD_PRESSED_KEYS_MAX; i++)
		{
			const uint8_t key = kb->pressed_keys[i];
			kb_nop = false;

			// regular keys (only one pressed at a time
			switch(key) {
				// basic letters
				case HID_USAGE_KB_A:
					c64b_keyboard_char_press(&keyboard, "a");
					break;
				case HID_USAGE_KB_B:
					c64b_keyboard_char_press(&keyboard, "b");
					break;
				case HID_USAGE_KB_C:
					c64b_keyboard_char_press(&keyboard, "c");
					break;
				case HID_USAGE_KB_D:
					c64b_keyboard_char_press(&keyboard, "d");
					break;
				case HID_USAGE_KB_E:
					c64b_keyboard_char_press(&keyboard, "e");
					break;
				case HID_USAGE_KB_F:
					c64b_keyboard_char_press(&keyboard, "f");
					break;
				case HID_USAGE_KB_G:
					c64b_keyboard_char_press(&keyboard, "g");
					break;
				case HID_USAGE_KB_H:
					c64b_keyboard_char_press(&keyboard, "h");
					break;
				case HID_USAGE_KB_I:
					c64b_keyboard_char_press(&keyboard, "i");
					break;
				case HID_USAGE_KB_J:
					c64b_keyboard_char_press(&keyboard, "j");
					break;
				case HID_USAGE_KB_K:
					c64b_keyboard_char_press(&keyboard, "k");
					break;
				case HID_USAGE_KB_L:
					c64b_keyboard_char_press(&keyboard, "l");
					break;
				case HID_USAGE_KB_M:
					c64b_keyboard_char_press(&keyboard, "m");
					break;
				case HID_USAGE_KB_N:
					c64b_keyboard_char_press(&keyboard, "n");
					break;
				case HID_USAGE_KB_O:
					c64b_keyboard_char_press(&keyboard, "o");
					break;
				case HID_USAGE_KB_P:
					c64b_keyboard_char_press(&keyboard, "p");
					break;
				case HID_USAGE_KB_Q:
					c64b_keyboard_char_press(&keyboard, "q");
					break;
				case HID_USAGE_KB_R:
					c64b_keyboard_char_press(&keyboard, "r");
					break;
				case HID_USAGE_KB_S:
					c64b_keyboard_char_press(&keyboard, "s");
					break;
				case HID_USAGE_KB_T:
					c64b_keyboard_char_press(&keyboard, "t");
					break;
				case HID_USAGE_KB_U:
					c64b_keyboard_char_press(&keyboard, "u");
					break;
				case HID_USAGE_KB_V:
					c64b_keyboard_char_press(&keyboard, "v");
					break;
				case HID_USAGE_KB_W:
					c64b_keyboard_char_press(&keyboard, "w");
					break;
				case HID_USAGE_KB_X:
					c64b_keyboard_char_press(&keyboard, "x");
					break;
				case HID_USAGE_KB_Y:
					c64b_keyboard_char_press(&keyboard, "y");
					break;
				case HID_USAGE_KB_Z:
					c64b_keyboard_char_press(&keyboard, "z");
					break;

				// numbers

				// other ascii keys
				case HID_USAGE_KB_SPACEBAR:
					c64b_keyboard_char_press(&keyboard, " ");
					break;
				case HID_USAGE_KB_RETURN:
					c64b_keyboard_char_press(&keyboard, "~ret~");
					break;
				case HID_USAGE_KB_BACKSPACE:
					c64b_keyboard_char_press(&keyboard, "~del~");
					break;
				case HID_USAGE_KB_DELETE:
					c64b_keyboard_char_press(&keyboard, "~clr~");
					break;

				// arrows
				case HID_USAGE_KB_LEFT_ARROW:
					c64b_keyboard_char_press(&keyboard, "~ll~");
					break;
				case HID_USAGE_KB_RIGHT_ARROW:
					c64b_keyboard_char_press(&keyboard, "~rr~");
					break;
				case HID_USAGE_KB_UP_ARROW:
					c64b_keyboard_char_press(&keyboard, "~up~");
					break;
				case HID_USAGE_KB_DOWN_ARROW:
					c64b_keyboard_char_press(&keyboard, "~dn~");
					break;

				default:
					kb_nop = true;
					break;
			}

			// only the first key pressed (other than the modifiers) is registered
			if(!kb_nop)
				break;
		}

		if(kb_nop)
			c64b_keyboard_keys_release(&keyboard, !shift);

		xSemaphoreGive(keyboard_sem_h);
	}
	*kb_old = *kb;
}


static void process_gamepad(uni_hid_device_t* d)
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
	bool              kb_nop = true;

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
		logi("custom: gamepad unregistered\n");
		return;
	}

	if (memcmp(gp_old, gp, sizeof(uni_gamepad_t)) == 0)
		return;

//	uni_controller_dump(ctl);
	logi("custom: processing gamepad event\n");

	if (xSemaphoreTake(keyboard_sem_h, (TickType_t)0) == pdTRUE)
	{
		// shift + run
		if(gp->misc_buttons & BTN_SELECT_MASK)
		{
			kb_nop = false;
			c64b_keyboard_char_press(&keyboard, "~run~");
		}

		// space
		if(gp->misc_buttons & BTN_START_MASK)
		{
			kb_nop = false;
			c64b_keyboard_char_press(&keyboard, " ");
		}

		// keyboard macros
		if((gp->buttons & BTN_L3_MASK) && !(gp_old->buttons & BTN_L3_MASK))
		{
			xTaskCreatePinnedToCore(task_keyboard_macro_feed,
			                        "keyboard-feed",
			                        4096,
			                        feed_hello_world,
			                        3,
			                        NULL,
			                        tskNO_AFFINITY);
		}

		// controller ports override characters
		if((gp->dpad & BTN_DPAD_UP_MASK) | (gp->buttons & BTN_A_MASK))
			c64b_keyboard_cport_press(&keyboard, CPORT_UP, cport_idx);
		else
			c64b_keyboard_cport_release(&keyboard, CPORT_UP, cport_idx);

		if((gp->dpad & BTN_DPAD_DN_MASK) || gp->throttle != 0)
			c64b_keyboard_cport_press(&keyboard, CPORT_DN, cport_idx);
		else
			c64b_keyboard_cport_release(&keyboard, CPORT_DN, cport_idx);

		if(gp->dpad & BTN_DPAD_RR_MASK)
			c64b_keyboard_cport_press(&keyboard, CPORT_RR, cport_idx);
		else
			c64b_keyboard_cport_release(&keyboard, CPORT_RR, cport_idx);

		if(gp->dpad & BTN_DPAD_LL_MASK)
			c64b_keyboard_cport_press(&keyboard, CPORT_LL, cport_idx);
		else
			c64b_keyboard_cport_release(&keyboard, CPORT_LL, cport_idx);

		if(gp->buttons & BTN_B_MASK)
			c64b_keyboard_cport_press(&keyboard, CPORT_FF, cport_idx);
		else
			c64b_keyboard_cport_release(&keyboard, CPORT_FF, cport_idx);

		// swap ports
		if((gp->misc_buttons & BTN_HOME_MASK) && !(gp_old->misc_buttons & BTN_HOME_MASK))
		{
			logi("Swapping Ports\n");
			swap_ports ^= true;
			c64b_keyboard_reset(&keyboard);
			trigger_event_on_gamepad(d);
		}

		if(kb_nop)
			c64b_keyboard_keys_release(&keyboard, true);

		xSemaphoreGive(keyboard_sem_h);
	}
	*gp_old = *gp;
}


//
// Platform Overrides
//
static void c64_blue_init(int argc, const char** argv) {

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	logi("custom: init()\n");

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

	keyboard.pin_ken   = PIN_KEN;
	keyboard.pin_nrst  = PIN_nRST;
	keyboard.pin_ctrl  = PIN_CTRL;
	keyboard.pin_shift = PIN_SHIFT;
	keyboard.pin_cmdr  = PIN_CMDR;

	keyboard.feed_press_ms = 25;
	keyboard.feed_clear_ms = 25;

	c64b_keyboard_init(&keyboard);

	keyboard_sem_h = xSemaphoreCreateMutex();

#if 0
	uni_gamepad_mappings_t mappings = GAMEPAD_DEFAULT_MAPPINGS;

	// Inverted axis with inverted Y in RY.
	mappings.axis_x = UNI_GAMEPAD_MAPPINGS_AXIS_RX;
	mappings.axis_y = UNI_GAMEPAD_MAPPINGS_AXIS_RY;
	mappings.axis_ry_inverted = true;
	mappings.axis_rx = UNI_GAMEPAD_MAPPINGS_AXIS_X;
	mappings.axis_ry = UNI_GAMEPAD_MAPPINGS_AXIS_Y;

	// Invert A & B
	mappings.button_a = UNI_GAMEPAD_MAPPINGS_BUTTON_B;
	mappings.button_b = UNI_GAMEPAD_MAPPINGS_BUTTON_A;

	uni_gamepad_set_mappings(&mappings);
#endif
}

static void c64_blue_on_init_complete(void) {
	logi("custom: on_init_complete()\n");
}

static void c64_blue_on_device_connected(uni_hid_device_t* d) {
	logi("custom: device connected: %p\n", d);
}

static void c64_blue_on_device_disconnected(uni_hid_device_t* d) {
	logi("custom: device disconnected: %p\n", d);
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

static uni_error_t c64_blue_on_device_ready(uni_hid_device_t* d) {
	logi("custom: device ready: %p\n", d);

	d->report_parser.init_report(d);

	// keyboard ID always sits on index 0
	if(d->controller.klass == UNI_CONTROLLER_CLASS_KEYBOARD)
	{
		logi("custom: keyboard connected: %p\n", d);
		ctrl_id[0] = NULL;
	}
	// inserting controller ID in the first free location after 0
	else if(d->controller.klass == UNI_CONTROLLER_CLASS_GAMEPAD)
	{
		logi("custom: gamepad connected: %p\n", d);
		if(ctrl_id[1] == NULL)
			ctrl_id[1] = d;
		else if(ctrl_id[2] == NULL)
			ctrl_id[2] = d;
	}
	else
	{
		logi("custom: device class not supported: %d\n", d->controller.klass);
	}

	c64_blue_instance_t* ins = get_c64_blue_instance(d);
	ins->gamepad_seat = GAMEPAD_SEAT_A;

	trigger_event_on_gamepad(d);
	return UNI_ERROR_SUCCESS;
}

static void c64_blue_on_controller_data(uni_hid_device_t* d, uni_controller_t* ctl) {

	if (xSemaphoreTake(keyboard_sem_h, (TickType_t)0) == pdTRUE)
	{
		if(ctrl_id[0] == d)
			process_keyboard(d);
		else
			process_gamepad(d);

		xSemaphoreGive(keyboard_sem_h);
	}
}


static int32_t c64_blue_get_property(uni_platform_property_t key) {
	logi("custom: get_property(): %d\n", key);
	if (key != UNI_PLATFORM_PROPERTY_DELETE_STORED_KEYS)
		return -1;
	return g_delete_keys;
}

static void c64_blue_on_oob_event(uni_platform_oob_event_t event, void* data) {
	logi("custom: on_device_oob_event(): %d\n", event);

	if (event != UNI_PLATFORM_OOB_GAMEPAD_SYSTEM_BUTTON) {
		logi("c64_blue_on_device_gamepad_event: unsupported event: 0x%04x\n", event);
		return;
	}

	uni_hid_device_t* d = data;

	if (d == NULL) {
		loge("ERROR: c64_blue_on_device_gamepad_event: Invalid NULL device\n");
		return;
	}

	c64_blue_instance_t* ins = get_c64_blue_instance(d);
	ins->gamepad_seat = ins->gamepad_seat == GAMEPAD_SEAT_A ? GAMEPAD_SEAT_B : GAMEPAD_SEAT_A;

	trigger_event_on_gamepad(d);
}

//
// Helpers
//
static c64_blue_instance_t* get_c64_blue_instance(uni_hid_device_t* d) {
	return (c64_blue_instance_t*)&d->platform_data[0];
}

static void trigger_event_on_gamepad(uni_hid_device_t* d) {

	unsigned int seat;
	if(ctrl_id[1] == d)
		seat = GAMEPAD_SEAT_A;
	else if(ctrl_id[2] == d)
		seat = GAMEPAD_SEAT_B;
	else
		return;

	if (d->report_parser.set_rumble != NULL) {
		d->report_parser.set_rumble(d, 0x80 /* value */, 15 /* duration */);
	}

	if (d->report_parser.set_player_leds != NULL) {
		logi("setting leds for seat %d\n", seat);
		d->report_parser.set_player_leds(d, seat);
	}

	if (d->report_parser.set_lightbar_color != NULL) {
		uint8_t red   = (seat & 0x01) ? 0xff : 0;
		uint8_t green = (seat & 0x02) ? 0xff : 0;
		uint8_t blue  = (seat & 0x04) ? 0xff : 0;
		d->report_parser.set_lightbar_color(d, red, green, blue);
	}
}

//
// Entry Point
//
struct uni_platform* uni_platform_c64_blue_create(void) {
	static struct uni_platform plat = {
		.name = "c64_blue",
		.init = c64_blue_init,
		.on_init_complete = c64_blue_on_init_complete,
		.on_device_connected = c64_blue_on_device_connected,
		.on_device_disconnected = c64_blue_on_device_disconnected,
		.on_device_ready = c64_blue_on_device_ready,
		.on_oob_event = c64_blue_on_oob_event,
		.on_controller_data = c64_blue_on_controller_data,
		.get_property = c64_blue_get_property,
	};

	return &plat;
}
