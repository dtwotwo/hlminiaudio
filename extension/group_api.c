#include "utils.h"

// ─── SOUND GROUP ──────────────────────────────────────

ma_sound_group* group_ptr(ma_group_handle* group)
{
	return group == NULL || !group->initialized ? NULL : &group->group;
}

void group_handle_ref(ma_group_handle* group)
{
	if (group != NULL)
		group->refCount++;
}

void group_handle_release(ma_group_handle* handle, int force)
{
	if (handle == NULL || !handle->initialized)
		return;

	if (!force && handle->refCount > 0)
	{
		handle->disposeRequested = 1;
		return;
	}

	ma_sound_group_uninit(&handle->group);
	handle->initialized = 0;
	handle->finalize = NULL;

	if (handle->parent != NULL)
	{
		ma_group_handle* parent = handle->parent;
		handle->parent = NULL;
		hl_remove_root(&handle->parent);
		if (parent->refCount > 0)
			parent->refCount--;
		if (parent->disposeRequested && parent->refCount == 0)
			group_handle_release(parent, 0);
	}
}

static void group_handle_finalize(ma_group_handle* handle)
{
	group_handle_release(handle, 0);
}

HL_PRIM void HL_NAME(sound_group_dispose)(ma_group_handle* group)
{
	group_handle_release(group, 0);
}
DEFINE_PRIM(_VOID, sound_group_dispose, _GROUP);

HL_PRIM ma_group_handle* HL_NAME(sound_group_init)(ma_group_handle* parent)
{
	ma_group_handle* group = (ma_group_handle*)hl_gc_alloc_finalizer(sizeof(ma_group_handle));
	memset(group, 0, sizeof(ma_group_handle));
	group->finalize = group_handle_finalize;
	group->parent = parent;
	if (parent != NULL)
	{
		group_handle_ref(parent);
		hl_add_root(&group->parent);
	}

	lastResult = ma_sound_group_init(&engine, 0, group_ptr(parent), &group->group);

	if (lastResult == MA_SUCCESS)
	{
		group->initialized = 1;
		return group;
	}
	else
	{
		if (parent != NULL)
		{
			hl_remove_root(&group->parent);
			if (parent->refCount > 0)
				parent->refCount--;
			group->parent = NULL;
		}
		group->finalize = NULL;
		return NULL;
	}
}
DEFINE_PRIM(_GROUP, sound_group_init, _GROUP);

HL_PRIM bool HL_NAME(sound_group_start)(ma_group_handle* group)
{
	lastResult = ma_sound_group_start(group_ptr(group));
	return lastResult == MA_SUCCESS;
}
DEFINE_PRIM(_BOOL, sound_group_start, _GROUP);

HL_PRIM bool HL_NAME(sound_group_stop)(ma_group_handle* group)
{
	lastResult = ma_sound_group_stop(group_ptr(group));
	return lastResult == MA_SUCCESS;
}
DEFINE_PRIM(_BOOL, sound_group_stop, _GROUP);

#define GET_SET_FLOAT(n) HL_PRIM double HL_NAME(sound_group_get_##n)(ma_group_handle* group) \
{ \
	return ma_sound_group_get_##n(group_ptr(group)); \
} \
DEFINE_PRIM(_F64, sound_group_get_##n, _GROUP) \
HL_PRIM double HL_NAME(sound_group_set_##n)(ma_group_handle* group, double value) \
{ \
	ma_sound_group_set_##n(group_ptr(group), value); \
	return value; \
} \
DEFINE_PRIM(_F64, sound_group_set_##n, _GROUP _F64)

GET_SET_FLOAT(volume)
GET_SET_FLOAT(pan)

HL_PRIM int HL_NAME(sound_group_get_pan_mode)(ma_group_handle* group)
{
	return (int)ma_sound_group_get_pan_mode(group_ptr(group));
}
DEFINE_PRIM(_I32, sound_group_get_pan_mode, _GROUP);

HL_PRIM int HL_NAME(sound_group_set_pan_mode)(ma_group_handle* group, int mode)
{
	ma_sound_group_set_pan_mode(group_ptr(group), (ma_pan_mode)mode);
	return mode;
}
DEFINE_PRIM(_I32, sound_group_set_pan_mode, _GROUP _I32);

GET_SET_FLOAT(pitch)

HL_PRIM bool HL_NAME(sound_group_get_spatialization_enabled)(ma_group_handle* group)
{
	return ma_sound_group_is_spatialization_enabled(group_ptr(group)) == MA_TRUE;
}
DEFINE_PRIM(_BOOL, sound_group_get_spatialization_enabled, _GROUP);

HL_PRIM bool HL_NAME(sound_group_set_spatialization_enabled)(ma_group_handle* group, bool enabled)
{
	ma_sound_group_set_spatialization_enabled(group_ptr(group), enabled ? MA_TRUE : MA_FALSE);
	return enabled;
}
DEFINE_PRIM(_BOOL, sound_group_set_spatialization_enabled, _GROUP _BOOL);

HL_PRIM void HL_NAME(sound_group_set_position)(ma_group_handle* group, double x, double y, double z)
{
	ma_sound_group_set_position(group_ptr(group), (float)x, (float)y, (float)z);
}
DEFINE_PRIM(_VOID, sound_group_set_position, _GROUP _F64 _F64 _F64);

HL_PRIM void HL_NAME(sound_group_set_velocity)(ma_group_handle* group, double x, double y, double z)
{
	ma_sound_group_set_velocity(group_ptr(group), (float)x, (float)y, (float)z);
}
DEFINE_PRIM(_VOID, sound_group_set_velocity, _GROUP _F64 _F64 _F64);
