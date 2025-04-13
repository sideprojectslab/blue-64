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

// brutal inclusion of the entire original source, we need this to be able
// to extend its functionality and link against this extended copy rather than
// the original file
#include "bt/uni_bt_allowlist.h"
#include "bt/uni_bt_allowlist.c"


bool uni_bt_allowlist_add_addr_force(bd_addr_t addr) {
	// Don't add duplicate entries
	if (is_address_in_allowlist(addr))
		return false;

	size_t i;
	for (i = 0; i < ARRAY_SIZE(addr_allow_list); i++) {
		if (bd_addr_cmp(addr_allow_list[i], zero_addr) == 0) {
			bd_addr_copy(addr_allow_list[i], addr);
			update_allowlist_to_property();
			return true;
		}
	}

	// no space in the allowlist, if force is active we replace
	// the oldest entry
	if(ARRAY_SIZE(addr_allow_list) != 0) {
		for (i = 1; i < ARRAY_SIZE(addr_allow_list); i++) {
			bd_addr_copy(addr_allow_list[i-1], addr_allow_list[i]);
		}
		bd_addr_copy(addr_allow_list[i-1], addr);
		return true;
	}

	return false;
}

void uni_bt_allowlist_set_enabled_soft(bool enabled) {
	if (enabled != enforced) {
		enforced = enabled;
	}
}
