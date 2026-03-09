package miniaudio;

import haxe.io.Bytes;

@:hlNative('miniaudio', 'buffer_')
abstract Buffer(BufferImpl) from BufferImpl to BufferImpl
{
	public function dispose() {}

	public static inline function fromBytes(bytes:Bytes)
	{
		return _fromBytes(bytes, bytes.length);
	}

	@:hlNative('miniaudio', 'buffer_from_bytes')
	private static function _fromBytes(bytes:hl.Bytes, size:Int):Buffer
	{
		return null;
	}
}

private typedef BufferImpl = hl.Abstract<'ma_audio_buffer*'>;

@:hlNative('miniaudio', 'sound_group_')
abstract SoundGroup(SoundGroupImpl) from SoundGroupImpl to SoundGroupImpl
{
	public var volume(get, set):Float;
	public var pan(get, set):Float;
	// TODO: pan mode (??)
	public var pitch(get, set):Float;

	// TODO: spatialization (??)

	public inline function new(?parent:SoundGroup)
	{
		this = _init(parent);
	}

	public function start():Bool
	{
		return false;
	}

	public function stop():Bool
	{
		return false;
	}

	@:hlNative('miniaudio', 'sound_group_init')
	private static function _init(?parent:SoundGroup):SoundGroup
	{
		return null;
	}

	private function get_volume():Float
	{
		return 0;
	}

	private function set_volume(v:Float):Float
	{
		return 0;
	}

	private function get_pan():Float
	{
		return 0;
	}

	private function set_pan(v:Float):Float
	{
		return 0;
	}

	private function get_pitch():Float
	{
		return 0;
	}

	private function set_pitch(v:Float):Float
	{
		return 0;
	}
}

private typedef SoundGroupImpl = hl.Abstract<'ma_sound_group*'>;

@:hlNative('miniaudio', 'sound_')
abstract Sound(SoundImpl) from SoundImpl to SoundImpl
{
	public var volume(get, set):Float;
	public var pan(get, set):Float;
	// TODO: pan mode (??)
	public var pitch(get, set):Float;
	// TODO: spatialization (??)
	public var time(get, never):Float;

	public inline function new(buffer:Buffer, ?parent:SoundGroup)
	{
		this = _init(buffer, parent);
	}

	public function dispose() {}

	public function start():Bool
	{
		return false;
	}

	public function stop():Bool
	{
		return false;
	}

	@:hlNative('miniaudio', 'sound_init')
	private static function _init(buffer:Buffer, ?parent:SoundGroup):Sound
	{
		return null;
	}

	private function get_volume():Float
	{
		return 0;
	}

	private function set_volume(v:Float):Float
	{
		return 0;
	}

	private function get_pan():Float
	{
		return 0;
	}

	private function set_pan(v:Float):Float
	{
		return 0;
	}

	private function get_pitch():Float
	{
		return 0;
	}

	private function set_pitch(v:Float):Float
	{
		return 0;
	}

	private function get_time():Float
	{
		return 0;
	}
}

private typedef SoundImpl = hl.Abstract<'ma_sound*'>;

@:hlNative('miniaudio')
class Miniaudio
{
	public static function init():Bool
	{
		return false;
	}

	public static function uninit() {}

	public static inline function describeLastError():String
	{
		return @:privateAccess String.fromUTF8(_describeLastError());
	}

	@:hlNative('miniaudio', 'describe_last_error')
	private static function _describeLastError():hl.Bytes
	{
		return null;
	}
}
