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

#ifndef BT64_KEYBOARD_H
#define BT64_KEYBOARD_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define BT64_CTL_BITS 5
#define BT64_KKA_BITS 3

#define NUM_KEYS         116
#define BT64_KB_IDX_REST (NUM_KEYS - 2)
#define BT64_KB_IDX_NONE (NUM_KEYS - 1)

typedef struct
{
	char*        str;
	unsigned int col;
	unsigned int row;
	bool         shft;
	bool         ctrl;
	bool         prnt;
} t_bt64_key_id;


typedef struct
{
	unsigned int         pin_kca[3];
	unsigned int         pin_kra[3];
	unsigned int         pin_col[5];
	unsigned int         pin_row[5];
	unsigned int         pin_nrst;
	unsigned int         pin_ctrl;
	unsigned int         pin_shft;
	unsigned int         pin_cmdr;
	unsigned int         pin_kben;
	unsigned int         feed_psh_ms;
	unsigned int         feed_rel_ms;
	const uint8_t*       col_perm;
	const uint8_t*       row_perm;
	const t_bt64_key_id* trace_key;
} t_bt64_keyboard;


typedef enum
{
	CPORT_UP,
	CPORT_LL,
	CPORT_RR,
	CPORT_DN,
	CPORT_FF
}t_bt64_cport_key;


typedef unsigned int t_bt64_cport_idx;
#define KPORT      0
#define CPORT_1    1
#define CPORT_2    2

extern const t_bt64_key_id KEY_IDS[];
//extern const unsigned int NUM_KEYS;

//----------------------------------------------------------------------------//
// FUNCTIONS

void bt64_keyboard_init(t_bt64_keyboard *h);
void bt64_keyboard_reset(t_bt64_keyboard *h);

void bt64_keyboard_cport_psh(t_bt64_keyboard *h, t_bt64_cport_key key, t_bt64_cport_idx idx);
void bt64_keyboard_cport_rel(t_bt64_keyboard *h, t_bt64_cport_key key, t_bt64_cport_idx idx);

void bt64_keyboard_rest_psh(t_bt64_keyboard *h);
void bt64_keyboard_rest_rel(t_bt64_keyboard *h);

void bt64_keyboard_ctrl_psh(t_bt64_keyboard *h);
void bt64_keyboard_ctrl_rel(t_bt64_keyboard *h);

void bt64_keyboard_shft_psh(t_bt64_keyboard *h);
void bt64_keyboard_shft_rel(t_bt64_keyboard *h);

void bt64_keyboard_cmdr_psh(t_bt64_keyboard *h);
void bt64_keyboard_cmdr_rel(t_bt64_keyboard *h);

bool bt64_keyboard_key_psh(t_bt64_keyboard *h, const t_bt64_key_id *k);
bool bt64_keyboard_key_rel(t_bt64_keyboard *h, const t_bt64_key_id *k);

void bt64_keyboard_keys_rel(t_bt64_keyboard *h, bool rel_shft);
void bt64_keyboard_mods_rel(t_bt64_keyboard *h);

bool bt64_keyboard_char_psh(t_bt64_keyboard *h, const char *s);
bool bt64_keyboard_char_rel(t_bt64_keyboard *h, const char *s);

bool bt64_keyboard_feed_str(t_bt64_keyboard *h, char* s);
bool bt64_keyboard_feed_prg(t_bt64_keyboard *h, char** s, uint32_t nlines);

unsigned int         bt64_keyboard_char_to_idx(const char* s);
const char*          bt64_keyboard_idx_to_char(unsigned int i);
const t_bt64_key_id* bt64_keyboard_idx_to_key(unsigned int i);

void                 bt64_keyboard_trace_reset(t_bt64_keyboard *h);
const t_bt64_key_id* bt64_keyboard_trace_get(t_bt64_keyboard *h);

#endif
