#include "utils.h"

// ─── ENGINE ──────────────────────────────────────

HL_PRIM bool HL_NAME(init)()
{
	if (engine_initialized)
		return true;

	lastResult = ma_engine_init(NULL, &engine);
	if (lastResult == MA_SUCCESS)
	{
		am_mutex_init(&callback_mutex);
		engine_initialized = true;
	}
	return lastResult == MA_SUCCESS;
}
DEFINE_PRIM(_BOOL, init, _NO_ARG);

HL_PRIM void HL_NAME(uninit)()
{
	if (!engine_initialized)
		return;

	clear_sound_callbacks();
	ma_engine_uninit(&engine);
	am_mutex_uninit(&callback_mutex);
	engine_initialized = false;
}
DEFINE_PRIM(_VOID, uninit, _NO_ARG);

HL_PRIM vbyte* HL_NAME(describe_last_error)()
{
	return (vbyte*)ma_result_description(lastResult);
}
DEFINE_PRIM(_BYTES, describe_last_error, _NO_ARG);
