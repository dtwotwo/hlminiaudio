#include "utils.h"

// ─── SOUND ──────────────────────────────────────

static ma_sound* sound_ptr(ma_sound_handle* sound)
{
	return sound == NULL || !sound->initialized ? NULL : &sound->sound;
}

static void sound_handle_release(ma_sound_handle* sound)
{
	ma_buffer_handle* buffer;
	ma_stream_decoder* stream;
	ma_group_handle* parent;

	if (sound == NULL || !sound->initialized)
		return;

	lastResult = ma_sound_set_end_callback(&sound->sound, NULL, NULL);

	am_mutex_lock(&callback_mutex);
	if (sound->callback_entry != NULL)
	{
		sound_callback_entry* entry = sound->callback_entry;
		if (entry->prev != NULL)
			entry->prev->next = entry->next;
		else
			sound_callbacks = entry->next;

		if (entry->next != NULL)
			entry->next->prev = entry->prev;

		hl_remove_root(&entry->callback);
		free(entry);
		sound->callback_entry = NULL;
	}
	am_mutex_unlock(&callback_mutex);

	ma_sound_uninit(&sound->sound);
	sound->initialized = 0;
	sound->finalize = NULL;

	buffer = sound->buffer;
	if (buffer != NULL)
	{
		sound->buffer = NULL;
		hl_remove_root(&sound->buffer);
		if (buffer->refCount > 0)
			buffer->refCount--;
		if (buffer->disposeRequested && buffer->refCount == 0)
			buffer_handle_release(buffer, 0);
	}

	stream = sound->stream;
	if (stream != NULL)
	{
		sound->stream = NULL;
		hl_remove_root(&sound->stream);
		if (stream->refCount > 0)
			stream->refCount--;
		if (stream->disposeRequested && stream->refCount == 0)
			stream_decoder_release(stream, 0);
	}

	parent = sound->parent;
	if (parent != NULL)
	{
		sound->parent = NULL;
		hl_remove_root(&sound->parent);
		if (parent->refCount > 0)
			parent->refCount--;
		if (parent->disposeRequested && parent->refCount == 0)
			group_handle_release(parent, 0);
	}
}

HL_PRIM void HL_NAME(sound_dispose)(ma_sound_handle* sound)
{
	sound_handle_release(sound);
}
DEFINE_PRIM(_VOID, sound_dispose, _SOUND);

static sound_callback_entry* sound_get_callback_entry(ma_sound_handle* sound, bool create)
{
	sound_callback_entry* entry;

	am_mutex_lock(&callback_mutex);
	entry = sound->callback_entry;
	if (entry != NULL || !create)
	{
		am_mutex_unlock(&callback_mutex);
		return entry;
	}

	entry = (sound_callback_entry*)calloc(1, sizeof(sound_callback_entry));
	entry->sound = &sound->sound;
	hl_add_root(&entry->callback);

	entry->next = sound_callbacks;
	if (sound_callbacks != NULL)
		sound_callbacks->prev = entry;
	sound_callbacks = entry;
	sound->callback_entry = entry;
	am_mutex_unlock(&callback_mutex);

	return entry;
}

static void sound_end_callback(void* pUserData, ma_sound* pSound)
{
	sound_callback_entry* entry = (sound_callback_entry*)pUserData;
	if (entry != NULL && entry->sound == pSound && entry->callback != NULL)
		entry->pending = 1;
}

HL_PRIM ma_sound_handle* HL_NAME(sound_init)(ma_buffer_handle* buffer, ma_group_handle* parent)
{
	ma_sound_handle* sound;

	if (buffer == NULL || buffer->buffer == NULL)
	{
		lastResult = MA_INVALID_ARGS;
		return NULL;
	}

	sound = (ma_sound_handle*)hl_gc_alloc_finalizer(sizeof(ma_sound_handle));
	memset(sound, 0, sizeof(ma_sound_handle));
	sound->finalize = sound_handle_release;

	lastResult = ma_sound_init_from_data_source(&engine, buffer->buffer, 0, group_ptr(parent), &sound->sound);

	if (lastResult == MA_SUCCESS)
	{
		sound->buffer = buffer;
		sound->parent = parent;
		buffer->refCount++;
		hl_add_root(&sound->buffer);
		if (parent != NULL)
		{
			group_handle_ref(parent);
			hl_add_root(&sound->parent);
		}
		sound->initialized = 1;
		return sound;
	}
	else
	{
		sound->finalize = NULL;
		return NULL;
	}
}
DEFINE_PRIM(_SOUND, sound_init, _BUFFER _GROUP);

HL_PRIM ma_sound_handle* HL_NAME(sound_init_stream)(ma_stream_decoder* stream, ma_group_handle* parent)
{
	ma_sound_handle* sound;
	ma_data_source* dataSource;

	if (stream == NULL || !stream->initialized)
	{
		lastResult = MA_INVALID_ARGS;
		return NULL;
	}

	dataSource = stream_decoder_data_source(stream);
	if (dataSource == NULL)
	{
		lastResult = MA_INVALID_ARGS;
		return NULL;
	}

	sound = (ma_sound_handle*)hl_gc_alloc_finalizer(sizeof(ma_sound_handle));
	memset(sound, 0, sizeof(ma_sound_handle));
	sound->finalize = sound_handle_release;

	lastResult = ma_sound_init_from_data_source(&engine, dataSource, 0, group_ptr(parent), &sound->sound);

	if (lastResult == MA_SUCCESS)
	{
		sound->stream = stream;
		sound->parent = parent;
		stream->refCount++;
		hl_add_root(&sound->stream);
		if (parent != NULL)
		{
			group_handle_ref(parent);
			hl_add_root(&sound->parent);
		}
		sound->initialized = 1;
		return sound;
	}

	sound->finalize = NULL;
	return NULL;
}
DEFINE_PRIM(_SOUND, sound_init_stream, _DECODER _GROUP);

HL_PRIM bool HL_NAME(sound_start)(ma_sound_handle* sound)
{
	lastResult = ma_sound_start(sound_ptr(sound));
	return lastResult == MA_SUCCESS;
}
DEFINE_PRIM(_BOOL, sound_start, _SOUND);

HL_PRIM bool HL_NAME(sound_stop)(ma_sound_handle* sound)
{
	lastResult = ma_sound_stop(sound_ptr(sound));
	return lastResult == MA_SUCCESS;
}
DEFINE_PRIM(_BOOL, sound_stop, _SOUND);

HL_PRIM int HL_NAME(sound_seek_samples)(ma_sound_handle* sound, int sample)
{
	if (sample < 0)
		sample = 0;

	lastResult = ma_sound_seek_to_pcm_frame(sound_ptr(sound), (ma_uint64)sample);
	return lastResult == MA_SUCCESS ? sample : -1;
}
DEFINE_PRIM(_I32, sound_seek_samples, _SOUND _I32);

HL_PRIM double HL_NAME(sound_seek_seconds)(ma_sound_handle* sound, double seconds)
{
	if (seconds < 0)
		seconds = 0;

	lastResult = ma_sound_seek_to_second(sound_ptr(sound), (float)seconds);
	return lastResult == MA_SUCCESS ? seconds : -1;
}
DEFINE_PRIM(_F64, sound_seek_seconds, _SOUND _F64);

HL_PRIM double HL_NAME(sound_seek_milliseconds)(ma_sound_handle* sound, double milliseconds)
{
	double seconds;

	if (milliseconds < 0)
		milliseconds = 0;

	seconds = milliseconds / 1000.0;
	lastResult = ma_sound_seek_to_second(sound_ptr(sound), (float)seconds);
	return lastResult == MA_SUCCESS ? milliseconds : -1;
}
DEFINE_PRIM(_F64, sound_seek_milliseconds, _SOUND _F64);

HL_PRIM int HL_NAME(sound_get_cursor_samples)(ma_sound_handle* sound)
{
	ma_uint64 cursor = 0;
	lastResult = ma_sound_get_cursor_in_pcm_frames(sound_ptr(sound), &cursor);
	return lastResult == MA_SUCCESS ? (int)cursor : 0;
}
DEFINE_PRIM(_I32, sound_get_cursor_samples, _SOUND);

HL_PRIM bool HL_NAME(sound_is_playing)(ma_sound_handle* sound)
{
	return ma_sound_is_playing(sound_ptr(sound)) == MA_TRUE;
}
DEFINE_PRIM(_BOOL, sound_is_playing, _SOUND);

HL_PRIM void HL_NAME(sound_set_end_callback)(ma_sound_handle* sound, vclosure* callback)
{
	ma_sound* rawSound = sound_ptr(sound);
	sound_callback_entry* entry = sound_get_callback_entry(sound, true);
	entry->callback = callback;
	entry->pending = 0;
	lastResult = ma_sound_set_end_callback(rawSound, callback == NULL ? NULL : sound_end_callback, callback == NULL ? NULL : entry);
}
DEFINE_PRIM(_VOID, sound_set_end_callback, _SOUND _FUN(_VOID, _NO_ARG));

HL_PRIM void HL_NAME(sound_clear_end_callback)(ma_sound_handle* sound)
{
	ma_sound* rawSound = sound_ptr(sound);
	sound_callback_entry* entry = sound_get_callback_entry(sound, false);
	if (entry != NULL)
	{
		entry->callback = NULL;
		entry->pending = 0;
	}

	lastResult = ma_sound_set_end_callback(rawSound, NULL, NULL);
}
DEFINE_PRIM(_VOID, sound_clear_end_callback, _SOUND);

#undef GET_SET_FLOAT
#define GET_SET_FLOAT(n) HL_PRIM double HL_NAME(sound_get_##n)(ma_sound_handle* sound) \
{ \
	return ma_sound_get_##n(sound_ptr(sound)); \
} \
DEFINE_PRIM(_F64, sound_get_##n, _SOUND) \
HL_PRIM double HL_NAME(sound_set_##n)(ma_sound_handle* sound, double value) \
{ \
	ma_sound_set_##n(sound_ptr(sound), value); \
	return value; \
} \
DEFINE_PRIM(_F64, sound_set_##n, _SOUND _F64)

GET_SET_FLOAT(volume)
GET_SET_FLOAT(pan)

HL_PRIM int HL_NAME(sound_get_pan_mode)(ma_sound_handle* sound)
{
	return (int)ma_sound_get_pan_mode(sound_ptr(sound));
}
DEFINE_PRIM(_I32, sound_get_pan_mode, _SOUND);

HL_PRIM int HL_NAME(sound_set_pan_mode)(ma_sound_handle* sound, int mode)
{
	ma_sound_set_pan_mode(sound_ptr(sound), (ma_pan_mode)mode);
	return mode;
}
DEFINE_PRIM(_I32, sound_set_pan_mode, _SOUND _I32);

GET_SET_FLOAT(pitch)

HL_PRIM bool HL_NAME(sound_get_spatialization_enabled)(ma_sound_handle* sound)
{
	return ma_sound_is_spatialization_enabled(sound_ptr(sound)) == MA_TRUE;
}
DEFINE_PRIM(_BOOL, sound_get_spatialization_enabled, _SOUND);

HL_PRIM bool HL_NAME(sound_set_spatialization_enabled)(ma_sound_handle* sound, bool enabled)
{
	ma_sound_set_spatialization_enabled(sound_ptr(sound), enabled ? MA_TRUE : MA_FALSE);
	return enabled;
}
DEFINE_PRIM(_BOOL, sound_set_spatialization_enabled, _SOUND _BOOL);

HL_PRIM void HL_NAME(sound_set_position)(ma_sound_handle* sound, double x, double y, double z)
{
	ma_sound_set_position(sound_ptr(sound), (float)x, (float)y, (float)z);
}
DEFINE_PRIM(_VOID, sound_set_position, _SOUND _F64 _F64 _F64);

HL_PRIM void HL_NAME(sound_set_velocity)(ma_sound_handle* sound, double x, double y, double z)
{
	ma_sound_set_velocity(sound_ptr(sound), (float)x, (float)y, (float)z);
}
DEFINE_PRIM(_VOID, sound_set_velocity, _SOUND _F64 _F64 _F64);

HL_PRIM void HL_NAME(listener_set_position)(int index, double x, double y, double z)
{
	ma_engine_listener_set_position(&engine, (ma_uint32)index, (float)x, (float)y, (float)z);
}
DEFINE_PRIM(_VOID, listener_set_position, _I32 _F64 _F64 _F64);

HL_PRIM void HL_NAME(listener_set_direction)(int index, double x, double y, double z)
{
	ma_engine_listener_set_direction(&engine, (ma_uint32)index, (float)x, (float)y, (float)z);
}
DEFINE_PRIM(_VOID, listener_set_direction, _I32 _F64 _F64 _F64);

HL_PRIM void HL_NAME(listener_set_world_up)(int index, double x, double y, double z)
{
	ma_engine_listener_set_world_up(&engine, (ma_uint32)index, (float)x, (float)y, (float)z);
}
DEFINE_PRIM(_VOID, listener_set_world_up, _I32 _F64 _F64 _F64);

HL_PRIM double HL_NAME(sound_get_time)(ma_sound_handle* sound)
{
	ma_uint64 cursor = 0;
	ma_uint32 sampleRate = 0;
	ma_sound* rawSound = sound_ptr(sound);

	lastResult = ma_sound_get_cursor_in_pcm_frames(rawSound, &cursor);
	if (lastResult != MA_SUCCESS)
		return 0;

	lastResult = ma_sound_get_data_format(rawSound, NULL, NULL, &sampleRate, NULL, 0);
	if (lastResult != MA_SUCCESS || sampleRate == 0)
		return 0;

	return ((double)cursor * 1000.0) / (double)sampleRate;
}
DEFINE_PRIM(_F64, sound_get_time, _SOUND);

HL_PRIM double HL_NAME(sound_set_time)(ma_sound_handle* sound, double seconds)
{
	return HL_NAME(sound_seek_milliseconds)(sound, seconds);
}
DEFINE_PRIM(_F64, sound_set_time, _SOUND _F64);

HL_PRIM double HL_NAME(sound_get_time_seconds)(ma_sound_handle* sound)
{
	ma_uint64 cursor = 0;
	ma_uint32 sampleRate = 0;
	ma_sound* rawSound = sound_ptr(sound);

	lastResult = ma_sound_get_cursor_in_pcm_frames(rawSound, &cursor);
	if (lastResult != MA_SUCCESS)
		return 0;

	lastResult = ma_sound_get_data_format(rawSound, NULL, NULL, &sampleRate, NULL, 0);
	if (lastResult != MA_SUCCESS || sampleRate == 0)
		return 0;

	return (double)cursor / (double)sampleRate;
}
DEFINE_PRIM(_F64, sound_get_time_seconds, _SOUND);

HL_PRIM double HL_NAME(sound_set_time_seconds)(ma_sound_handle* sound, double seconds)
{
	return HL_NAME(sound_seek_seconds)(sound, seconds);
}
DEFINE_PRIM(_F64, sound_set_time_seconds, _SOUND _F64);

HL_PRIM double HL_NAME(sound_get_duration)(ma_sound_handle* sound)
{
	float length = 0;
	lastResult = ma_sound_get_length_in_seconds(sound_ptr(sound), &length);
	return lastResult == MA_SUCCESS ? (double)length * 1000.0 : 0;
}
DEFINE_PRIM(_F64, sound_get_duration, _SOUND);

HL_PRIM double HL_NAME(sound_get_duration_seconds)(ma_sound_handle* sound)
{
	float length = 0;
	lastResult = ma_sound_get_length_in_seconds(sound_ptr(sound), &length);
	return lastResult == MA_SUCCESS ? length : 0;
}
DEFINE_PRIM(_F64, sound_get_duration_seconds, _SOUND);

HL_PRIM int HL_NAME(sound_get_length_samples)(ma_sound_handle* sound)
{
	ma_uint64 length = 0;
	lastResult = ma_sound_get_length_in_pcm_frames(sound_ptr(sound), &length);
	return lastResult == MA_SUCCESS ? (int)length : 0;
}
DEFINE_PRIM(_I32, sound_get_length_samples, _SOUND);

HL_PRIM int HL_NAME(update)()
{
	sound_callback_entry* entry;
	int dispatched = 0;

	am_mutex_lock(&callback_mutex);
	entry = sound_callbacks;
	while (entry != NULL)
	{
		if (entry->pending && entry->callback != NULL)
		{
			bool isException = false;
			entry->pending = 0;
			am_mutex_unlock(&callback_mutex);
			hl_dyn_call_safe(entry->callback, NULL, 0, &isException);
			am_mutex_lock(&callback_mutex);
			dispatched++;
		}

		entry = entry->next;
	}
	am_mutex_unlock(&callback_mutex);

	return dispatched;
}
DEFINE_PRIM(_I32, update, _NO_ARG);
