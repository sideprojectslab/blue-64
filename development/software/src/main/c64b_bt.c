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

#include "bt/uni_bt.h"
#include "bt/uni_bt_allowlist.h"
#include "uni_hid_device.h"
#include "uni_log.h"

extern bool uni_bt_allowlist_add_addr_force(bd_addr_t addr);
extern void uni_bt_allowlist_set_enabled_soft(bool enabled);

static btstack_context_callback_registration_t cmd_callback_registration;

enum {
	CMD_PAIR_ENABLE,
	CMD_PAIR_DISABLE,
	CMD_FORGET_DEVICES
};


static void enable_pairing(bool enabled) {
	if(uni_bt_allowlist_is_enabled() != !enabled) {
		uni_bt_allowlist_set_enabled_soft(!enabled);
	}
}


static void c64b_cmd_callback(void* context) {
	uni_hid_device_t* d;
	unsigned long ctx = (unsigned long)context;
	uint16_t cmd = ctx & 0xffff;
	uint16_t args = (ctx >> 16) & 0xffff;

	switch (cmd) {
		case CMD_PAIR_ENABLE:
			enable_pairing(true);
			break;
		case CMD_PAIR_DISABLE:
			enable_pairing(false);
			break;
		case CMD_FORGET_DEVICES:
			uni_bt_allowlist_remove_all();
			break;
		default:
			loge("Unknown command: %#x\n", cmd);
			break;
	}
}


void uni_bt_enable_pairing_safe(bool enabled) {
	cmd_callback_registration.callback = &c64b_cmd_callback;
	cmd_callback_registration.context = (void*)(enabled ? (intptr_t)CMD_PAIR_ENABLE : (intptr_t)CMD_PAIR_DISABLE);
	btstack_run_loop_execute_on_main_thread(&cmd_callback_registration);
}


void uni_bt_forget_devices_safe() {
	cmd_callback_registration.callback = &c64b_cmd_callback;
	cmd_callback_registration.context = (void*)((intptr_t)CMD_FORGET_DEVICES);
	btstack_run_loop_execute_on_main_thread(&cmd_callback_registration);
}
