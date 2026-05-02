#include "utils.h"

// ─── DECODE ───────────────────────────────────────────────────────────────────────

static vbyte* copy_pcm_float_bytes(const float* data, ma_uint64 frameCount, ma_uint32 channels)
{
	ma_uint64 byteCount64 = frameCount * channels * (ma_uint64)sizeof(float);
	int byteCount;

	if (byteCount64 == 0 || byteCount64 > 0x7FFFFFFF)
	{
		lastResult = MA_OUT_OF_MEMORY;
		return NULL;
	}

	byteCount = (int)byteCount64;
	return hl_copy_bytes((const vbyte*)data, byteCount);
}

static vbyte* decode_vorbis_to_pcm_float(const unsigned char* bytes, size_t size)
{
	memory_stream stream;
	ma_libvorbis decoder;
	ma_decoding_backend_config config;
	ma_uint64 frameCount = 0;
	ma_uint32 channels = 0;
	ma_uint32 sampleRate = 0;
	ma_uint64 framesRead = 0;
	vbyte* result;

	stream.data = bytes;
	stream.size = size;
	stream.cursor = 0;

	config = ma_decoding_backend_config_init(ma_format_f32, 0);
	lastResult = ma_libvorbis_init(memory_stream_read, memory_stream_seek, memory_stream_tell, &stream, &config, NULL, &decoder);
	if (lastResult != MA_SUCCESS)
		return NULL;

	lastResult = ma_libvorbis_get_data_format(&decoder, NULL, &channels, &sampleRate, NULL, 0);
	if (lastResult != MA_SUCCESS)
	{
		ma_libvorbis_uninit(&decoder, NULL);
		return NULL;
	}

	lastResult = ma_libvorbis_get_length_in_pcm_frames(&decoder, &frameCount);
	if (lastResult != MA_SUCCESS || frameCount == 0)
	{
		ma_libvorbis_uninit(&decoder, NULL);
		lastResult = MA_INVALID_FILE;
		return NULL;
	}

	ma_uint64 byteCount64 = frameCount * channels * sizeof(float);
	if (byteCount64 > 0x7FFFFFFF)
	{
		ma_libvorbis_uninit(&decoder, NULL);
		lastResult = MA_OUT_OF_MEMORY;
		return NULL;
	}

	result = hl_gc_alloc_noptr((int)byteCount64);
	lastResult = ma_libvorbis_read_pcm_frames(&decoder, (float*)result, frameCount, &framesRead);
	ma_libvorbis_uninit(&decoder, NULL);

	if ((lastResult != MA_SUCCESS && lastResult != MA_AT_END) || framesRead == 0)
	{
		return NULL;
	}

	set_last_decoded_format(channels, sampleRate, framesRead);
	return result;
}

static vbyte* decode_vorbis_to_pcm_s16(const unsigned char* bytes, size_t size)
{
	memory_stream stream;
	ma_libvorbis decoder;
	ma_decoding_backend_config config;
	ma_uint64 frameCount = 0;
	ma_uint32 channels = 0;
	ma_uint32 sampleRate = 0;
	ma_uint64 framesRead = 0;
	vbyte* result;

	stream.data = bytes;
	stream.size = size;
	stream.cursor = 0;

	config = ma_decoding_backend_config_init(ma_format_s16, 0);
	lastResult = ma_libvorbis_init(memory_stream_read, memory_stream_seek, memory_stream_tell, &stream, &config, NULL, &decoder);
	if (lastResult != MA_SUCCESS)
		return NULL;

	lastResult = ma_libvorbis_get_data_format(&decoder, NULL, &channels, &sampleRate, NULL, 0);
	if (lastResult != MA_SUCCESS)
	{
		ma_libvorbis_uninit(&decoder, NULL);
		return NULL;
	}

	lastResult = ma_libvorbis_get_length_in_pcm_frames(&decoder, &frameCount);
	if (lastResult != MA_SUCCESS || frameCount == 0)
	{
		ma_libvorbis_uninit(&decoder, NULL);
		lastResult = MA_INVALID_FILE;
		return NULL;
	}

	ma_uint64 byteCount64 = frameCount * channels * sizeof(ma_int16);
	if (byteCount64 > 0x7FFFFFFF)
	{
		ma_libvorbis_uninit(&decoder, NULL);
		lastResult = MA_OUT_OF_MEMORY;
		return NULL;
	}

	result = hl_gc_alloc_noptr((int)byteCount64);
	lastResult = ma_libvorbis_read_pcm_frames(&decoder, (ma_int16*)result, frameCount, &framesRead);
	ma_libvorbis_uninit(&decoder, NULL);

	if ((lastResult != MA_SUCCESS && lastResult != MA_AT_END) || framesRead == 0)
	{
		return NULL;
	}

	set_last_decoded_format(channels, sampleRate, framesRead);
	return result;
}

static vbyte* decode_opus_to_pcm_float(const unsigned char* bytes, size_t size)
{
	memory_stream stream;
	ma_libopus decoder;
	ma_decoding_backend_config config;
	ma_uint64 frameCount = 0;
	ma_uint32 channels = 0;
	ma_uint32 sampleRate = 0;
	ma_uint64 framesRead = 0;
	vbyte* result;

	stream.data = bytes;
	stream.size = size;
	stream.cursor = 0;

	config = ma_decoding_backend_config_init(ma_format_f32, 0);
	lastResult = ma_libopus_init(memory_stream_read, memory_stream_seek, memory_stream_tell, &stream, &config, NULL, &decoder);
	if (lastResult != MA_SUCCESS)
		return NULL;

	lastResult = ma_libopus_get_data_format(&decoder, NULL, &channels, &sampleRate, NULL, 0);
	if (lastResult != MA_SUCCESS)
	{
		ma_libopus_uninit(&decoder, NULL);
		return NULL;
	}

	lastResult = ma_libopus_get_length_in_pcm_frames(&decoder, &frameCount);
	if (lastResult != MA_SUCCESS || frameCount == 0)
	{
		ma_libopus_uninit(&decoder, NULL);
		lastResult = MA_INVALID_FILE;
		return NULL;
	}

	ma_uint64 byteCount64 = frameCount * channels * sizeof(float);
	if (byteCount64 > 0x7FFFFFFF)
	{
		ma_libopus_uninit(&decoder, NULL);
		lastResult = MA_OUT_OF_MEMORY;
		return NULL;
	}

	result = hl_gc_alloc_noptr((int)byteCount64);
	lastResult = ma_libopus_read_pcm_frames(&decoder, (float*)result, frameCount, &framesRead);
	ma_libopus_uninit(&decoder, NULL);

	if ((lastResult != MA_SUCCESS && lastResult != MA_AT_END) || framesRead == 0)
	{
		return NULL;
	}

	set_last_decoded_format(channels, sampleRate, framesRead);
	return result;
}

static vbyte* decode_opus_to_pcm_s16(const unsigned char* bytes, size_t size)
{
	memory_stream stream;
	ma_libopus decoder;
	ma_decoding_backend_config config;
	ma_uint64 frameCount = 0;
	ma_uint32 channels = 0;
	ma_uint32 sampleRate = 0;
	ma_uint64 framesRead = 0;
	vbyte* result;

	stream.data = bytes;
	stream.size = size;
	stream.cursor = 0;

	config = ma_decoding_backend_config_init(ma_format_s16, 0);
	lastResult = ma_libopus_init(memory_stream_read, memory_stream_seek, memory_stream_tell, &stream, &config, NULL, &decoder);
	if (lastResult != MA_SUCCESS)
		return NULL;

	lastResult = ma_libopus_get_data_format(&decoder, NULL, &channels, &sampleRate, NULL, 0);
	if (lastResult != MA_SUCCESS)
	{
		ma_libopus_uninit(&decoder, NULL);
		return NULL;
	}

	lastResult = ma_libopus_get_length_in_pcm_frames(&decoder, &frameCount);
	if (lastResult != MA_SUCCESS || frameCount == 0)
	{
		ma_libopus_uninit(&decoder, NULL);
		lastResult = MA_INVALID_FILE;
		return NULL;
	}

	ma_uint64 byteCount64 = frameCount * channels * sizeof(ma_int16);
	if (byteCount64 > 0x7FFFFFFF)
	{
		ma_libopus_uninit(&decoder, NULL);
		lastResult = MA_OUT_OF_MEMORY;
		return NULL;
	}

	result = hl_gc_alloc_noptr((int)byteCount64);
	lastResult = ma_libopus_read_pcm_frames(&decoder, (ma_int16*)result, frameCount, &framesRead);
	ma_libopus_uninit(&decoder, NULL);

	if ((lastResult != MA_SUCCESS && lastResult != MA_AT_END) || framesRead == 0)
	{
		return NULL;
	}

	set_last_decoded_format(channels, sampleRate, framesRead);
	return result;
}

vbyte* decode_bytes_to_pcm_float(const unsigned char* bytes, size_t size)
{
	ma_decoder decoder;
	ma_decoder_config config;
	ma_uint32 channels;
	ma_uint32 sampleRate;
	ma_uint64 frameCount;
	ma_uint64 framesRead = 0;
	vbyte* result;

	if (bytes == NULL || size == 0)
	{
		lastResult = MA_INVALID_ARGS;
		return NULL;
	}

	if (is_opus_stream(bytes, size))
		return decode_opus_to_pcm_float(bytes, size);

	if (is_vorbis_stream(bytes, size))
		return decode_vorbis_to_pcm_float(bytes, size);

	config = ma_decoder_config_init(ma_format_f32, 0, 0);
	lastResult = ma_decoder_init_memory(bytes, size, &config, &decoder);
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

	ma_uint64 byteCount64 = frameCount * channels * sizeof(float);
	if (byteCount64 > 0x7FFFFFFF)
	{
		ma_decoder_uninit(&decoder);
		lastResult = MA_OUT_OF_MEMORY;
		return NULL;
	}

	result = hl_gc_alloc_noptr((int)byteCount64);
	lastResult = ma_decoder_read_pcm_frames(&decoder, (float*)result, frameCount, &framesRead);
	ma_decoder_uninit(&decoder);

	if ((lastResult != MA_SUCCESS && lastResult != MA_AT_END) || framesRead == 0)
	{
		return NULL;
	}

	set_last_decoded_format(channels, sampleRate, framesRead);
	return result;
}

vbyte* decode_bytes_to_pcm_s16(const unsigned char* bytes, size_t size)
{
	ma_decoder decoder;
	ma_decoder_config config;
	ma_uint32 channels;
	ma_uint32 sampleRate;
	ma_uint64 frameCount;
	ma_uint64 framesRead = 0;
	vbyte* result;

	if (bytes == NULL || size == 0)
	{
		lastResult = MA_INVALID_ARGS;
		return NULL;
	}

	if (is_opus_stream(bytes, size))
		return decode_opus_to_pcm_s16(bytes, size);

	if (is_vorbis_stream(bytes, size))
		return decode_vorbis_to_pcm_s16(bytes, size);

	config = ma_decoder_config_init(ma_format_s16, 0, 0);
	lastResult = ma_decoder_init_memory(bytes, size, &config, &decoder);
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

	ma_uint64 byteCount64 = frameCount * channels * sizeof(ma_int16);
	if (byteCount64 > 0x7FFFFFFF)
	{
		ma_decoder_uninit(&decoder);
		lastResult = MA_OUT_OF_MEMORY;
		return NULL;
	}

	result = hl_gc_alloc_noptr((int)byteCount64);
	lastResult = ma_decoder_read_pcm_frames(&decoder, (ma_int16*)result, frameCount, &framesRead);
	ma_decoder_uninit(&decoder);

	if ((lastResult != MA_SUCCESS && lastResult != MA_AT_END) || framesRead == 0)
	{
		return NULL;
	}

	set_last_decoded_format(channels, sampleRate, framesRead);
	return result;
}

static ma_buffer_handle* decode_vorbis_from_memory(const unsigned char* bytes, size_t size)
{
	vbyte* pcmBytes = decode_vorbis_to_pcm_float(bytes, size);
	if (pcmBytes == NULL)
		return NULL;

	return HL_NAME(buffer_from_pcm_float)(pcmBytes, lastDecodedSamples * lastDecodedChannels * (int)sizeof(float), lastDecodedChannels, lastDecodedSampleRate);
}

static ma_buffer_handle* decode_opus_from_memory(const unsigned char* bytes, size_t size)
{
	vbyte* pcmBytes = decode_opus_to_pcm_float(bytes, size);
	if (pcmBytes == NULL)
		return NULL;

	return HL_NAME(buffer_from_pcm_float)(pcmBytes, lastDecodedSamples * lastDecodedChannels * (int)sizeof(float), lastDecodedChannels, lastDecodedSampleRate);
}

static float* decode_vorbis_float_raw(const unsigned char* bytes, size_t size, ma_uint32* channels, ma_uint32* sampleRate, ma_uint64* framesRead)
{
	memory_stream stream;
	ma_libvorbis decoder;
	ma_decoding_backend_config config;
	ma_uint64 frameCount = 0;
	float* data;

	stream.data = bytes;
	stream.size = size;
	stream.cursor = 0;

	config = ma_decoding_backend_config_init(ma_format_f32, 0);
	lastResult = ma_libvorbis_init(memory_stream_read, memory_stream_seek, memory_stream_tell, &stream, &config, NULL, &decoder);
	if (lastResult != MA_SUCCESS)
		return NULL;

	lastResult = ma_libvorbis_get_data_format(&decoder, NULL, channels, sampleRate, NULL, 0);
	if (lastResult != MA_SUCCESS)
	{
		ma_libvorbis_uninit(&decoder, NULL);
		return NULL;
	}

	lastResult = ma_libvorbis_get_length_in_pcm_frames(&decoder, &frameCount);
	if (lastResult != MA_SUCCESS || frameCount == 0)
	{
		ma_libvorbis_uninit(&decoder, NULL);
		lastResult = MA_INVALID_FILE;
		return NULL;
	}

	data = (float*)ma_malloc((size_t)(frameCount * (*channels) * sizeof(float)), NULL);
	if (data == NULL)
	{
		ma_libvorbis_uninit(&decoder, NULL);
		lastResult = MA_OUT_OF_MEMORY;
		return NULL;
	}

	lastResult = ma_libvorbis_read_pcm_frames(&decoder, data, frameCount, framesRead);
	ma_libvorbis_uninit(&decoder, NULL);
	if ((lastResult != MA_SUCCESS && lastResult != MA_AT_END) || *framesRead == 0)
	{
		ma_free(data, NULL);
		return NULL;
	}

	return data;
}

static float* decode_opus_float_raw(const unsigned char* bytes, size_t size, ma_uint32* channels, ma_uint32* sampleRate, ma_uint64* framesRead)
{
	memory_stream stream;
	ma_libopus decoder;
	ma_decoding_backend_config config;
	ma_uint64 frameCount = 0;
	float* data;

	stream.data = bytes;
	stream.size = size;
	stream.cursor = 0;

	config = ma_decoding_backend_config_init(ma_format_f32, 0);
	lastResult = ma_libopus_init(memory_stream_read, memory_stream_seek, memory_stream_tell, &stream, &config, NULL, &decoder);
	if (lastResult != MA_SUCCESS)
		return NULL;

	lastResult = ma_libopus_get_data_format(&decoder, NULL, channels, sampleRate, NULL, 0);
	if (lastResult != MA_SUCCESS)
	{
		ma_libopus_uninit(&decoder, NULL);
		return NULL;
	}

	lastResult = ma_libopus_get_length_in_pcm_frames(&decoder, &frameCount);
	if (lastResult != MA_SUCCESS || frameCount == 0)
	{
		ma_libopus_uninit(&decoder, NULL);
		lastResult = MA_INVALID_FILE;
		return NULL;
	}

	data = (float*)ma_malloc((size_t)(frameCount * (*channels) * sizeof(float)), NULL);
	if (data == NULL)
	{
		ma_libopus_uninit(&decoder, NULL);
		lastResult = MA_OUT_OF_MEMORY;
		return NULL;
	}

	lastResult = ma_libopus_read_pcm_frames(&decoder, data, frameCount, framesRead);
	ma_libopus_uninit(&decoder, NULL);
	if ((lastResult != MA_SUCCESS && lastResult != MA_AT_END) || *framesRead == 0)
	{
		ma_free(data, NULL);
		return NULL;
	}

	return data;
}

ma_buffer_handle* decode_bytes_to_buffer_direct(const unsigned char* bytes, size_t size)
{
	ma_decoder decoder;
	ma_decoder_config config;
	ma_uint32 channels;
	ma_uint32 sampleRate;
	ma_uint64 frameCount;
	ma_uint64 framesRead = 0;
	float* data;
	ma_buffer_handle* buffer;

	if (is_opus_stream(bytes, size))
	{
		data = decode_opus_float_raw(bytes, size, &channels, &sampleRate, &framesRead);
		if (data == NULL)
			return NULL;
		buffer = create_buffer_from_pcm(data, framesRead, channels, sampleRate);
		if (buffer != NULL)
			set_last_decoded_format(channels, sampleRate, framesRead);
		return buffer;
	}

	if (is_vorbis_stream(bytes, size))
	{
		data = decode_vorbis_float_raw(bytes, size, &channels, &sampleRate, &framesRead);
		if (data == NULL)
			return NULL;
		buffer = create_buffer_from_pcm(data, framesRead, channels, sampleRate);
		if (buffer != NULL)
			set_last_decoded_format(channels, sampleRate, framesRead);
		return buffer;
	}

	config = ma_decoder_config_init(ma_format_f32, 0, 0);
	lastResult = ma_decoder_init_memory(bytes, size, &config, &decoder);
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

HL_PRIM ma_buffer_handle* HL_NAME(buffer_from_pcm_float)(vbyte* bytes, int size, int channels, int sampleRate);
