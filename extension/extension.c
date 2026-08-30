#define MINIAUDIO_IMPLEMENTATION
#include "utils.h"

// ─── MAIN ──────────────────────────────────────

ma_engine engine;
ma_result lastResult;
int lastDecodedChannels, lastDecodedSampleRate, lastDecodedSamples;
bool engine_initialized = false;
am_mutex callback_mutex;

sound_callback_entry* sound_callbacks = NULL;

void clear_sound_callbacks()
{
	sound_callback_entry* entry;
	am_mutex_lock(&callback_mutex);
	entry = sound_callbacks;
	while (entry != NULL)
	{
		sound_callback_entry* next = entry->next;
		hl_remove_root(&entry->callback);
		free(entry);
		entry = next;
	}
	sound_callbacks = NULL;
	am_mutex_unlock(&callback_mutex);
}

HL_PRIM ma_buffer_handle* HL_NAME(buffer_from_pcm_float)(vbyte* bytes, int size, int channels, int sampleRate);

ma_result memory_stream_read(void* userData, void* bufferOut, size_t bytesToRead, size_t* bytesRead)
{
	memory_stream* stream = (memory_stream*)userData;
	size_t remaining;
	size_t bytesToCopy;

	if (bytesRead != NULL)
		*bytesRead = 0;

	if (stream == NULL || bufferOut == NULL)
		return MA_INVALID_ARGS;

	remaining = stream->size - stream->cursor;
	bytesToCopy = bytesToRead;
	if (bytesToCopy > remaining)
		bytesToCopy = remaining;

	if (bytesToCopy > 0)
	{
		memcpy(bufferOut, stream->data + stream->cursor, bytesToCopy);
		stream->cursor += bytesToCopy;
	}

	if (bytesRead != NULL)
		*bytesRead = bytesToCopy;

	return bytesToCopy == 0 ? MA_AT_END : MA_SUCCESS;
}

ma_result memory_stream_seek(void* userData, ma_int64 byteOffset, ma_seek_origin origin)
{
	memory_stream* stream = (memory_stream*)userData;
	size_t base = 0;
	ma_int64 target;

	if (stream == NULL)
		return MA_INVALID_ARGS;

	switch (origin)
	{
		case ma_seek_origin_start:
			base = 0;
			break;
		case ma_seek_origin_current:
			base = stream->cursor;
			break;
		case ma_seek_origin_end:
			base = stream->size;
			break;
		default:
			return MA_INVALID_ARGS;
	}

	target = (ma_int64)base + byteOffset;
	if (target < 0 || (ma_uint64)target > stream->size)
		return MA_INVALID_ARGS;

	stream->cursor = (size_t)target;
	return MA_SUCCESS;
}

ma_result memory_stream_tell(void* userData, ma_int64* cursor)
{
	memory_stream* stream = (memory_stream*)userData;
	if (stream == NULL || cursor == NULL)
		return MA_INVALID_ARGS;

	*cursor = (ma_int64)stream->cursor;
	return MA_SUCCESS;
}

static void buffer_handle_finalize(ma_buffer_handle* handle);

ma_buffer_handle* buffer_handle_alloc(ma_audio_buffer* buffer, void* data)
{
	ma_buffer_handle* handle;

	if (buffer == NULL)
		return NULL;

	handle = (ma_buffer_handle*)hl_gc_alloc_finalizer(sizeof(ma_buffer_handle));
	memset(handle, 0, sizeof(ma_buffer_handle));
	handle->finalize = buffer_handle_finalize;
	handle->buffer = buffer;
	handle->data = data;
	return handle;
}

static void buffer_handle_finalize(ma_buffer_handle* handle)
{
	buffer_handle_release(handle, 0);
}

void buffer_handle_release(ma_buffer_handle* handle, int force)
{
	if (handle == NULL || handle->buffer == NULL)
		return;

	if (!force && handle->refCount > 0)
	{
		handle->disposeRequested = 1;
		return;
	}

	ma_audio_buffer_uninit_and_free(handle->buffer);
	handle->buffer = NULL;
	if (handle->data != NULL)
	{
		ma_free(handle->data, NULL);
		handle->data = NULL;
	}
	handle->finalize = NULL;
	handle->disposeRequested = 0;
}

ma_buffer_handle* create_buffer_from_pcm(float* data, ma_uint64 frameCount, ma_uint32 channels, ma_uint32 sampleRate)
{
	ma_audio_buffer_config bufferConfig;
	ma_audio_buffer* buffer = NULL;

	bufferConfig = ma_audio_buffer_config_init(ma_format_f32, channels, frameCount, data, NULL);
	lastResult = ma_audio_buffer_alloc_and_init(&bufferConfig, &buffer);
	if (lastResult == MA_SUCCESS)
	{
		buffer->ref.sampleRate = sampleRate;
		return buffer_handle_alloc(buffer, data);
	}

	ma_free(data, NULL);
	return NULL;
}

void stream_decoder_finalize(ma_stream_decoder* decoder)
{
	stream_decoder_release(decoder, 0);
}

void stream_decoder_release(ma_stream_decoder* decoder, int force)
{
	if (decoder == NULL || !decoder->initialized)
		return;

	if (!force && decoder->refCount > 0)
	{
		decoder->disposeRequested = 1;
		return;
	}

	switch (decoder->kind)
	{
		case STREAM_DECODER_VORBIS:
			ma_libvorbis_uninit(&decoder->vorbis, NULL);
			break;
		case STREAM_DECODER_OPUS:
			ma_libopus_uninit(&decoder->opus, NULL);
			break;
		default:
			ma_decoder_uninit(&decoder->decoder);
			break;
	}

	decoder->initialized = 0;
	decoder->finalize = NULL;
	decoder->disposeRequested = 0;
	if (decoder->bytes != NULL)
	{
		hl_remove_root(&decoder->bytes);
		decoder->bytes = NULL;
	}
}

ma_data_source* stream_decoder_data_source(ma_stream_decoder* decoder)
{
	if (decoder == NULL || !decoder->initialized)
		return NULL;

	switch (decoder->kind)
	{
		case STREAM_DECODER_VORBIS:
			return (ma_data_source*)&decoder->vorbis;
		case STREAM_DECODER_OPUS:
			return (ma_data_source*)&decoder->opus;
		default:
			return (ma_data_source*)&decoder->decoder;
	}
}

ma_result stream_decoder_seek(ma_stream_decoder* decoder, ma_uint64 frame)
{
	if (decoder == NULL || !decoder->initialized)
		return MA_INVALID_ARGS;

	switch (decoder->kind)
	{
		case STREAM_DECODER_VORBIS:
			return ma_libvorbis_seek_to_pcm_frame(&decoder->vorbis, frame);
		case STREAM_DECODER_OPUS:
			return ma_libopus_seek_to_pcm_frame(&decoder->opus, frame);
		default:
			return ma_decoder_seek_to_pcm_frame(&decoder->decoder, frame);
	}
}

ma_result stream_decoder_read(ma_stream_decoder* decoder, float* out, ma_uint64 frames, ma_uint64* framesRead)
{
	if (decoder == NULL || !decoder->initialized || out == NULL)
		return MA_INVALID_ARGS;

	switch (decoder->kind)
	{
		case STREAM_DECODER_VORBIS:
			return ma_libvorbis_read_pcm_frames(&decoder->vorbis, out, frames, framesRead);
		case STREAM_DECODER_OPUS:
			return ma_libopus_read_pcm_frames(&decoder->opus, out, frames, framesRead);
		default:
			return ma_decoder_read_pcm_frames(&decoder->decoder, out, frames, framesRead);
	}
}

void set_last_decoded_format(ma_uint32 channels, ma_uint32 sampleRate, ma_uint64 frameCount)
{
	lastDecodedChannels = (int)channels;
	lastDecodedSampleRate = (int)sampleRate;
	lastDecodedSamples = (int)frameCount;
}

static int is_ogg_stream(const unsigned char* bytes, size_t size)
{
	return size >= 4 && memcmp(bytes, "OggS", 4) == 0;
}

static int read_ogg_packet_signature(const unsigned char* bytes, size_t size, const char* signature, size_t signatureSize)
{
	size_t segmentTableSize;
	size_t packetOffset;

	if (!is_ogg_stream(bytes, size) || size < 27)
		return 0;

	segmentTableSize = bytes[26];
	packetOffset = 27 + segmentTableSize;
	if (packetOffset + signatureSize > size)
		return 0;

	return memcmp(bytes + packetOffset, signature, signatureSize) == 0;
}

int is_opus_stream(const unsigned char* bytes, size_t size)
{
	return read_ogg_packet_signature(bytes, size, "OpusHead", 8);
}

int is_vorbis_stream(const unsigned char* bytes, size_t size)
{
	static const unsigned char signature[] = { 0x01, 'v', 'o', 'r', 'b', 'i', 's' };
	return read_ogg_packet_signature(bytes, size, (const char*)signature, sizeof(signature));
}
