/****************************************************************************
Copyright 2020 Vittorio Pascucci

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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "c64b_keyboard.h"

#include "uni_log.h"
#include "uni_platform.h"
#include "uni_gpio.h"

// the "~" character is used to "escape" complex characters.
// It is not present on the c64 keyboard and thus is free to use

#define ESC_LEN_MAX 6
#define SHIFT_COL   1
#define SHIFT_ROW   7

const t_c64b_key_id KEY_IDS[] =
{
	// unshifted keys

	// 0
	{"~stop~"  , 7, 7, false},
	{"q"       , 7, 6, false},
	{"~cmdr~"  , 7, 5, false},
	{" "       , 7, 4, false},
	{"2"       , 7, 3, false},
	{"~ctrl~"  , 7, 2, false},
	{"~arll~"  , 7, 1, false},
	{"1"       , 7, 0, false},

	// 8
	{"/"       , 6, 7, false},
	{"~arup~"  , 6, 6, false},
	{"="       , 6, 5, false},
	{"~rsh~"   , 6, 4, false},
	{"~home~"  , 6, 3, false},
	{";"       , 6, 2, false},
	{"*"       , 6, 1, false},
	{"£"       , 6, 0, false},

	// 16
	{","       , 5, 7, false},
	{"@"       , 5, 6, false},
	{":"       , 5, 5, false},
	{"."       , 5, 4, false},
	{"-"       , 5, 3, false},
	{"l"       , 5, 2, false},
	{"p"       , 5, 1, false},
	{"+"       , 5, 0, false},

	// 24
	{"n"       , 4, 7, false},
	{"o"       , 4, 6, false},
	{"k"       , 4, 5, false},
	{"m"       , 4, 4, false},
	{"0"       , 4, 3, false},
	{"j"       , 4, 2, false},
	{"i"       , 4, 1, false},
	{"9"       , 4, 0, false},

	// 32
	{"v"       , 3, 7, false},
	{"u"       , 3, 6, false},
	{"h"       , 3, 5, false},
	{"b"       , 3, 4, false},
	{"8"       , 3, 3, false},
	{"g"       , 3, 2, false},
	{"y"       , 3, 1, false},
	{"7"       , 3, 0, false},

	// 40
	{"x"       , 2, 7, false},
	{"t"       , 2, 6, false},
	{"f"       , 2, 5, false},
	{"c"       , 2, 4, false},
	{"6"       , 2, 3, false},
	{"d"       , 2, 2, false},
	{"r"       , 2, 1, false},
	{"5"       , 2, 0, false},

	// 48
	{"lsh"     , 1, 7, false},
	{"e"       , 1, 6, false},
	{"s"       , 1, 5, false},
	{"z"       , 1, 4, false},
	{"4"       , 1, 3, false},
	{"a"       , 1, 2, false},
	{"w"       , 1, 1, false},
	{"3"       , 1, 0, false},

	// 56
	{"~dn~"    , 0, 7, false},
	{"~f5~"    , 0, 6, false},
	{"~f3~"    , 0, 5, false},
	{"~f1~"    , 0, 4, false},
	{"~f7~"    , 0, 3, false},
	{"~rr~"    , 0, 2, false},
	{"~ret~"   , 0, 1, false},
	{"~del~"   , 0, 0, false},

	// shifted characters

	// 64
	{"~run~"   , 7, 7, true},
	{"\""      , 7, 3, true},
	{"!"       , 7, 0, true},

	// 67
	{"?"       , 6, 7, true},
	{"~clr~"   , 6, 3, true},
	{"]"       , 6, 2, true},

	// 70
	{"<"       , 5, 7, true},
	{"["       , 5, 5, true},
	{">"       , 5, 4, true},

	// 73
	{")"       , 4, 0, true},

	// 75
	{"("       , 3, 3, true},
	{"'"       , 3, 0, true},

	// 77
	{"&"       , 2, 3, true},
	{"%%"      , 2, 0, true},

	// 79
	{"$"       , 1, 3, true},
	{"#"       , 1, 0, true},

	// 81
	{"~up~"    , 0, 7, true},
	{"~f6~"    , 0, 6, true},
	{"~f4~"    , 0, 5, true},
	{"~f2~"    , 0, 4, true},
	{"~f8~"    , 0, 3, true},
	{"~ll~"    , 0, 2, true},
	{"~inst~"  , 0, 0, true}
};


static const t_c64b_key_id * c64b_keyboard_char_to_key(const char* s)
{
	if(s == NULL)
		return NULL;

	for(unsigned int i = 0; i < sizeof(KEY_IDS) / sizeof(KEY_IDS[0]); ++i)
	{
		unsigned int j = 0;
		while(KEY_IDS[i].string[j] != 0)
		{
			if(KEY_IDS[i].string[j] != s[j])
				break;
			++j;
		}

		if(KEY_IDS[i].string[j] == 0)
			return &(KEY_IDS[i]);
	}
	return NULL;
}


void c64b_keyboard_init(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;

	c64b_keyboard_clear(h);

	for(unsigned int i = 0; i < 8; ++i)
	{
		gpio_reset_pin    (h->pin_col[i]);
		gpio_reset_pin    (h->pin_row[i]);
		gpio_set_direction(h->pin_col[i], GPIO_MODE_OUTPUT);
		gpio_set_direction(h->pin_row[i], GPIO_MODE_OUTPUT);
	}

	gpio_reset_pin    (h->pin_nrestore);
	gpio_set_direction(h->pin_nrestore, GPIO_MODE_OUTPUT);
}


void c64b_keyboard_clear(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;

	for(unsigned int i = 0; i < 8; ++i)
	{
		gpio_set_level(h->pin_col[i], 0);
		gpio_set_level(h->pin_row[i], 0);
	}
	gpio_set_level(h->pin_nrestore, 0);
}


bool c64b_keyboard_key_press(t_c64b_keyboard *h, const t_c64b_key_id *k)
{
	if((h == NULL) || (k == NULL))
		return false;

	logi("keyboard: pressing key \"%s\"\n", k->string);

	if(k->shift)
	{
		gpio_set_level(h->pin_col[SHIFT_COL], 1);
		gpio_set_level(h->pin_row[SHIFT_ROW], 1);
		vTaskDelay(h->feed_rate_ms / portTICK_PERIOD_MS);
	}

	gpio_set_level(h->pin_col[k->col], 1);
	gpio_set_level(h->pin_row[k->row], 1);
	return true;
}


bool c64b_keyboard_key_release(t_c64b_keyboard *h, const t_c64b_key_id *k)
{
	if((h == NULL) || (k == NULL))
		return false;

	logi("keyboard: releasing key \"%s\"\n", k->string);

	gpio_set_level(h->pin_row[k->row], 0);
	gpio_set_level(h->pin_col[k->col], 0);

	if(k->shift)
	{
		gpio_set_level(h->pin_row[SHIFT_ROW], 0);
		gpio_set_level(h->pin_col[SHIFT_COL], 0);
	}

	return true;
}


void c64b_keyboard_cport_press(t_c64b_keyboard *h, t_c64b_cport_id key)
{
	if(h == NULL)
		return;

	switch(key)
	{
		case CPORT_UP:
			gpio_set_level(h->pin_col[0], 1);
			break;
		case CPORT_LL:
			gpio_set_level(h->pin_col[2], 1);
			break;
		case CPORT_RR:
			gpio_set_level(h->pin_col[3], 1);
			break;
		case CPORT_DN:
			gpio_set_level(h->pin_col[1], 1);
			break;
		case CPORT_FF:
			gpio_set_level(h->pin_col[4], 1);
			break;
	}
}


void c64b_keyboard_cport_release(t_c64b_keyboard *h, t_c64b_cport_id key)
{
	if(h == NULL)
		return;

	switch(key)
	{
		case CPORT_UP:
			gpio_set_level(h->pin_col[0], 0);
			break;
		case CPORT_LL:
			gpio_set_level(h->pin_col[2], 0);
			break;
		case CPORT_RR:
			gpio_set_level(h->pin_col[3], 0);
			break;
		case CPORT_DN:
			gpio_set_level(h->pin_col[1], 0);
			break;
		case CPORT_FF:
			gpio_set_level(h->pin_col[4], 0);
			break;
	}
}


void c64b_keyboard_restore_press(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;
	gpio_set_level(h->pin_nrestore, 1);
}


void c64b_keyboard_restore_release(t_c64b_keyboard *h)
{
	if(h == NULL)
		return;
	gpio_set_level(h->pin_nrestore, 0);
}


bool c64b_keyboard_char_press(t_c64b_keyboard *h, char *s)
{
	if((h == NULL) || (s == NULL))
		return false;

	const t_c64b_key_id *key  = NULL;

	key = c64b_keyboard_char_to_key(s);

	if(key == NULL)
		return false;

	c64b_keyboard_key_press(h, key);
	return true;
}


bool c64b_keyboard_char_release(t_c64b_keyboard *h, char *s)
{
	if((h == NULL) || (s == NULL))
		return false;

	const t_c64b_key_id *key  = NULL;

	key = c64b_keyboard_char_to_key(s);

	if(key == NULL)
		return false;

	c64b_keyboard_key_release(h, key);
	return true;
}


bool c64b_keyboard_feed_string(t_c64b_keyboard *h, char* s)
{
	if((h == NULL) || (s == NULL))
		return false;

	const char          *head = s;
	const t_c64b_key_id *key  = NULL;

	while(*head != 0)
	{
		key = c64b_keyboard_char_to_key(head);

		if(key == NULL)
			return false;

		c64b_keyboard_key_press(h, key);
		vTaskDelay(h->feed_rate_ms / portTICK_PERIOD_MS);
		c64b_keyboard_key_release(h, key);

		head += strlen(key->string);
	}
	return true;
}
