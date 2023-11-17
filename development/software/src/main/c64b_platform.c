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
//                  Based on original work by Ricardo Quesada                 //
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

#include "c64b_parser.h"

//----------------------------------------------------------------------------//
// Globals

static int g_delete_keys = 0;

//----------------------------------------------------------------------------//
// Platform Overrides

static void c64b_init(int argc, const char** argv) {

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	logi("c64b: init()\n");
	c64b_parser_init();
}

static void c64b_on_init_complete(void) {
	logi("c64b: on_init_complete()\n");
}

static void c64b_on_device_connected(uni_hid_device_t* d) {
	logi("c64b: device connected: %p\n", d);
}

static void c64b_on_device_disconnected(uni_hid_device_t* d) {
	logi("c64b: device disconnected: %p\n", d);
	c64b_parser_disconnect(d);
}

static uni_error_t c64b_on_device_ready(uni_hid_device_t* d) {
	logi("c64b: device ready: %p\n", d);
	d->report_parser.init_report(d);
	c64b_parser_connect(d);
	return UNI_ERROR_SUCCESS;
}

static void c64b_on_controller_data(uni_hid_device_t* d, uni_controller_t* ctl) {
	c64b_parse(d);
}

static int32_t c64b_get_property(uni_platform_property_t key) {
	logi("c64b: get_property(): %d\n", key);
	if (key != UNI_PLATFORM_PROPERTY_DELETE_STORED_KEYS)
		return -1;
	return g_delete_keys;
}

static void c64b_on_oob_event(uni_platform_oob_event_t event, void* data) {
	logi("c64b: on_device_oob_event(): %d\n", event);

	if (event != UNI_PLATFORM_OOB_GAMEPAD_SYSTEM_BUTTON) {
		logi("c64b_on_device_gamepad_event: unsupported event: 0x%04x\n", event);
		return;
	}

	uni_hid_device_t* d = data;

	if (d == NULL) {
		loge("ERROR: c64b_on_device_gamepad_event: Invalid NULL device\n");
		return;
	}
}

//----------------------------------------------------------------------------//
// Entry Point

struct uni_platform* c64b_platform_create(void) {
	static struct uni_platform plat = {
		.name = "c64b",
		.init = c64b_init,
		.on_init_complete = c64b_on_init_complete,
		.on_device_connected = c64b_on_device_connected,
		.on_device_disconnected = c64b_on_device_disconnected,
		.on_device_ready = c64b_on_device_ready,
		.on_oob_event = c64b_on_oob_event,
		.on_controller_data = c64b_on_controller_data,
		.get_property = c64b_get_property,
	};

	return &plat;
}
