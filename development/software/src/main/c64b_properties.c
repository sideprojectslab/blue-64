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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include <nvs.h>
#include <nvs_flash.h>

#include "c64b_properties.h"

//----------------------------------------------------------------------------//
// properties

unsigned int kb_map                 = KB_MAP_SYMBOLIC;
unsigned int af_rate                = 0;
TickType_t   af_prd                 = (TickType_t)portMAX_DELAY;
unsigned int scan_time              = 0;
unsigned int ct_map[CT_MAP_IDX_NUM] = {0};

const char* ct_map_key[CT_MAP_IDX_NUM] =
{
	"c64b.ct_bh",
	"c64b.ct_bm",
	"c64b.ct_lt",
	"c64b.ct_rt",
	"c64b.ct_ls",
	"c64b.ct_rs"
};

const uint8_t scan_time_to_minutes[6] = {0, 1, 2, 5, 10, 30};

//----------------------------------------------------------------------------//
// former bluepad32 functions

static const char* STORAGE_NAMESPACE = "bp32";

// Uses NVS for storage. Used in all ESP32 Bluepad32 platforms.

void old_property_set(const char* key, uni_property_type_t type, uni_property_value_t value) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    uint32_t* float_alias;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        loge("Could not open readwrite NVS storage, key: %s, err=%#x\n", key, err);
        return;
    }

    switch (type) {
        case UNI_PROPERTY_TYPE_U8:
            err = nvs_set_u8(nvs_handle, key, value.u8);
            break;
        case UNI_PROPERTY_TYPE_U32:
            err = nvs_set_u32(nvs_handle, key, value.u32);
            break;
        case UNI_PROPERTY_TYPE_FLOAT:
            float_alias = (uint32_t*)&value.f32;
            err = nvs_set_u32(nvs_handle, key, *float_alias);
            break;
        case UNI_PROPERTY_TYPE_STRING:
            err = nvs_set_str(nvs_handle, key, value.str);
            break;
        default:
    }

    if (err != ESP_OK) {
        loge("Could not store '%s' in NVS, err=%#x\n", key, err);
        goto out;
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        loge("Could not commit '%s' in NVS, err=%#x\n", key, err);
    }

out:
    nvs_close(nvs_handle);
}

uni_property_value_t old_property_get(const char* key, uni_property_type_t type, uni_property_value_t def) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    uni_property_value_t ret;
    size_t str_len;
    static char str_ret[128];

    err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        // Might be valid if no bp32 keys were stored
        logd("Could not open readonly NVS storage, key:'%s'\n", key);
        return def;
    }

    switch (type) {
        case UNI_PROPERTY_TYPE_U8:
            err = nvs_get_u8(nvs_handle, key, &ret.u8);
            break;
        case UNI_PROPERTY_TYPE_U32:
            err = nvs_get_u32(nvs_handle, key, &ret.u32);
            break;
        case UNI_PROPERTY_TYPE_FLOAT:
            err = nvs_get_u32(nvs_handle, key, (uint32_t*)&ret.f32);
            break;
        case UNI_PROPERTY_TYPE_STRING:
            ret.str = str_ret;
            memset(str_ret, 0, sizeof(str_ret));
            err = nvs_get_str(nvs_handle, key, str_ret, &str_len);
            break;
        default:
    }

    if (err != ESP_OK) {
        // Might be valid if the key was not previously stored
        logd("could not read property '%s' from NVS, err=%#x\n", key, err);
        ret = def;
        /* falltrhough */
    }

    nvs_close(nvs_handle);
    return ret;
}

void old_property_init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        logi("Erasing flash\n");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

//----------------------------------------------------------------------------//
// functions

void c64b_property_reset(void)
{
	ESP_ERROR_CHECK(nvs_flash_erase());
	old_property_init();
	ESP_ERROR_CHECK(nvs_flash_init());
}


void c64b_property_set_u8(const char* key, uint8_t value)
{
	uni_property_value_t prop;
	prop.u8 = value;
	old_property_set(key, UNI_PROPERTY_TYPE_U8, prop);
}

uint8_t c64b_property_get_u8(const char* key, uint8_t def)
{
	uni_property_value_t prop;
	prop.u8 = def;
	prop = old_property_get(key, UNI_PROPERTY_TYPE_U8, prop);
	return prop.u8;
}

void c64b_property_init(void)
{
	old_property_init();
	kb_map    = c64b_property_get_u8(C64B_PROPERTY_KEY_KB_MAP, KB_MAP_SYMBOLIC);
	scan_time = c64b_property_get_u8(C64B_PROPERTY_KEY_SCAN_TIME, 2);
	af_rate   = c64b_property_get_u8(C64B_PROPERTY_KEY_AF_RATE, 0);
	if (af_rate != 0)
		af_prd = (1000 / (TickType_t)af_rate) / portTICK_PERIOD_MS;

	ct_map[CT_MAP_IDX_BH] = c64b_property_get_u8(ct_map_key[CT_MAP_IDX_BH], C64B_KB_IDX_NONE);
	ct_map[CT_MAP_IDX_BM] = c64b_property_get_u8(ct_map_key[CT_MAP_IDX_BM], c64b_keyboard_key_to_idx(" "));
	ct_map[CT_MAP_IDX_LT] = c64b_property_get_u8(ct_map_key[CT_MAP_IDX_LT], C64B_KB_IDX_NONE);
	ct_map[CT_MAP_IDX_RT] = c64b_property_get_u8(ct_map_key[CT_MAP_IDX_RT], C64B_KB_IDX_NONE);
	ct_map[CT_MAP_IDX_LS] = c64b_property_get_u8(ct_map_key[CT_MAP_IDX_LS], C64B_KB_IDX_NONE);
	ct_map[CT_MAP_IDX_RS] = c64b_property_get_u8(ct_map_key[CT_MAP_IDX_RS], c64b_keyboard_key_to_idx("~f1~"));
}
