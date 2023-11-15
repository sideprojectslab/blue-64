/*
 * Copyright (C) 2022 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL BLUEKITCHEN
 * GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

/**
 * @title LE Audio Util
 * 
 */

#ifndef LE_AUDIO_UTIL_H
#define LE_AUDIO_UTIL_H

#include <stdint.h>
#include "le_audio.h"
#include "btstack_lc3.h"

#if defined __cplusplus
extern "C" {
#endif

uint16_t le_audio_util_virtual_memcpy_helper(
    const uint8_t * field_data, uint16_t field_len, uint16_t field_offset,
    uint8_t * buffer, uint16_t buffer_size, uint16_t buffer_offset);

uint16_t le_audio_util_metadata_virtual_memcpy(const le_audio_metadata_t * metadata, uint8_t metadata_length, uint16_t * records_offset, uint8_t * buffer, uint16_t buffer_size, uint16_t buffer_offset);

uint16_t le_audio_util_metadata_parse(const uint8_t *buffer, uint8_t buffer_size, le_audio_metadata_t * metadata);

uint16_t le_audio_util_metadata_serialize(const le_audio_metadata_t *metadata, uint8_t * event, uint16_t event_size);

uint16_t le_audio_util_metadata_serialize_using_mask(const le_audio_metadata_t *metadata, uint8_t * tlv_buffer, uint16_t tlv_buffer_size);


btstack_lc3_frame_duration_t le_audio_util_get_btstack_lc3_frame_duration(le_audio_codec_frame_duration_index_t frame_duration_index);

uint16_t le_audio_get_frame_duration_us(le_audio_codec_frame_duration_index_t frame_duration_index);
le_audio_codec_frame_duration_index_t le_audio_get_frame_duration_index(uint16_t frame_duration_us);


uint32_t le_audio_get_sampling_frequency_hz(le_audio_codec_sampling_frequency_index_t sampling_frequency_index);
le_audio_codec_sampling_frequency_index_t le_audio_get_sampling_frequency_index(uint32_t sampling_frequency_hz);

const le_audio_codec_configuration_t * le_audio_util_get_codec_setting(
    le_audio_codec_sampling_frequency_index_t sampling_frequency_index, 
    le_audio_codec_frame_duration_index_t frame_duration_index, 
    le_audio_quality_t audio_quality);

const le_audio_qos_configuration_t * le_audio_util_get_qos_setting(
    le_audio_codec_sampling_frequency_index_t sampling_frequency_index, 
    le_audio_codec_frame_duration_index_t frame_duration_index, 
    le_audio_quality_t audio_quality, uint8_t num_channels);

#if defined __cplusplus
}
#endif

#endif

