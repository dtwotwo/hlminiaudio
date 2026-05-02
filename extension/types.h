#pragma once

#include <hl.h>
#include <stddef.h>

#ifdef _GUID
#undef _GUID
#endif

#define MA_NO_VORBIS
#include <miniaudio.h>
#include "extras/decoders/libopus/miniaudio_libopus.h"
#include "extras/decoders/libvorbis/miniaudio_libvorbis.h"

// ─── TYPES ──────────────────────────────────────

typedef struct ma_buffer_handle
{
	void (*finalize)(struct ma_buffer_handle*);
	ma_audio_buffer* buffer;
	void* data;
	int refCount;
	int disposeRequested;
} ma_buffer_handle;

typedef struct ma_group_handle
{
	void (*finalize)(struct ma_group_handle*);
	ma_sound_group group;
	struct ma_group_handle* parent;
	int refCount;
	int initialized;
	int disposeRequested;
} ma_group_handle;

typedef struct ma_sound_handle
{
	void (*finalize)(struct ma_sound_handle*);
	ma_sound sound;
	ma_buffer_handle* buffer;
	struct ma_stream_decoder* stream;
	ma_group_handle* parent;
	struct sound_callback_entry* callback_entry;
	int initialized;
} ma_sound_handle;

typedef struct
{
	const unsigned char* data;
	size_t size;
	size_t cursor;
} memory_stream;

typedef enum
{
	STREAM_DECODER_MINIAUDIO,
	STREAM_DECODER_VORBIS,
	STREAM_DECODER_OPUS,
	STREAM_DECODER_FILE
} stream_decoder_kind;

typedef struct ma_stream_decoder
{
	void (*finalize)(struct ma_stream_decoder*);
	stream_decoder_kind kind;
	vbyte* bytes;
	memory_stream stream;
	ma_decoder decoder;
	ma_libvorbis vorbis;
	ma_libopus opus;
	ma_uint32 channels;
	ma_uint32 sampleRate;
	ma_uint64 frameCount;
	int refCount;
	int disposeRequested;
	int initialized;
} ma_stream_decoder;

typedef struct sound_callback_entry
{
	ma_sound* sound;
	vclosure* callback;
	volatile int pending;
	struct sound_callback_entry* next;
	struct sound_callback_entry* prev;
} sound_callback_entry;

#ifdef _WIN32
#include <windows.h>
typedef CRITICAL_SECTION am_mutex;
#define am_mutex_init(m) InitializeCriticalSection(m)
#define am_mutex_lock(m) EnterCriticalSection(m)
#define am_mutex_unlock(m) LeaveCriticalSection(m)
#define am_mutex_uninit(m) DeleteCriticalSection(m)
#else
#include <pthread.h>
typedef pthread_mutex_t am_mutex;
#define am_mutex_init(m) pthread_mutex_init(m, NULL)
#define am_mutex_lock(m) pthread_mutex_lock(m)
#define am_mutex_unlock(m) pthread_mutex_unlock(m)
#define am_mutex_uninit(m) pthread_mutex_destroy(m)
#endif
