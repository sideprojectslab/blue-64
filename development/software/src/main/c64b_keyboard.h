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

#ifndef C64B_KEYBOARD_H
#define C64B_KEYBOARD_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define C64B_CTL_BITS 5
#define C64B_KKA_BITS 3

#define C64B_KB_IDX_NONE 255

typedef struct
{
	unsigned int   pin_kca[3];
	unsigned int   pin_kra[3];
	unsigned int   pin_col[5];
	unsigned int   pin_row[5];
	unsigned int   pin_nrst;
	unsigned int   pin_ctrl;
	unsigned int   pin_shft;
	unsigned int   pin_cmdr;
	unsigned int   pin_kben;
	unsigned int   feed_psh_ms;
	unsigned int   feed_rel_ms;
	const uint8_t* col_perm;
	const uint8_t* row_perm;
} t_c64b_keyboard;


typedef struct
{
	char*        str;
	unsigned int col;
	unsigned int row;
	bool         shft;
	bool         ctrl;
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

//----------------------------------------------------------------------------//
// FUNCTIONS

void c64b_keyboard_init(t_c64b_keyboard *h);
void c64b_keyboard_reset(t_c64b_keyboard *h);

void c64b_keyboard_cport_psh(t_c64b_keyboard *h, t_c64b_cport_key key, t_c64b_cport_idx idx);
void c64b_keyboard_cport_rel(t_c64b_keyboard *h, t_c64b_cport_key key, t_c64b_cport_idx idx);

void c64b_keyboard_rest_psh(t_c64b_keyboard *h);
void c64b_keyboard_rest_rel(t_c64b_keyboard *h);

void c64b_keyboard_ctrl_psh(t_c64b_keyboard *h);
void c64b_keyboard_ctrl_rel(t_c64b_keyboard *h);

void c64b_keyboard_shft_psh(t_c64b_keyboard *h);
void c64b_keyboard_shft_rel(t_c64b_keyboard *h);

void c64b_keyboard_cmdr_psh(t_c64b_keyboard *h);
void c64b_keyboard_cmdr_rel(t_c64b_keyboard *h);

bool c64b_keyboard_key_psh(t_c64b_keyboard *h, const t_c64b_key_id *k);
bool c64b_keyboard_key_rel(t_c64b_keyboard *h, const t_c64b_key_id *k);

void c64b_keyboard_keys_rel(t_c64b_keyboard *h, bool rel_shft);
void c64b_keyboard_mods_rel(t_c64b_keyboard *h);

bool c64b_keyboard_char_psh(t_c64b_keyboard *h, const char *s);
bool c64b_keyboard_char_rel(t_c64b_keyboard *h, const char *s);

bool c64b_keyboard_feed_str(t_c64b_keyboard *h, char* s);
bool c64b_keyboard_feed_prg(t_c64b_keyboard *h, char** s, uint32_t nlines);

unsigned int c64b_keyboard_key_to_idx(const char* s);
const char*  c64b_keyboard_idx_to_key(unsigned int i);

#endif
