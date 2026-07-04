#include "utils.h"

static size_t pcm_sink_sample_count(ma_pcm_sink_handle* sink, size_t frames)
{
	return frames * (size_t)sink->channels;
}

static void pcm_sink_data_callback(ma_device* device, void* output, const void* input, ma_uint32 frameCount)
{
	ma_pcm_sink_handle* sink = (ma_pcm_sink_handle*)device->pUserData;
	float* out = (float*)output;
	ma_uint32 readFrames = 0;

	(void)input;
	ma_silence_pcm_frames(out, frameCount, ma_format_f32, sink->channels);

	am_mutex_lock(&sink->mutex);
	if (!sink->paused)
	{
		while (readFrames < frameCount && sink->bufferedFrames > 0)
		{
			size_t frames = frameCount - readFrames;
			size_t availableBeforeWrap = sink->capacityFrames - sink->readFrame;
			size_t sampleCount;
			float* target;
			float* source;
			size_t i;

			if (frames > sink->bufferedFrames)
				frames = sink->bufferedFrames;
			if (frames > availableBeforeWrap)
				frames = availableBeforeWrap;

			sampleCount = pcm_sink_sample_count(sink, frames);
			source = sink->samples + pcm_sink_sample_count(sink, sink->readFrame);
			target = out + pcm_sink_sample_count(sink, readFrames);
			for (i = 0; i < sampleCount; ++i)
				target[i] = source[i] * sink->volume;

			sink->readFrame = (sink->readFrame + frames) % sink->capacityFrames;
			sink->bufferedFrames -= frames;
			readFrames += (ma_uint32)frames;
		}
		sink->playedFrames += readFrames;
	}
	am_mutex_unlock(&sink->mutex);
}

static void pcm_sink_release(ma_pcm_sink_handle* sink)
{
	if (sink == NULL || !sink->initialized)
		return;

	ma_device_uninit(&sink->device);
	am_mutex_uninit(&sink->mutex);
	ma_free(sink->samples, NULL);
	sink->samples = NULL;
	sink->initialized = 0;
	sink->finalize = NULL;
}

static void pcm_sink_finalize(ma_pcm_sink_handle* sink)
{
	pcm_sink_release(sink);
}

HL_PRIM ma_pcm_sink_handle* HL_NAME(pcm_sink_init)(int sampleRate, int channels, int bufferFrames)
{
	ma_pcm_sink_handle* sink;
	ma_device_config config;
	size_t capacityFrames;

	if (sampleRate <= 0)
		sampleRate = 48000;
	if (channels <= 0)
		channels = 2;

	capacityFrames = bufferFrames > 0 ? (size_t)bufferFrames : (size_t)sampleRate * 2;
	if (capacityFrames == 0)
		return NULL;

	sink = (ma_pcm_sink_handle*)hl_gc_alloc_finalizer(sizeof(ma_pcm_sink_handle));
	memset(sink, 0, sizeof(ma_pcm_sink_handle));
	sink->finalize = pcm_sink_finalize;
	sink->sampleRate = (ma_uint32)sampleRate;
	sink->channels = (ma_uint32)channels;
	sink->capacityFrames = capacityFrames;
	sink->volume = 1.0f;
	sink->paused = 1;
	sink->samples = (float*)ma_malloc(pcm_sink_sample_count(sink, capacityFrames) * sizeof(float), NULL);
	if (sink->samples == NULL)
		return NULL;

	am_mutex_init(&sink->mutex);

	config = ma_device_config_init(ma_device_type_playback);
	config.playback.format = ma_format_f32;
	config.playback.channels = sink->channels;
	config.sampleRate = sink->sampleRate;
	config.dataCallback = pcm_sink_data_callback;
	config.pUserData = sink;
	config.performanceProfile = ma_performance_profile_low_latency;

	lastResult = ma_device_init(NULL, &config, &sink->device);
	if (lastResult != MA_SUCCESS)
	{
		am_mutex_uninit(&sink->mutex);
		ma_free(sink->samples, NULL);
		sink->samples = NULL;
		return NULL;
	}

	lastResult = ma_device_start(&sink->device);
	if (lastResult != MA_SUCCESS)
	{
		ma_device_uninit(&sink->device);
		am_mutex_uninit(&sink->mutex);
		ma_free(sink->samples, NULL);
		sink->samples = NULL;
		return NULL;
	}

	sink->initialized = 1;
	return sink;
}
DEFINE_PRIM(_PCM_SINK, pcm_sink_init, _I32 _I32 _I32);

HL_PRIM void HL_NAME(pcm_sink_dispose)(ma_pcm_sink_handle* sink)
{
	pcm_sink_release(sink);
}
DEFINE_PRIM(_VOID, pcm_sink_dispose, _PCM_SINK);

HL_PRIM int HL_NAME(pcm_sink_write_float)(ma_pcm_sink_handle* sink, vbyte* bytes, int frames)
{
	const float* input;
	size_t writeFrames;
	size_t written = 0;

	if (sink == NULL || !sink->initialized || bytes == NULL || frames <= 0)
		return 0;

	input = (const float*)bytes;
	am_mutex_lock(&sink->mutex);
	writeFrames = (size_t)frames;
	if (writeFrames > sink->capacityFrames - sink->bufferedFrames)
		writeFrames = sink->capacityFrames - sink->bufferedFrames;

	while (written < writeFrames)
	{
		size_t framesNow = writeFrames - written;
		size_t availableBeforeWrap = sink->capacityFrames - sink->writeFrame;
		if (framesNow > availableBeforeWrap)
			framesNow = availableBeforeWrap;

		memcpy(
			sink->samples + pcm_sink_sample_count(sink, sink->writeFrame),
			input + pcm_sink_sample_count(sink, written),
			pcm_sink_sample_count(sink, framesNow) * sizeof(float)
		);

		sink->writeFrame = (sink->writeFrame + framesNow) % sink->capacityFrames;
		sink->bufferedFrames += framesNow;
		written += framesNow;
	}
	am_mutex_unlock(&sink->mutex);

	return (int)writeFrames;
}
DEFINE_PRIM(_I32, pcm_sink_write_float, _PCM_SINK _BYTES _I32);

HL_PRIM int HL_NAME(pcm_sink_buffered_frames)(ma_pcm_sink_handle* sink)
{
	int frames;

	if (sink == NULL || !sink->initialized)
		return 0;

	am_mutex_lock(&sink->mutex);
	frames = (int)sink->bufferedFrames;
	am_mutex_unlock(&sink->mutex);
	return frames;
}
DEFINE_PRIM(_I32, pcm_sink_buffered_frames, _PCM_SINK);

HL_PRIM double HL_NAME(pcm_sink_played_frames)(ma_pcm_sink_handle* sink)
{
	double frames;

	if (sink == NULL || !sink->initialized)
		return 0.0;

	am_mutex_lock(&sink->mutex);
	frames = sink->playedFrames;
	am_mutex_unlock(&sink->mutex);
	return frames;
}
DEFINE_PRIM(_F64, pcm_sink_played_frames, _PCM_SINK);

HL_PRIM void HL_NAME(pcm_sink_pause)(ma_pcm_sink_handle* sink, bool paused)
{
	if (sink == NULL || !sink->initialized)
		return;

	am_mutex_lock(&sink->mutex);
	sink->paused = paused;
	am_mutex_unlock(&sink->mutex);
}
DEFINE_PRIM(_VOID, pcm_sink_pause, _PCM_SINK _BOOL);

HL_PRIM void HL_NAME(pcm_sink_flush)(ma_pcm_sink_handle* sink)
{
	if (sink == NULL || !sink->initialized)
		return;

	am_mutex_lock(&sink->mutex);
	sink->readFrame = 0;
	sink->writeFrame = 0;
	sink->bufferedFrames = 0;
	sink->playedFrames = 0.0;
	am_mutex_unlock(&sink->mutex);
}
DEFINE_PRIM(_VOID, pcm_sink_flush, _PCM_SINK);

HL_PRIM void HL_NAME(pcm_sink_set_volume)(ma_pcm_sink_handle* sink, double volume)
{
	if (sink == NULL || !sink->initialized)
		return;

	if (volume < 0.0)
		volume = 0.0;
	else if (volume > 1.0)
		volume = 1.0;

	am_mutex_lock(&sink->mutex);
	sink->volume = (float)volume;
	am_mutex_unlock(&sink->mutex);
}
DEFINE_PRIM(_VOID, pcm_sink_set_volume, _PCM_SINK _F64);
