#pragma once

#define HL_NAME(n) miniaudio_##n

#include <string.h>
#include "types.h"

#define _BUFFER _ABSTRACT(ma_buffer_handle)
#define _SOUND _ABSTRACT(ma_sound_handle)
#define _GROUP _ABSTRACT(ma_group_handle)
#define _DECODER _ABSTRACT(ma_stream_decoder)
#define _PCM_SINK _ABSTRACT(ma_pcm_sink_handle)

// ─── UTILS ──────────────────────────────────────

extern ma_engine engine;
extern ma_result lastResult;
extern int lastDecodedChannels;
extern int lastDecodedSampleRate;
extern int lastDecodedSamples;
extern sound_callback_entry* sound_callbacks;
extern bool engine_initialized;
extern am_mutex callback_mutex;

ma_result memory_stream_read(void* userData, void* bufferOut, size_t bytesToRead, size_t* bytesRead);
ma_result memory_stream_seek(void* userData, ma_int64 byteOffset, ma_seek_origin origin);
ma_result memory_stream_tell(void* userData, ma_int64* cursor);

ma_buffer_handle* buffer_handle_alloc(ma_audio_buffer* buffer, void* data);
void buffer_handle_release(ma_buffer_handle* handle, int force);
ma_buffer_handle* create_buffer_from_pcm(float* data, ma_uint64 frameCount, ma_uint32 channels, ma_uint32 sampleRate);

void group_handle_ref(ma_group_handle* group);
void group_handle_release(ma_group_handle* handle, int force);
ma_sound_group* group_ptr(ma_group_handle* group);

void stream_decoder_finalize(ma_stream_decoder* decoder);
void stream_decoder_release(ma_stream_decoder* decoder, int force);
ma_data_source* stream_decoder_data_source(ma_stream_decoder* decoder);
ma_result stream_decoder_seek(ma_stream_decoder* decoder, ma_uint64 frame);
ma_result stream_decoder_read(ma_stream_decoder* decoder, float* out, ma_uint64 frames, ma_uint64* framesRead);

void set_last_decoded_format(ma_uint32 channels, ma_uint32 sampleRate, ma_uint64 frameCount);
void clear_sound_callbacks();
int is_opus_stream(const unsigned char* bytes, size_t size);
int is_vorbis_stream(const unsigned char* bytes, size_t size);

vbyte* decode_bytes_to_pcm_float(const unsigned char* bytes, size_t size);
vbyte* decode_bytes_to_pcm_s16(const unsigned char* bytes, size_t size);
ma_buffer_handle* decode_bytes_to_buffer_direct(const unsigned char* bytes, size_t size);

HL_PRIM ma_buffer_handle* HL_NAME(buffer_from_pcm_float)(vbyte* bytes, int size, int channels, int sampleRate);
