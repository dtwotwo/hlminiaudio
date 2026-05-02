#include "utils.h"

// ─── STREAM DECODER ──────────────────────────────────────

HL_PRIM ma_stream_decoder* HL_NAME(stream_open)(vbyte* bytes, int size)
{
	ma_stream_decoder* decoder;
	ma_decoding_backend_config backendConfig;
	ma_decoder_config decoderConfig;

	if (bytes == NULL || size <= 0)
	{
		lastResult = MA_INVALID_ARGS;
		return NULL;
	}

	decoder = (ma_stream_decoder*)hl_gc_alloc_finalizer(sizeof(ma_stream_decoder));
	memset(decoder, 0, sizeof(ma_stream_decoder));
	decoder->finalize = stream_decoder_finalize;
	decoder->bytes = bytes;
	hl_add_root(&decoder->bytes);
	decoder->stream.data = (const unsigned char*)bytes;
	decoder->stream.size = (size_t)size;
	decoder->stream.cursor = 0;

	if (is_opus_stream((const unsigned char*)bytes, (size_t)size))
	{
		decoder->kind = STREAM_DECODER_OPUS;
		backendConfig = ma_decoding_backend_config_init(ma_format_f32, 0);
		lastResult = ma_libopus_init(memory_stream_read, memory_stream_seek, memory_stream_tell, &decoder->stream, &backendConfig, NULL, &decoder->opus);
		if (lastResult != MA_SUCCESS)
		{
			hl_remove_root(&decoder->bytes);
			decoder->bytes = NULL;
			return NULL;
		}

		decoder->initialized = 1;
		lastResult = ma_libopus_get_data_format(&decoder->opus, NULL, &decoder->channels, &decoder->sampleRate, NULL, 0);
		if (lastResult == MA_SUCCESS)
			lastResult = ma_libopus_get_length_in_pcm_frames(&decoder->opus, &decoder->frameCount);
	}
	else if (is_vorbis_stream((const unsigned char*)bytes, (size_t)size))
	{
		decoder->kind = STREAM_DECODER_VORBIS;
		backendConfig = ma_decoding_backend_config_init(ma_format_f32, 0);
		lastResult = ma_libvorbis_init(memory_stream_read, memory_stream_seek, memory_stream_tell, &decoder->stream, &backendConfig, NULL, &decoder->vorbis);
		if (lastResult != MA_SUCCESS)
		{
			hl_remove_root(&decoder->bytes);
			decoder->bytes = NULL;
			return NULL;
		}

		decoder->initialized = 1;
		lastResult = ma_libvorbis_get_data_format(&decoder->vorbis, NULL, &decoder->channels, &decoder->sampleRate, NULL, 0);
		if (lastResult == MA_SUCCESS)
			lastResult = ma_libvorbis_get_length_in_pcm_frames(&decoder->vorbis, &decoder->frameCount);
	}
	else
	{
		decoder->kind = STREAM_DECODER_MINIAUDIO;
		decoderConfig = ma_decoder_config_init(ma_format_f32, 0, 0);
		lastResult = ma_decoder_init_memory(bytes, (size_t)size, &decoderConfig, &decoder->decoder);
		if (lastResult != MA_SUCCESS)
		{
			hl_remove_root(&decoder->bytes);
			decoder->bytes = NULL;
			return NULL;
		}

		decoder->initialized = 1;
		decoder->channels = decoder->decoder.outputChannels;
		decoder->sampleRate = decoder->decoder.outputSampleRate;
		lastResult = ma_decoder_get_length_in_pcm_frames(&decoder->decoder, &decoder->frameCount);
	}

	if (lastResult != MA_SUCCESS || decoder->channels == 0 || decoder->sampleRate == 0 || decoder->frameCount == 0)
	{
		stream_decoder_finalize(decoder);
		if (lastResult == MA_SUCCESS)
			lastResult = MA_INVALID_FILE;
		return NULL;
	}

	return decoder;
}
DEFINE_PRIM(_DECODER, stream_open, _BYTES _I32);

HL_PRIM ma_stream_decoder* HL_NAME(stream_open_file)(vbyte* path)
{
	ma_stream_decoder* decoder;
	ma_decoder_config decoderConfig;
	char* cpath = hl_to_utf8((uchar*)path);

	decoder = (ma_stream_decoder*)hl_gc_alloc_finalizer(sizeof(ma_stream_decoder));
	memset(decoder, 0, sizeof(ma_stream_decoder));
	decoder->finalize = stream_decoder_finalize;
	decoder->kind = STREAM_DECODER_FILE;

	decoderConfig = ma_decoder_config_init(ma_format_f32, 0, 0);
	lastResult = ma_decoder_init_file(cpath, &decoderConfig, &decoder->decoder);
	if (lastResult != MA_SUCCESS)
	{
		return NULL;
	}

	decoder->initialized = 1;
	decoder->channels = decoder->decoder.outputChannels;
	decoder->sampleRate = decoder->decoder.outputSampleRate;
	lastResult = ma_decoder_get_length_in_pcm_frames(&decoder->decoder, &decoder->frameCount);

	if (lastResult != MA_SUCCESS || decoder->channels == 0 || decoder->sampleRate == 0 || decoder->frameCount == 0)
	{
		stream_decoder_finalize(decoder);
		if (lastResult == MA_SUCCESS)
			lastResult = MA_INVALID_FILE;
		return NULL;
	}

	return decoder;
}
DEFINE_PRIM(_DECODER, stream_open_file, _BYTES);

HL_PRIM void HL_NAME(stream_dispose)(ma_stream_decoder* decoder)
{
	stream_decoder_release(decoder, 0);
}
DEFINE_PRIM(_VOID, stream_dispose, _DECODER);

HL_PRIM int HL_NAME(stream_channels)(ma_stream_decoder* decoder)
{
	return decoder == NULL ? 0 : (int)decoder->channels;
}
DEFINE_PRIM(_I32, stream_channels, _DECODER);

HL_PRIM int HL_NAME(stream_sample_rate)(ma_stream_decoder* decoder)
{
	return decoder == NULL ? 0 : (int)decoder->sampleRate;
}
DEFINE_PRIM(_I32, stream_sample_rate, _DECODER);

HL_PRIM int HL_NAME(stream_samples)(ma_stream_decoder* decoder)
{
	return decoder == NULL ? 0 : (int)decoder->frameCount;
}
DEFINE_PRIM(_I32, stream_samples, _DECODER);

HL_PRIM int HL_NAME(stream_decode)(ma_stream_decoder* decoder, vbyte* out, int outPos, int sampleStart, int sampleCount)
{
	ma_uint64 framesRead = 0;

	if (decoder == NULL || out == NULL || outPos < 0 || sampleStart < 0 || sampleCount < 0)
	{
		lastResult = MA_INVALID_ARGS;
		return 0;
	}

	lastResult = stream_decoder_seek(decoder, (ma_uint64)sampleStart);
	if (lastResult != MA_SUCCESS)
		return 0;

	lastResult = stream_decoder_read(decoder, (float*)(out + outPos), (ma_uint64)sampleCount, &framesRead);
	if (lastResult != MA_SUCCESS && lastResult != MA_AT_END)
		return (int)framesRead;

	return (int)framesRead;
}
DEFINE_PRIM(_I32, stream_decode, _DECODER _BYTES _I32 _I32 _I32);

HL_PRIM void HL_NAME(gc)()
{
	hl_gc_major();
}
DEFINE_PRIM(_VOID, gc, _NO_ARG);
