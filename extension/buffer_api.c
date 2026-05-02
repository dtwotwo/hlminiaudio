#include "utils.h"

// ─── BUFFER ───────────────────────────────────────────────────────────────────────

HL_PRIM void HL_NAME(buffer_dispose)(ma_buffer_handle* buffer)
{
	buffer_handle_release(buffer, 0);
}
DEFINE_PRIM(_VOID, buffer_dispose, _BUFFER);

HL_PRIM ma_buffer_handle* HL_NAME(buffer_from_bytes)(vbyte* bytes, int size)
{
	if (bytes == NULL || size <= 0)
	{
		lastResult = MA_INVALID_ARGS;
		return NULL;
	}

	return decode_bytes_to_buffer_direct((const unsigned char*)bytes, (size_t)size);
}
DEFINE_PRIM(_BUFFER, buffer_from_bytes, _BYTES _I32);

HL_PRIM ma_buffer_handle* HL_NAME(buffer_from_file)(vbyte* path)
{
	ma_decoder decoder;
	ma_decoder_config config;
	ma_uint32 channels;
	ma_uint32 sampleRate;
	ma_uint64 frameCount;
	ma_uint64 framesRead = 0;
	float* data;
	ma_buffer_handle* buffer;
	char* cpath = hl_to_utf8((uchar*)path);

	config = ma_decoder_config_init(ma_format_f32, 0, 0);
	lastResult = ma_decoder_init_file(cpath, &config, &decoder);
	if (lastResult != MA_SUCCESS)
		return NULL;

	channels = decoder.outputChannels;
	sampleRate = decoder.outputSampleRate;

	lastResult = ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount);
	if (lastResult != MA_SUCCESS || frameCount == 0)
	{
		ma_decoder_uninit(&decoder);
		lastResult = MA_INVALID_FILE;
		return NULL;
	}

	data = (float*)ma_malloc((size_t)(frameCount * channels * sizeof(float)), NULL);
	if (data == NULL)
	{
		ma_decoder_uninit(&decoder);
		lastResult = MA_OUT_OF_MEMORY;
		return NULL;
	}

	lastResult = ma_decoder_read_pcm_frames(&decoder, data, frameCount, &framesRead);
	ma_decoder_uninit(&decoder);
	if ((lastResult != MA_SUCCESS && lastResult != MA_AT_END) || framesRead == 0)
	{
		ma_free(data, NULL);
		return NULL;
	}

	buffer = create_buffer_from_pcm(data, framesRead, channels, sampleRate);
	if (buffer != NULL)
		set_last_decoded_format(channels, sampleRate, framesRead);

	return buffer;
}
DEFINE_PRIM(_BUFFER, buffer_from_file, _BYTES);

HL_PRIM vbyte* HL_NAME(decode_pcm_float)(vbyte* bytes, int size)
{
	return decode_bytes_to_pcm_float((const unsigned char*)bytes, (size_t)size);
}
DEFINE_PRIM(_BYTES, decode_pcm_float, _BYTES _I32);

HL_PRIM vbyte* HL_NAME(decode_pcm_s16)(vbyte* bytes, int size)
{
	return decode_bytes_to_pcm_s16((const unsigned char*)bytes, (size_t)size);
}
DEFINE_PRIM(_BYTES, decode_pcm_s16, _BYTES _I32);

HL_PRIM int HL_NAME(decoded_channels)()
{
	return lastDecodedChannels;
}
DEFINE_PRIM(_I32, decoded_channels, _NO_ARG);

HL_PRIM int HL_NAME(decoded_sample_rate)()
{
	return lastDecodedSampleRate;
}
DEFINE_PRIM(_I32, decoded_sample_rate, _NO_ARG);

HL_PRIM int HL_NAME(decoded_samples)()
{
	return lastDecodedSamples;
}
DEFINE_PRIM(_I32, decoded_samples, _NO_ARG);


// ===== BUFFER PCM FACTORIES =====
HL_PRIM ma_buffer_handle* HL_NAME(buffer_from_pcm_float)(vbyte* bytes, int size, int channels, int sampleRate)
{
	if (bytes == NULL || size <= 0 || channels <= 0 || sampleRate <= 0)
	{
		lastResult = MA_INVALID_ARGS;
		return NULL;
	}

	int frameSize = channels * (int)sizeof(float);
	if (frameSize <= 0 || (size % frameSize) != 0)
	{
		lastResult = MA_INVALID_ARGS;
		return NULL;
	}

	ma_uint64 frameCount = (ma_uint64)(size / frameSize);
	float* data = (float*)ma_malloc((size_t)size, NULL);
	if (data == NULL)
	{
		lastResult = MA_OUT_OF_MEMORY;
		return NULL;
	}

	memcpy(data, bytes, (size_t)size);

	ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(
		ma_format_f32,
		(ma_uint32)channels,
		frameCount,
		data,
		NULL
	);

	ma_audio_buffer* buffer;
	lastResult = ma_audio_buffer_alloc_and_init(&bufferConfig, &buffer);
	if (lastResult == MA_SUCCESS)
	{
		buffer->ref.sampleRate = (ma_uint32)sampleRate;
		return buffer_handle_alloc(buffer, data);
	}

	ma_free(data, NULL);
	return NULL;
}
DEFINE_PRIM(_BUFFER, buffer_from_pcm_float, _BYTES _I32 _I32 _I32);

HL_PRIM ma_buffer_handle* HL_NAME(buffer_from_pcm_s16)(vbyte* bytes, int size, int channels, int sampleRate)
{
	if (bytes == NULL || size <= 0 || channels <= 0 || sampleRate <= 0)
	{
		lastResult = MA_INVALID_ARGS;
		return NULL;
	}

	int frameSize = channels * (int)sizeof(ma_int16);
	if (frameSize <= 0 || (size % frameSize) != 0)
	{
		lastResult = MA_INVALID_ARGS;
		return NULL;
	}

	ma_uint64 frameCount = (ma_uint64)(size / frameSize);
	ma_int16* data = (ma_int16*)ma_malloc((size_t)size, NULL);
	if (data == NULL)
	{
		lastResult = MA_OUT_OF_MEMORY;
		return NULL;
	}

	memcpy(data, bytes, (size_t)size);

	ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(
		ma_format_s16,
		(ma_uint32)channels,
		frameCount,
		data,
		NULL
	);

	ma_audio_buffer* buffer;
	lastResult = ma_audio_buffer_alloc_and_init(&bufferConfig, &buffer);
	if (lastResult == MA_SUCCESS)
	{
		buffer->ref.sampleRate = (ma_uint32)sampleRate;
		return buffer_handle_alloc(buffer, data);
	}

	ma_free(data, NULL);
	return NULL;
}
DEFINE_PRIM(_BUFFER, buffer_from_pcm_s16, _BYTES _I32 _I32 _I32);

HL_PRIM int HL_NAME(buffer_get_length_samples)(ma_buffer_handle* buffer)
{
	ma_uint64 length = 0;
	if (buffer == NULL || buffer->buffer == NULL)
	{
		lastResult = MA_INVALID_ARGS;
		return 0;
	}
	lastResult = ma_audio_buffer_get_length_in_pcm_frames(buffer->buffer, &length);
	return lastResult == MA_SUCCESS ? (int)length : 0;
}
DEFINE_PRIM(_I32, buffer_get_length_samples, _BUFFER);

HL_PRIM double HL_NAME(buffer_get_duration)(ma_buffer_handle* buffer)
{
	ma_uint64 length = 0;

	if (buffer == NULL || buffer->buffer == NULL)
	{
		lastResult = MA_INVALID_ARGS;
		return 0;
	}

	lastResult = ma_audio_buffer_get_length_in_pcm_frames(buffer->buffer, &length);
	if (lastResult != MA_SUCCESS || buffer->buffer->ref.sampleRate == 0)
		return 0;

	return ((double)length * 1000.0) / (double)buffer->buffer->ref.sampleRate;
}
DEFINE_PRIM(_F64, buffer_get_duration, _BUFFER);

HL_PRIM double HL_NAME(buffer_get_duration_seconds)(ma_buffer_handle* buffer)
{
	ma_uint64 length = 0;

	if (buffer == NULL || buffer->buffer == NULL)
	{
		lastResult = MA_INVALID_ARGS;
		return 0;
	}

	lastResult = ma_audio_buffer_get_length_in_pcm_frames(buffer->buffer, &length);
	if (lastResult != MA_SUCCESS || buffer->buffer->ref.sampleRate == 0)
		return 0;

	return (double)length / (double)buffer->buffer->ref.sampleRate;
}
DEFINE_PRIM(_F64, buffer_get_duration_seconds, _BUFFER);
