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

#include <uni.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "c64b_keyboard.h"
#include "driver/gpio.h"

#define ESC_LEN_MAX 6
#define SHIFT_COL   1
#define SHIFT_ROW   7
#define NUM_KEYS    116

// the "~" character is used to "escape" complex characters.
// It is not present on the c64 keyboard and thus is free to use

const t_c64b_key_id KEY_IDS[NUM_KEYS] =
{
	{" "       , 7, 4, false, false, true },

	{"~f1~"    , 0, 4, false, false, false},
	{"~f2~"    , 0, 4, true , false, false},
	{"~f3~"    , 0, 5, false, false, false},
	{"~f4~"    , 0, 5, true , false, false},
	{"~f5~"    , 0, 6, false, false, false},
	{"~f6~"    , 0, 6, true , false, false},
	{"~f7~"    , 0, 3, false, false, false},
	{"~f8~"    , 0, 3, true , false, false},

	{"~up~"    , 0, 7, true , false, false},
	{"~dn~"    , 0, 7, false, false, false},
	{"~rr~"    , 0, 2, false, false, false},
	{"~ll~"    , 0, 2, true , false, false},

	{"~lsh~"   , 1, 7, false, false, false},
	{"~rsh~"   , 6, 4, false, false, false},
	{"~cmdr~"  , 7, 5, false, false, false},
	{"~ctrl~"  , 7, 2, false, false, false},
	{"~run~"   , 7, 7, true , false, false},
	{"~stop~"  , 7, 7, false, false, false},
	{"~ret~"   , 0, 1, false, false, false},
	{"~del~"   , 0, 0, false, false, false},
	{"~clr~"   , 6, 3, true , false, false},
	{"~inst~"  , 0, 0, true , false, false},
	{"~home~"  , 6, 3, false, false, false},

	{"0"       , 4, 3, false, false, true },
	{"1"       , 7, 0, false, false, true },
	{"2"       , 7, 3, false, false, true },
	{"3"       , 1, 0, false, false, true },
	{"4"       , 1, 3, false, false, true },
	{"5"       , 2, 0, false, false, true },
	{"6"       , 2, 3, false, false, true },
	{"7"       , 3, 0, false, false, true },
	{"8"       , 3, 3, false, false, true },
	{"9"       , 4, 0, false, false, true },

	{"/"       , 6, 7, false, false, true },
	{"="       , 6, 5, false, false, true },
	{";"       , 6, 2, false, false, true },
	{"*"       , 6, 1, false, false, true },
	{"^"       , 6, 0, false, false, true }, // this replaced the pound symbol
	{","       , 5, 7, false, false, true },
	{"@"       , 5, 6, false, false, true },
	{":"       , 5, 5, false, false, true },
	{"."       , 5, 4, false, false, true },
	{"-"       , 5, 3, false, false, true },
	{"+"       , 5, 0, false, false, true },
	{"~pi~"    , 6, 6, true , false, true },
	{"\""      , 7, 3, true , false, true },
	{"!"       , 7, 0, true , false, true },
	{"?"       , 6, 7, true , false, true },
	{"]"       , 6, 2, true , false, true },
	{"["       , 5, 5, true , false, true },
	{"<"       , 5, 7, true , false, true },
	{">"       , 5, 4, true , false, true },
	{")"       , 4, 0, true , false, true },
	{"("       , 3, 3, true , false, true },
	{"'"       , 3, 0, true , false, true },
	{"&"       , 2, 3, true , false, true },
	{"%"       , 2, 0, true , false, true },
	{"$"       , 1, 3, true , false, true },
	{"#"       , 1, 0, true , false, true },
	{"~arll~"  , 7, 1, false, false, true },
	{"~arup~"  , 6, 6, false, false, true },

	{"a"       , 1, 2, false, false, true }, {"A", 1, 2, true, false, true},
	{"b"       , 3, 4, false, false, true }, {"B", 3, 4, true, false, true},
	{"c"       , 2, 4, false, false, true }, {"C", 2, 4, true, false, true},
	{"d"       , 2, 2, false, false, true }, {"D", 2, 2, true, false, true},
	{"e"       , 1, 6, false, false, true }, {"E", 1, 6, true, false, true},
	{"f"       , 2, 5, false, false, true }, {"F", 2, 5, true, false, true},
	{"g"       , 3, 2, false, false, true }, {"G", 3, 2, true, false, true},
	{"h"       , 3, 5, false, false, true }, {"H", 3, 5, true, false, true},
	{"i"       , 4, 1, false, false, true }, {"I", 4, 1, true, false, true},
	{"j"       , 4, 2, false, false, true }, {"J", 4, 2, true, false, true},
	{"k"       , 4, 5, false, false, true }, {"K", 4, 5, true, false, true},
	{"l"       , 5, 2, false, false, true }, {"L", 5, 2, true, false, true},
	{"m"       , 4, 4, false, false, true }, {"M", 4, 4, true, false, true},
	{"n"       , 4, 7, false, false, true }, {"N", 4, 7, true, false, true},
	{"o"       , 4, 6, false, false, true }, {"O", 4, 6, true, false, true},
	{"p"       , 5, 1, false, false, true }, {"P", 5, 1, true, false, true},
	{"q"       , 7, 6, false, false, true }, {"Q", 7, 6, true, false, true},
	{"r"       , 2, 1, false, false, true }, {"R", 2, 1, true, false, true},
	{"s"       , 1, 5, false, false, true }, {"S", 1, 5, true, false, true},
	{"t"       , 2, 6, false, false, true }, {"T", 2, 6, true, false, true},
	{"u"       , 3, 6, false, false, true }, {"U", 3, 6, true, false, true},
	{"v"       , 3, 7, false, false, true }, {"V", 3, 7, true, false, true},
	{"w"       , 1, 1, false, false, true }, {"W", 1, 1, true, false, true},
	{"x"       , 2, 7, false, false, true }, {"X", 2, 7, true, false, true},
	{"y"       , 3, 1, false, false, true }, {"Y", 3, 1, true, false, true},
	{"z"       , 1, 4, false, false, true }, {"Z", 1, 4, true, false, true},

	// additional dummy keys with exceptions
	{"~rest~"  , 0, 0, false, false, false},
	{""        , 0, 0, false, false, true}
};

//const unsigned int NUM_KEYS = sizeof(KEY_IDS) / sizeof(KEY_IDS[0]);

typedef enum
{
	NONE = -1,
	CTRL_PSH,
	CTRL_REL,
	CMDR_PSH,
	CMDR_REL,
	SHFT_PSH,
	SHFT_REL,
	STRIP,
	MOD_EVT_NUM
} t_c64b_mod_evt;


static const char* MOD_EVT_IDS[] =
{
	"~ctrl-psh~",
	"~ctrl-rel~",
	"~cmdr-psh~",
	"~cmdr-rel~",
	"~shft-psh~",
	"~shft-rel~",
	"~strip~"
};

//----------------------------------------------------------------------------//
const t_c64b_key_id * c64b_keyboard_idx_to_key(unsigned int i)
{
	if(i < sizeof(KEY_IDS) / sizeof(KEY_IDS[0]))
		return &KEY_IDS[i];
	return NULL;
}

//----------------------------------------------------------------------------//
unsigned int c64b_keyboard_char_to_idx(const char* s)
{
	if(s == NULL)
		return C64B_KB_IDX_NONE;

	for(unsigned int i = 0; i < sizeof(KEY_IDS) / sizeof(KEY_IDS[0]); ++i)
	{
		unsigned int j = 0;
		while(KEY_IDS[i].str[j] != 0)
		{
			if(KEY_IDS[i].str[j] != s[j])
				break;
			++j;
		}

		if(KEY_IDS[i].str[j] == 0)
			return i;
	}
	return C64B_KB_IDX_NONE;
}

//----------------------------------------------------------------------------//
const char *c64b_keyboard_idx_to_char(unsigned int i)
{
	if(i == C64B_KB_IDX_NONE)
		return "";
	if(i == C64B_KB_IDX_REST)
		return "rest";

	const t_c64b_key_id * key = c64b_keyboard_idx_to_key(i);
	return key->str;
}

//----------------------------------------------------------------------------//

static const t_c64b_key_id * c64b_keyboard_char_to_key(const char* s)
{
	if(s == NULL)
		return NULL;

	int i = c64b_keyboard_char_to_idx(s);
	if(i != C64B_KB_IDX_NONE)
		return &(KEY_IDS[i]);
	return NULL;
}

//----------------------------------------------------------------------------//

static t_c64b_mod_evt c64b_keyboard_char_to_mod(const char* s)
{
	if(s == NULL)
		return NONE;

	for(unsigned int i = 0; i < MOD_EVT_NUM; ++i)
	{
		unsigned int j = 0;
		while(MOD_EVT_IDS[i][j] != 0)
		{
			if(MOD_EVT_IDS[i][j] != s[j])
				break;
			++j;
		}

		if(MOD_EVT_IDS[i][j] == 0)
			return i;
	}
	return NONE;
}

//----------------------------------------------------------------------------//

bool c64b_keyboard_set_mux(t_c64b_keyboard *h, unsigned int col, unsigned int row)
{
	if(h == NULL)
		return false;

	col = h->col_perm[col];
	row = h->row_perm[row];

	gpio_set_level(h->pin_kca[0], (col >> 0) & 1);
	gpio_set_level(h->pin_kca[1], (col >> 1) & 1);
	gpio_set_level(h->pin_kca[2], (col >> 2) & 1);

	gpio_set_level(h->pin_kra[0], (row >> 0) & 1);
	gpio_set_level(h->pin_kra[1], (row >> 1) & 1);
	gpio_set_level(h->pin_kra[2], (row >> 2) & 1);

	gpio_set_level(h->pin_kben, 0);

	return true;
}

//----------------------------------------------------------------------------//

bool c64b_keyboard_clr_mux(t_c64b_keyboard *h)
{
	if(h == NULL)
		return false;

	gpio_set_level(h->pin_kben, 1);
	return true;
}

//----------------------------------------------------------------------------//

void c64b_keyboard_keys_rel(t_c64b_keyboard *h, bool rel_shft)
{
	if(h == NULL)
		return;

	c64b_keyboard_clr_mux(h);
	c64b_keyboard_rest_rel(h);

	if(rel_shft)
		gpio_set_level(h->pin_shft, 0);

	vTaskDelay(h->feed_rel_ms / portTICK_PERIOD_MS);
}

//----------------------------------------------------------------------------//

void c64b_keyboard_mods_rel(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;

	c64b_keyboard_ctrl_rel(h);
	c64b_keyboard_cmdr_rel(h);
	c64b_keyboard_shft_rel(h);
}

//----------------------------------------------------------------------------//

bool c64b_keyboard_key_psh(t_c64b_keyboard *h, const t_c64b_key_id *k)
{
	if((h == NULL) || (k == NULL) || (k == &(KEY_IDS[C64B_KB_IDX_NONE])))
		return false;

	h->trace_key = k;

	if(k == &(KEY_IDS[C64B_KB_IDX_REST]))
	{
		c64b_keyboard_rest_psh(h);
	}
	else
	{
		if(k->shft)
			c64b_keyboard_shft_psh(h);

		c64b_keyboard_set_mux(h, k->col, k->row);
	}

	vTaskDelay(h->feed_psh_ms / portTICK_PERIOD_MS);
	return true;
}

//----------------------------------------------------------------------------//

bool c64b_keyboard_key_rel(t_c64b_keyboard *h, const t_c64b_key_id *k)
{
	if((h == NULL) || (k == NULL))
		return false;

	c64b_keyboard_clr_mux(h);

	if(k->shft)
		c64b_keyboard_shft_rel(h);

	vTaskDelay(h->feed_rel_ms / portTICK_PERIOD_MS);

	return true;
}

//----------------------------------------------------------------------------//

void c64b_keyboard_cport_psh(t_c64b_keyboard *h, t_c64b_cport_key key, t_c64b_cport_idx idx)
{
	if(h == NULL)
		return;

	unsigned int *pins;
	if(idx == CPORT_1)
		pins = h->pin_row;
	else
		pins = h->pin_col;

	switch(key)
	{
		case CPORT_UP:
			gpio_set_level(pins[0], 1);
			break;
		case CPORT_LL:
			gpio_set_level(pins[2], 1);
			break;
		case CPORT_RR:
			gpio_set_level(pins[3], 1);
			break;
		case CPORT_DN:
			gpio_set_level(pins[1], 1);
			break;
		case CPORT_FF:
			gpio_set_level(pins[4], 1);
			break;
	}
}

//----------------------------------------------------------------------------//

void c64b_keyboard_cport_rel(t_c64b_keyboard *h, t_c64b_cport_key key, t_c64b_cport_idx idx)
{
	if(h == NULL)
		return;

	unsigned int *pins;
	if(idx == CPORT_1)
		pins = h->pin_row;
	else
		pins = h->pin_col;

	switch(key)
	{
		case CPORT_UP:
			gpio_set_level(pins[0], 0);
			break;
		case CPORT_LL:
			gpio_set_level(pins[2], 0);
			break;
		case CPORT_RR:
			gpio_set_level(pins[3], 0);
			break;
		case CPORT_DN:
			gpio_set_level(pins[1], 0);
			break;
		case CPORT_FF:
			gpio_set_level(pins[4], 0);
			break;
	}
}

//----------------------------------------------------------------------------//

void c64b_keyboard_rest_psh(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;

	if (h->pin_nrst != 255)
		gpio_set_level(h->pin_nrst, 1);
}

//----------------------------------------------------------------------------//

void c64b_keyboard_rest_rel(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;

	if (h->pin_nrst != 255)
		gpio_set_level(h->pin_nrst, 0);
}

//----------------------------------------------------------------------------//

void c64b_keyboard_ctrl_psh(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;
	gpio_set_level(h->pin_ctrl, 1);
}

//----------------------------------------------------------------------------//

void c64b_keyboard_ctrl_rel(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;
	gpio_set_level(h->pin_ctrl, 0);
}

//----------------------------------------------------------------------------//

void c64b_keyboard_shft_psh(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;
	gpio_set_level(h->pin_shft, 1);
}

//----------------------------------------------------------------------------//

void c64b_keyboard_shft_rel(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;
	gpio_set_level(h->pin_shft, 0);
}

//----------------------------------------------------------------------------//

void c64b_keyboard_cmdr_psh(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;
	gpio_set_level(h->pin_cmdr, 1);
}

//----------------------------------------------------------------------------//

void c64b_keyboard_cmdr_rel(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;
	gpio_set_level(h->pin_cmdr, 0);
}

//----------------------------------------------------------------------------//

bool c64b_keyboard_char_psh(t_c64b_keyboard *h, const char *s)
{
	if((h == NULL) || (s == NULL))
		return false;

	const t_c64b_key_id *key  = NULL;

	key = c64b_keyboard_char_to_key(s);

	if(key == NULL)
		return false;

	c64b_keyboard_key_psh(h, key);
	return true;
}

//----------------------------------------------------------------------------//

bool c64b_keyboard_char_rel(t_c64b_keyboard *h, const char *s)
{
	if((h == NULL) || (s == NULL))
		return false;

	const t_c64b_key_id *key  = NULL;

	key = c64b_keyboard_char_to_key(s);

	if(key == NULL)
		return false;

	c64b_keyboard_key_rel(h, key);
	return true;
}

//----------------------------------------------------------------------------//

bool c64b_keyboard_feed_str(t_c64b_keyboard *h, char* s)
{
	static char stripped[] = " ";

	if((h == NULL) || (s == NULL))
		return false;

	const char          *head = s;
	const t_c64b_key_id *key  = NULL;
	t_c64b_mod_evt       mod  = NONE;

	bool shft                 = false;
	bool strip                = false;
	bool strip_first          = false;
	bool error                = false;

	while(*head != 0)
	{
		mod = c64b_keyboard_char_to_mod(head);
		key = c64b_keyboard_char_to_key(head);

		if(mod != NONE)
		{
			logi("activating modifier %s\n", MOD_EVT_IDS[mod]);
			switch(mod)
			{
				case NONE:
					break;
				case CTRL_PSH:
					c64b_keyboard_ctrl_psh(h);
					break;
				case CTRL_REL:
					c64b_keyboard_ctrl_rel(h);
					break;
				case CMDR_PSH:
					c64b_keyboard_cmdr_psh(h);
					break;
				case CMDR_REL:
					c64b_keyboard_cmdr_rel(h);
					break;
				case SHFT_PSH:
					c64b_keyboard_shft_psh(h);
					shft = true;
					break;
				case SHFT_REL:
					c64b_keyboard_shft_rel(h);
					shft = false;
					break;
				case STRIP:
					strip       = true;
					strip_first = true;
					break;
				default:
					break;
			}
			head += strlen(MOD_EVT_IDS[mod]);
		}
		else if(key != NULL)
		{
			// strip is ignored on printable characters
			if(key->prnt && strip_first)
			{
				strip       = false;
				strip_first = false;
			}

			if(strip)
			{
				if(*head != '~')
				{
					stripped[0] = *head;
					c64b_keyboard_char_psh(h, stripped);
					c64b_keyboard_keys_rel(h, !shft);

					if(strip_first)
						strip = false;
				}

				head        += 1;
				strip_first = false;

				if(*head == '~')
				{
					head  += 1;
					strip = false;
				}
			}
			else
			{
				c64b_keyboard_key_psh(h, key);
				c64b_keyboard_keys_rel(h, !shft);
				head += strlen(key->str);
			}
		}
		else
		{
			error = true;
			break;
		}
	}

	c64b_keyboard_keys_rel(h, true);
	c64b_keyboard_mods_rel(h);
	return !error;
}

//----------------------------------------------------------------------------//

bool c64b_keyboard_feed_prg(t_c64b_keyboard *h, char** p, uint32_t nlines)
{
	if((h == NULL) || (p == NULL))
		return false;

	bool ok = true;

	for(uint32_t i = 0; i < nlines; ++i)
	{
		ok &= c64b_keyboard_feed_str(h, p[i]);
		ok &= c64b_keyboard_feed_str(h, "~ret~");
	}
	return ok;
}

//----------------------------------------------------------------------------//

void c64b_keyboard_reset(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;

	for(unsigned int i = 0; i < C64B_CTL_BITS; ++i)
	{
		gpio_set_level(h->pin_col[i], 0);
		gpio_set_level(h->pin_row[i], 0);
	}

	gpio_set_level(h->pin_kben, 0);
	gpio_set_level(h->pin_ctrl, 0);
	gpio_set_level(h->pin_shft, 0);
	gpio_set_level(h->pin_cmdr, 0);

	if (h->pin_nrst != 255)
		gpio_set_level(h->pin_nrst, 0);

	c64b_keyboard_keys_rel(h, true);
	c64b_keyboard_mods_rel(h);
	c64b_keyboard_rest_rel(h);
}

//----------------------------------------------------------------------------//

void c64b_keyboard_trace_reset(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;
	h->trace_key = &(KEY_IDS[C64B_KB_IDX_NONE]);
}

const t_c64b_key_id* c64b_keyboard_trace_get(t_c64b_keyboard *h)
{
	if(h == NULL)
		return NULL;
	return h->trace_key;
}

//----------------------------------------------------------------------------//

void c64b_keyboard_init(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;

	logi("Clearing Keyboard\n");
	c64b_keyboard_reset(h);

	logi("Initialising Controller Pins\n");
	for(unsigned int i = 0; i < C64B_CTL_BITS; ++i)
	{
		gpio_reset_pin    (h->pin_col[i]);
		gpio_reset_pin    (h->pin_row[i]);
		gpio_set_direction(h->pin_col[i], GPIO_MODE_OUTPUT);
		gpio_set_direction(h->pin_row[i], GPIO_MODE_OUTPUT);
	}

	logi("Initialising Keyboard Pins\n");
	for(unsigned int i = 0; i < C64B_KKA_BITS; ++i)
	{
		gpio_reset_pin    (h->pin_kca[i]);
		gpio_reset_pin    (h->pin_kra[i]);
		gpio_set_direction(h->pin_kca[i], GPIO_MODE_OUTPUT);
		gpio_set_direction(h->pin_kra[i], GPIO_MODE_OUTPUT);
	}

	logi("Initialising Miscellaneous Pins\n");
	if (h->pin_nrst != 255)
	{
		logi("Restore pin enabled: GPIO%d\n", h->pin_nrst);
		gpio_reset_pin    (h->pin_nrst);
		gpio_set_direction(h->pin_nrst , GPIO_MODE_OUTPUT);
	}

	gpio_reset_pin(h->pin_kben);
	gpio_reset_pin(h->pin_ctrl);
	gpio_reset_pin(h->pin_shft);
	gpio_reset_pin(h->pin_cmdr);

	gpio_set_direction(h->pin_kben, GPIO_MODE_OUTPUT);
	gpio_set_direction(h->pin_ctrl, GPIO_MODE_OUTPUT);
	gpio_set_direction(h->pin_shft, GPIO_MODE_OUTPUT);
	gpio_set_direction(h->pin_cmdr, GPIO_MODE_OUTPUT);

	c64b_keyboard_trace_reset(h);
}
