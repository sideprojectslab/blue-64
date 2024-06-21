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

#include "c64b_macros.h"
#include "c64b_parser.h"
#include "c64b_threadsafe.h"

//----------------------------------------------------------------------------//
// static variables

#define MENU_LVL_MAX 3

typedef unsigned int (*menu)(int);

static menu menu_current_plt;
static menu menu_current_act;
static menu menu_current_ext;

static unsigned int step     = 0;
static unsigned int menu_lvl = 0;
static unsigned int menu_idx[MENU_LVL_MAX] = {0, KB_MAP_SYMBOLIC, 0};
static unsigned int last_len = 0;

static char str_buf[128] = {0};

//----------------------------------------------------------------------------//
//                       FORWARD FUNCTION DECLARATIONS                        //
//----------------------------------------------------------------------------//

unsigned int menu_main_plt(int i);
unsigned int menu_main_act(int i);
unsigned int menu_main_ext(int i);

unsigned int menu_kb_plt(int i);
unsigned int menu_kb_act(int i);
unsigned int menu_kb_ext(int i);

unsigned int menu_key_plt(int i);
unsigned int menu_key_act(int i);
unsigned int menu_key_ext(int i);

//----------------------------------------------------------------------------//
//                              UTILITY MACROS                                //
//----------------------------------------------------------------------------//

unsigned int wrap(int i, unsigned int num_entries)
{
	if(i < 0)
		i = num_entries - 1;
	if(i >= num_entries)
		i = 0;
	return i;
}

#define WRAP(i, e) \
	const unsigned int num_entries = sizeof(e) / sizeof(e[0]); \
	i = wrap(i, num_entries)

//----------------------------------------------------------------------------//
//                                RESTORE MENU                                //
//----------------------------------------------------------------------------//

unsigned int menu_restore_plt(int i)
{
	static const char* entries[] =
	{
		"~home~~ret~0 no",
		"~home~~ret~1 yes"
	};

	WRAP(i, entries);
	keyboard_macro_feed(entries[i]);
	return i;
}

unsigned int menu_restore_act(int i)
{
	if(i == 1)
	{
		c64b_property_reset();
		c64b_property_init();
	}

	menu_current_plt = menu_main_plt;
	menu_current_act = menu_main_act;
	menu_current_ext = menu_main_ext;
	menu_lvl--;
	menu_current_plt(menu_idx[menu_lvl]);
	return 0;
}

unsigned int menu_restore_ext(int i)
{
	menu_current_plt = menu_main_plt;
	menu_current_act = menu_main_act;
	menu_current_ext = menu_main_ext;
	menu_lvl--;
	menu_current_plt(menu_idx[menu_lvl]);
	return 0;
}

//----------------------------------------------------------------------------//
//                               AUTOFIRE MENU                                //
//----------------------------------------------------------------------------//

unsigned int menu_af_plt(int i)
{
	static const char* entries[] =
	{
		"~home~~ret~0 none ",
		"~home~~ret~1 1hz  ",
		"~home~~ret~2 2hz  ",
		"~home~~ret~3 3hz  ",
		"~home~~ret~4 4hz  ",
		"~home~~ret~5 5hz  ",
		"~home~~ret~6 6hz  ",
		"~home~~ret~7 7hz  ",
		"~home~~ret~8 8hz  ",
		"~home~~ret~9 9hz  ",
		"~home~~ret~10 10hz",
	};

	WRAP(i, entries);
	keyboard_macro_feed(entries[i]);
	return i;
}

unsigned int menu_af_act(int i)
{
	if (af_rate != i)
	{
		af_rate = i;
		c64b_property_set_u8(C64B_PROPERTY_KEY_AF_RATE, i);

		af_prd = (TickType_t)portMAX_DELAY;
		if (af_rate != 0)
			af_prd = (1000 / (TickType_t)af_rate) / portTICK_PERIOD_MS;
	}

	menu_current_plt = menu_main_plt;
	menu_current_act = menu_main_act;
	menu_current_ext = menu_main_ext;
	menu_lvl--;
	menu_current_plt(menu_idx[menu_lvl]);
	return 0;
}

unsigned int menu_af_ext(int i)
{
	menu_current_plt = menu_main_plt;
	menu_current_act = menu_main_act;
	menu_current_ext = menu_main_ext;
	menu_lvl--;
	menu_current_plt(menu_idx[menu_lvl]);
	return 0;
}

//----------------------------------------------------------------------------//
//                             KEYBOARD MAP MENU                              //
//----------------------------------------------------------------------------//

unsigned int menu_kb_plt(int i)
{
	static const char* entries[] =
	{
		"~home~~ret~0 symbolic         ",
		"~home~~ret~1 positional (vice)"
	};

	WRAP(i, entries);
	keyboard_macro_feed(entries[i]);
	return i;
}

unsigned int menu_kb_act(int i)
{
	if(i != kb_map)
	{
		kb_map = i;
		c64b_property_set_u8(C64B_PROPERTY_KEY_KB_MAP, kb_map);
	}
	menu_current_plt = menu_main_plt;
	menu_current_act = menu_main_act;
	menu_current_ext = menu_main_ext;
	menu_lvl--;
	menu_current_plt(menu_idx[menu_lvl]);
	return 0;
}

unsigned int menu_kb_ext(int i)
{
	menu_current_plt = menu_main_plt;
	menu_current_act = menu_main_act;
	menu_current_ext = menu_main_ext;
	menu_lvl--;
	menu_current_plt(menu_idx[menu_lvl]);
	return 0;
}

//----------------------------------------------------------------------------//
//                            CONTROLLER MAP MENU                             //
//----------------------------------------------------------------------------//

unsigned int menu_ct_plt(int i)
{
	static const char* entries[] =
	{
		"~home~~ret~2 home button   : ",
		"~home~~ret~3 menu button   : ",
		"~home~~ret~4 left trigger  : ",
		"~home~~ret~5 right trigger : ",
		"~home~~ret~6 left shoulder : ",
		"~home~~ret~7 right shoulder: "
	};

	WRAP(i, entries);

	strcpy(str_buf, "");
	for (unsigned int j = 0; j < last_len; ++j)
		strcat(str_buf, "~del~");

	strcat(str_buf, entries[i]);
	strcat(str_buf, "\"~strip~");
	last_len = strlen(str_buf);
	strcat(str_buf, c64b_keyboard_idx_to_key(ct_map[i]));
	strcat(str_buf, "\"");

	if(KEY_IDS[i].prnt)
	{
		last_len = 2;
	}
	else
	{
		for (unsigned int j = last_len; str_buf[j] != 0; ++j)
			if(str_buf[j] == '~')
				last_len++;

		last_len = strlen(str_buf) - last_len + 1; // including the extra quotes
	}

	keyboard_macro_feed(str_buf);
	return i;
}

unsigned int menu_ct_act(int i)
{
	strcpy(str_buf, " -> ");

	menu_current_plt = menu_key_plt;
	menu_current_act = menu_key_act;
	menu_current_ext = menu_key_ext;
	last_len         = 0;
	menu_lvl++;
	menu_idx[menu_lvl] = C64B_KB_IDX_NONE;

	keyboard_macro_feed(str_buf);

	if(xSemaphoreTake(mcro_sem_h, (TickType_t)portMAX_DELAY) == true)
		menu_current_plt(menu_idx[menu_lvl]);

	return i;
}

unsigned int menu_ct_ext(int i)
{
	menu_current_plt = menu_main_plt;
	menu_current_act = menu_main_act;
	menu_current_ext = menu_main_ext;
	last_len         = 0;
	menu_lvl--;
	menu_current_plt(menu_idx[menu_lvl]);
	return 0;
}

//----------------------------------------------------------------------------//
//                            CONTROLLER MAP MENU                             //
//----------------------------------------------------------------------------//

unsigned int menu_key_plt(int i)
{
	i = wrap(i, NUM_KEYS + 1);

	strcpy(str_buf, "");
	for (unsigned int j = 0; j < last_len; ++j)
		strcat(str_buf, "~del~");

	if(i == C64B_KB_IDX_NONE)
	{
		last_len = 0;
	}
	else
	{
		strcat(str_buf, "\"~strip~");
		last_len = strlen(str_buf);
		strcat(str_buf, c64b_keyboard_idx_to_key(i));
		strcat(str_buf, "\"");

		if(KEY_IDS[i].prnt)
		{
			last_len = 3;
		}
		else
		{
			for (unsigned int j = last_len; str_buf[j] != 0; ++j)
				if(str_buf[j] == '~')
					last_len++;

			last_len = strlen(str_buf) - last_len + 1; // including extra quotes
		}
	}

	logi("printing string: %s\n", str_buf);

	keyboard_macro_feed(str_buf);
	return i;
}

unsigned int menu_key_act(int i)
{
	unsigned int j = menu_idx[menu_lvl-1];
	const t_c64b_key_id* key = NULL;
	key = c64b_keyboard_trace_get(&keyboard);

	if(i == C64B_KB_IDX_NONE)
	{
		if(key != NULL)
		{
			ct_map[j] = c64b_keyboard_key_to_idx(key->str);
		}
		else
		{
			ct_map[j] = C64B_KB_IDX_NONE;
		}
	}
	else
	{
		ct_map[j] = c64b_keyboard_key_to_idx(KEY_IDS[i].str);
	}

	c64b_property_set_u8(ct_map_key[j], ct_map[j]);

	menu_current_plt = menu_ct_plt;
	menu_current_act = menu_ct_act;
	menu_current_ext = menu_ct_ext;
	last_len         = last_len + 10; // deleting the arrow character and the original character
	menu_lvl--;
	menu_current_plt(menu_idx[menu_lvl]);
	return 0;
}

unsigned int menu_key_ext(int i)
{
	menu_current_plt = menu_ct_plt;
	menu_current_act = menu_ct_act;
	menu_current_ext = menu_ct_ext;
	last_len         = last_len + 10; // deleting the arrow character and the original character
	menu_lvl--;
	menu_current_plt(menu_idx[menu_lvl]);
	return 0;
}

//----------------------------------------------------------------------------//
//                                  MAIN MENU                                 //
//----------------------------------------------------------------------------//

unsigned int menu_main_plt(int i)
{
	static const char* entries[] =
	{
		"~clr~0 load tape",
		"~clr~1 load disk",
		"~clr~2 run disk",
		"~clr~3 device info",
		"~clr~4 keyboard mapping",
		"~clr~5 controller mapping (xbox)",
		"~clr~6 autofire rate",
		"~clr~7 restore defaults"
	};

	WRAP(i, entries);
	keyboard_macro_feed(entries[i]);
	return i;
}

unsigned int menu_main_act(int i)
{
	static const char device_info[] =
		"~clr~"
		"0 device info:~ret~"
		"0 blue-64 by side-projects-lab~ret~"
		"0 firmware version: "C64B_FW_VERSION;

	static const char* entries[] =
	{
		"~clr~load~ret~",
		"~clr~load \"$\",8~ret~",
		"~clr~load \"*\",8~ret~",
		device_info,
		":",
		":",
		":",
		"?"
	};

	keyboard_macro_feed(entries[i]);

	switch(i)
	{
		case 4:
			menu_current_plt = menu_kb_plt;
			menu_current_act = menu_kb_act;
			menu_current_ext = menu_kb_ext;
			menu_lvl++;
			menu_idx[menu_lvl] = kb_map;

			if(xSemaphoreTake(mcro_sem_h, (TickType_t)portMAX_DELAY) == true)
				menu_current_plt(menu_idx[menu_lvl]);
			break;

		case 5:
			menu_current_plt = menu_ct_plt;
			menu_current_act = menu_ct_act;
			menu_current_ext = menu_ct_ext;
			last_len         = 0;
			menu_lvl++;
			menu_idx[menu_lvl] = 0;

			if(xSemaphoreTake(mcro_sem_h, (TickType_t)portMAX_DELAY) == true)
				menu_current_plt(menu_idx[menu_lvl]);
			break;

		case 6:
			menu_current_plt = menu_af_plt;
			menu_current_act = menu_af_act;
			menu_current_ext = menu_af_ext;
			menu_lvl++;
			menu_idx[menu_lvl] = af_rate;

			if(xSemaphoreTake(mcro_sem_h, (TickType_t)portMAX_DELAY) == true)
				menu_current_plt(menu_idx[menu_lvl]);
			break;

		case 7:
			menu_current_plt = menu_restore_plt;
			menu_current_act = menu_restore_act;
			menu_current_ext = menu_restore_ext;
			menu_lvl++;
			menu_idx[menu_lvl] = 0;

			if(xSemaphoreTake(mcro_sem_h, (TickType_t)portMAX_DELAY) == true)
				menu_current_plt(menu_idx[menu_lvl]);
			break;
		default:
	}
	return 0;
}

unsigned int menu_main_ext(int i)
{
	keyboard_macro_feed("~clr~");
	menu_idx[0] = 0;
	step = 0;
	return i;
}

//----------------------------------------------------------------------------//
//                                  MENU LOGIC                                //
//----------------------------------------------------------------------------//

static menu menu_current_plt = menu_main_plt;
static menu menu_current_act = menu_main_act;
static menu menu_current_ext = menu_main_ext;


void menu_fwd()
{
	menu_idx[menu_lvl] = menu_current_plt(menu_idx[menu_lvl] + step);
	step = 1;
}


void menu_bwd()
{
	menu_idx[menu_lvl] = menu_current_plt(menu_idx[menu_lvl] - step);
	step = 1;
}

void menu_ext()
{
	menu_current_ext(menu_idx[menu_lvl]);
}

void menu_act()
{
	menu_current_act(menu_idx[menu_lvl]);
}


/*
const char color_change[] =
	"~clr~"
	"poke 53280,1~ret~"
	"poke 53281,5~ret~"
	"~ctrl-psh~6~ctrl-rel~"
	"~clr~new color scheme active";
*/
