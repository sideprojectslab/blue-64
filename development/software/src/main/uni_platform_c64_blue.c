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

#include "uni_config.h"
#include "uni_bt.h"
#include "uni_gamepad.h"
#include "uni_hid_device.h"
#include "uni_log.h"
#include "uni_platform.h"
#include "uni_gpio.h"

#include "c64b_keyboard.h"

//
// Globals
//
static int g_delete_keys = 0;

// Hardware pin assignments

#define PIN_COL7     32
#define PIN_COL1     23
#define PIN_COL2     33
#define PIN_COL3     22
#define PIN_COL4     25
#define PIN_COL5     21
#define PIN_COL6     26
#define PIN_COL0     19

#define PIN_ROW0     27
#define PIN_ROW1     18
#define PIN_ROW2     14
#define PIN_ROW7     5
#define PIN_ROW4     12
#define PIN_ROW5     4
#define PIN_ROW6     13
#define PIN_ROW3     2

#define PIN_nRESTORE 15

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
static uni_hid_device_t* ctrl_id[2]  = {NULL, NULL};
static uni_controller_t  ctrl_dat[2] = {{0}, {0}};
static bool              swap_ports  = false;
static SemaphoreHandle_t keyboard_sem_h;

// keyboard string feeds
static char feed_hello_world[] = "qq 22 ~arll~~arll~";


static void task_keyboard_feed(void *arg)
{
	logi("Starting Keyboard Feed\n");
	xSemaphoreTake(keyboard_sem_h, (TickType_t)10);
	c64b_keyboard_feed_string(&keyboard, (char *)arg);
	xSemaphoreGive(keyboard_sem_h);
	vTaskDelete(NULL);
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
	keyboard.pin_col[5] = PIN_COL5;
	keyboard.pin_col[6] = PIN_COL6;
	keyboard.pin_col[7] = PIN_COL7;

	keyboard.pin_row[0] = PIN_ROW0;
	keyboard.pin_row[1] = PIN_ROW1;
	keyboard.pin_row[2] = PIN_ROW2;
	keyboard.pin_row[3] = PIN_ROW3;
	keyboard.pin_row[4] = PIN_ROW4;
	keyboard.pin_row[5] = PIN_ROW5;
	keyboard.pin_row[6] = PIN_ROW6;
	keyboard.pin_row[7] = PIN_ROW7;

	keyboard.pin_nrestore = PIN_nRESTORE;

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

	// inserting the ID in the first free location of the device list
	if(ctrl_id[0] == NULL)
		ctrl_id[0] = d;
	else if(ctrl_id[1] == NULL)
		ctrl_id[1] = d;
}

static void c64_blue_on_device_disconnected(uni_hid_device_t* d) {
	logi("custom: device disconnected: %p\n", d);

	if(ctrl_id[0] == d)
		ctrl_id[0] = NULL;
	else if(ctrl_id[1] == d)
		ctrl_id[1] = NULL;
}

static uni_error_t c64_blue_on_device_ready(uni_hid_device_t* d) {
	logi("custom: device ready: %p\n", d);
	c64_blue_instance_t* ins = get_c64_blue_instance(d);
	ins->gamepad_seat = GAMEPAD_SEAT_A;

	trigger_event_on_gamepad(d);
	return UNI_ERROR_SUCCESS;
}

static void c64_blue_on_controller_data(uni_hid_device_t* d, uni_controller_t* ctl) {
	uni_gamepad_t*   gp;
	uni_gamepad_t*   gp_old;
	unsigned int     idx;
	t_c64b_cport_idx cport_idx;

	if(ctrl_id[0] == d)
	{
		idx = 0;
		cport_idx = swap_ports ? CPORT_1 : CPORT_2;
	}
	else if(ctrl_id[1] == d)
	{
		idx = 1;
		cport_idx = swap_ports ? CPORT_2 : CPORT_1;
	}
	else
		return;

	if (memcmp(&(ctrl_dat[idx]), ctl, sizeof(uni_controller_t)) == 0)
		return;

	// Print device Id before dumping gamepad.
	uni_controller_dump(ctl);

	if (xSemaphoreTake(keyboard_sem_h, (TickType_t)0) == pdTRUE)
	{
		switch (ctl->klass)
		{
			case UNI_CONTROLLER_CLASS_GAMEPAD:
				gp     = &(ctl->gamepad);
				gp_old = &(ctrl_dat[idx].gamepad);

				// shift + run
				if(gp->misc_buttons & BTN_SELECT_MASK)
					c64b_keyboard_char_press(&keyboard, "~run~");
				else
					c64b_keyboard_char_release(&keyboard, "~run~");

				// space
				if(gp->misc_buttons & BTN_START_MASK)
					c64b_keyboard_char_press(&keyboard, " ");
				else
					c64b_keyboard_char_release(&keyboard, " ");

				// easter egg
				if((gp->buttons & BTN_L3_MASK) && !(gp_old->buttons & BTN_L3_MASK))
				{
					xTaskCreatePinnedToCore(task_keyboard_feed,
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
					c64b_keyboard_clear(&keyboard);
				}

				// key parsing finished
				break;
			default:
				break;
		}

		xSemaphoreGive(keyboard_sem_h);
	}

	ctrl_dat[idx] = *ctl;
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
	c64_blue_instance_t* ins = get_c64_blue_instance(d);

	if (d->report_parser.set_rumble != NULL) {
		d->report_parser.set_rumble(d, 0x80 /* value */, 15 /* duration */);
	}

	if (d->report_parser.set_player_leds != NULL) {
		d->report_parser.set_player_leds(d, ins->gamepad_seat);
	}

	if (d->report_parser.set_lightbar_color != NULL) {
		uint8_t red = (ins->gamepad_seat & 0x01) ? 0xff : 0;
		uint8_t green = (ins->gamepad_seat & 0x02) ? 0xff : 0;
		uint8_t blue = (ins->gamepad_seat & 0x04) ? 0xff : 0;
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
