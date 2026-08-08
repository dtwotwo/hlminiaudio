#include "utils.h"

// ─── DEVICE ──────────────────────────────────────

static ma_device* device_get_engine_device()
{
	if (!engine_initialized)
		return NULL;

	return ma_engine_get_device(&engine);
}

static double device_get_latency_milliseconds(ma_device* device, ma_device_type type)
{
	ma_uint32 periodSizeInFrames;
	ma_uint32 sampleRate;

	if (device == NULL)
		return 0.0;

	switch (type)
	{
		case ma_device_type_playback:
			periodSizeInFrames = device->playback.internalPeriodSizeInFrames;
			sampleRate = device->playback.internalSampleRate;
			break;

		case ma_device_type_capture:
			periodSizeInFrames = device->capture.internalPeriodSizeInFrames;
			sampleRate = device->capture.internalSampleRate;
			break;

		default:
			return 0.0;
	}

	if (periodSizeInFrames == 0 || sampleRate == 0)
		return 0.0;

	return ((double)periodSizeInFrames * 1000.0) / (double)sampleRate;
}

HL_PRIM double HL_NAME(device_get_playback_latency)()
{
	return device_get_latency_milliseconds(
		device_get_engine_device(),
		ma_device_type_playback
	);
}
DEFINE_PRIM(_F64, device_get_playback_latency, _NO_ARG);
