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

#ifndef C64B_KEYBOARD_H
#define C64B_KEYBOARD_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define C64B_CTL_BITS 5
#define C64B_KKA_BITS 3

typedef struct
{
	unsigned int pin_kca[3];
	unsigned int pin_kra[3];
	unsigned int pin_col[5];
	unsigned int pin_row[5];
	unsigned int pin_nrst;
	unsigned int pin_ctrl;
	unsigned int pin_shift;
	unsigned int pin_cmdr;
	unsigned int pin_ken;
	unsigned int feed_press_ms;
	unsigned int feed_clear_ms;
} t_c64b_keyboard;


typedef struct
{
	char         *string;
	unsigned int col;
	unsigned int row;
	bool         shift;
} t_c64b_key_id;


typedef enum
{
	CPORT_UP,
	CPORT_LL,
	CPORT_RR,
	CPORT_DN,
	CPORT_FF
}t_c64b_cport_key;

typedef enum
{
	CPORT_1,
	CPORT_2
}t_c64b_cport_idx;

extern const t_c64b_key_id KEY_IDS[];

void c64b_keyboard_init(t_c64b_keyboard *h);
void c64b_keyboard_reset(t_c64b_keyboard *h);

void c64b_keyboard_keys_release(t_c64b_keyboard *h, bool clear_shift);

void c64b_keyboard_cport_press(t_c64b_keyboard *h, t_c64b_cport_key key, t_c64b_cport_idx idx);
void c64b_keyboard_cport_release(t_c64b_keyboard *h, t_c64b_cport_key key, t_c64b_cport_idx idx);

void c64b_keyboard_restore_press(t_c64b_keyboard *h);
void c64b_keyboard_restore_release(t_c64b_keyboard *h);

void c64b_keyboard_ctrl_press(t_c64b_keyboard *h);
void c64b_keyboard_ctrl_release(t_c64b_keyboard *h);

void c64b_keyboard_shift_press(t_c64b_keyboard *h);
void c64b_keyboard_shift_release(t_c64b_keyboard *h);

void c64b_keyboard_cmdr_press(t_c64b_keyboard *h);
void c64b_keyboard_cmdr_release(t_c64b_keyboard *h);

bool c64b_keyboard_key_press(t_c64b_keyboard *h, const t_c64b_key_id *k);
bool c64b_keyboard_key_release(t_c64b_keyboard *h, const t_c64b_key_id *k);

bool c64b_keyboard_char_press(t_c64b_keyboard *h, char *s);
bool c64b_keyboard_char_release(t_c64b_keyboard *h, char *s);

bool c64b_keyboard_feed_string(t_c64b_keyboard *h, char* s);

#endif
