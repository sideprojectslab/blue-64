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

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "driver/gpio.h"

#include "c64b_update.h"

// SDMMC includes
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

// OTA includes
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"

#include "uni_log.h"


#define MOUNT_POINT "/s"

#define OTA_BUF_SIZE 1024
static char ota_buf[OTA_BUF_SIZE + 1] = {0};

FILE *f;
sdmmc_card_t *card;

t_c64b_update_err c64b_update_init(bool check_only)
{
	logi("Checking Updates\n");

	gpio_set_pull_mode( 2, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode( 4, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);

	// Options for mounting the filesystem.
	// If format_if_mount_failed is set to true, SD card will be partitioned and
	// formatted in case when mounting fails.
	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
		.format_if_mount_failed = false,
		.max_files = 5,
		.allocation_unit_size = 16 * 1024
	};

	sdmmc_host_t host = SDMMC_HOST_DEFAULT();
	host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

	sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

	slot_config.width = 4;
	slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

	if(esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card) != ESP_OK)
		return NO_SDCARD;

	logi("Checking New Firmware\n");

	// Open update file
	const char fw_path[] = MOUNT_POINT"/application.bin";

	f = fopen(fw_path, "rb");
	if (f == NULL)
	{
		esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
		return NO_FIRMWARE;
	}

	if(check_only)
	{
		fclose(f);
		esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
	}

	return UPDATE_OK;
}

t_c64b_update_err c64b_update()
{
	logi("Starting Update\n");

	if(c64b_update_init(false) != ESP_OK)
	{
		fclose(f);
		esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
		return READ_ERROR;
	}

//	if(nvs_flash_init() != ESP_OK)
//	{
//		fclose(f);
//		esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
//		return WRITE_ERROR;
//	}

	esp_ota_handle_t update_handle = 0 ;
	const esp_partition_t *update_partition = NULL;

	update_partition = esp_ota_get_next_update_partition(NULL);
	assert(update_partition != NULL);

	if(esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle) != ESP_OK)
	{
		esp_ota_abort(update_handle);
		fclose(f);
		esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
		return WRITE_ERROR;
	}

	size_t nread = 0;
	int i = 0;

	while(true)
	{
		nread = fread(ota_buf, 1, OTA_BUF_SIZE, f);
		if (ferror(f))
		{
			esp_ota_abort(update_handle);
			fclose(f);
			esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
			return READ_ERROR;
		}

		if(esp_ota_write(update_handle, (const void *)ota_buf, nread) != ESP_OK)
		{
			esp_ota_abort(update_handle);
			fclose(f);
			esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
			return WRITE_ERROR;
		}

		i += nread;
		if (nread != OTA_BUF_SIZE)
			break;
	}

	fclose(f);
	esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);

	// commit OTA
	if(esp_ota_end(update_handle) != ESP_OK)
		return WRITE_ERROR;

	if(esp_ota_set_boot_partition(update_partition) != ESP_OK)
		return WRITE_ERROR;

	logi("Update Completed\n");
	return UPDATE_OK;
}
