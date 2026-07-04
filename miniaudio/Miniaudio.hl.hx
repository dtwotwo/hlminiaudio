package miniaudio;

import haxe.io.Bytes;
import miniaudio.types.DecodedAudio;
import miniaudio.types.PanMode;
import miniaudio.types.BuildGuards;

// ─── Buffer ───────────────────────────────────────────────────────────────

/**
	A pre-decoded audio buffer that can be shared between multiple sounds.
**/
@:hlNative("miniaudio", "buffer_")
abstract Buffer(BufferImpl) from BufferImpl to BufferImpl {
	/**
		The total number of samples in the buffer.
	**/
	public var lengthSamples(get, never):Int;

	/**
		The duration of the buffer in milliseconds.
	**/
	public var duration(get, never):Float;

	/**
		The duration of the buffer in seconds.
	**/
	public var durationSeconds(get, never):Float;

	/**
		Disposes of the buffer and frees its memory.
	**/
	@:hlNative("miniaudio", "buffer_dispose")
	public function dispose():Void {}

	/**
		Creates a new buffer by decoding the provided audio file bytes.
	**/
	public static inline function fromBytes(bytes:Bytes) {
		return _fromBytes(bytes, bytes.length);
	}

	/**
		Creates a new buffer by decoding the audio file at the provided path.
	**/
	public static inline function fromFile(path:String):Buffer {
		return _fromFile(@:privateAccess path.bytes);
	}

	@:hlNative("miniaudio", "buffer_from_file")
	private static function _fromFile(path:hl.Bytes):Buffer {
		return null;
	}

	/**
		Creates a new buffer from raw floating-point PCM data.
	**/
	public static inline function fromPCMFloat(bytes:Bytes, channels:Int, sampleRate:Int) {
		return _fromPCMFloat(bytes, bytes.length, channels, sampleRate);
	}

	/**
		Creates a new buffer from raw 16-bit integer PCM data.
	**/
	public static inline function fromPCM16(bytes:Bytes, channels:Int, sampleRate:Int) {
		return _fromPCM16(bytes, bytes.length, channels, sampleRate);
	}

	@:hlNative("miniaudio", "buffer_from_bytes")
	private static function _fromBytes(bytes:hl.Bytes, size:Int):Buffer {
		return null;
	}

	@:hlNative("miniaudio", "buffer_from_pcm_float")
	private static function _fromPCMFloat(bytes:hl.Bytes, size:Int, channels:Int, sampleRate:Int):Buffer {
		return null;
	}

	@:hlNative("miniaudio", "buffer_from_pcm_s16")
	private static function _fromPCM16(bytes:hl.Bytes, size:Int, channels:Int, sampleRate:Int):Buffer {
		return null;
	}

	@:hlNative("miniaudio", "buffer_get_length_samples")
	private function get_lengthSamples():Int {
		return 0;
	}

	@:hlNative("miniaudio", "buffer_get_duration")
	private function get_duration():Float {
		return 0;
	}

	@:hlNative("miniaudio", "buffer_get_duration_seconds")
	private function get_durationSeconds():Float {
		return 0;
	}
}

private typedef BufferImpl = hl.Abstract<"ma_buffer_handle">;

// ─── PcmSink ───────────────────────────────────────────────────────────────

/**
	A playback sink for interleaved floating-point PCM audio.
**/
@:hlNative("miniaudio", "pcm_sink_")
abstract PcmSink(PcmSinkImpl) from PcmSinkImpl to PcmSinkImpl {
	/**
		Opens a playback sink for the supplied audio format.
	**/
	public inline function new(sampleRate:Int, channels:Int, bufferFrames = 0) {
		this = _init(sampleRate, channels, bufferFrames);
	}

	/**
		Disposes of the sink and closes its playback device.
	**/
	@:hlNative("miniaudio", "pcm_sink_dispose")
	public function dispose():Void {}

	/**
		Queues interleaved 32-bit float PCM frames.
	**/
	public inline function writeFloat32Interleaved(bytes:Bytes, frames:Int):Int {
		return _writeFloat(@:privateAccess bytes.b, frames);
	}

	@:hlNative("miniaudio", "pcm_sink_init")
	private static function _init(sampleRate:Int, channels:Int, bufferFrames:Int):PcmSink {
		return null;
	}

	@:hlNative("miniaudio", "pcm_sink_write_float")
	private function _writeFloat(bytes:hl.Bytes, frames:Int):Int {
		return 0;
	}

	@:hlNative("miniaudio", "pcm_sink_buffered_frames")
	public function getBufferedFrames():Int {
		return 0;
	}

	@:hlNative("miniaudio", "pcm_sink_played_frames")
	public function getPlayedFrames():Float {
		return 0;
	}

	@:hlNative("miniaudio", "pcm_sink_pause")
	public function pause(paused:Bool):Void {}

	@:hlNative("miniaudio", "pcm_sink_flush")
	public function flush():Void {}

	@:hlNative("miniaudio", "pcm_sink_set_volume")
	public function setVolume(volume:Float):Void {}
}

private typedef PcmSinkImpl = hl.Abstract<"ma_pcm_sink_handle">;

// ─── SoundGroup ───────────────────────────────────────────────────────────────

/**
	A group of sounds that can be controlled together.
**/
@:hlNative("miniaudio", "sound_group_")
abstract SoundGroup(SoundGroupImpl) from SoundGroupImpl to SoundGroupImpl {
	/**
		The volume of the sound group (0.0 to 1.0).
	**/
	public var volume(get, set):Float;

	/**
		The pan of the sound group (-1.0 to 1.0).
	**/
	public var pan(get, set):Float;

	/**
		The pan mode of the sound group (Balance or Pan).
	**/
	public var panMode(get, set):PanMode;

	/**
		The pitch of the sound group (1.0 is normal).
	**/
	public var pitch(get, set):Float;

	/**
		Whether spatialization is enabled for the sound group.
	**/
	public var spatializationEnabled(get, set):Bool;

	/**
		Creates a new sound group, optionally attached to a parent group.
	**/
	public inline function new(?parent:SoundGroup) {
		this = _init(parent);
	}

	/**
		Disposes of the sound group.
	**/
	@:hlNative("miniaudio", "sound_group_dispose")
	public function dispose():Void {}

	/**
		Starts playback of all sounds in the group.
	**/
	@:hlNative("miniaudio", "sound_group_start")
	public function start():Bool {
		return false;
	}

	/**
		Stops playback of all sounds in the group.
	**/
	@:hlNative("miniaudio", "sound_group_stop")
	public function stop():Bool {
		return false;
	}

	@:hlNative("miniaudio", "sound_group_init")
	private static function _init(?parent:SoundGroup):SoundGroup {
		return null;
	}

	@:hlNative("miniaudio", "sound_group_get_volume")
	private function get_volume():Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_group_set_volume")
	private function set_volume(v:Float):Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_group_get_pan")
	private function get_pan():Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_group_set_pan")
	private function set_pan(v:Float):Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_group_get_pan_mode")
	private function get_panMode():PanMode {
		return Balance;
	}

	@:hlNative("miniaudio", "sound_group_set_pan_mode")
	private function set_panMode(v:PanMode):PanMode {
		return v;
	}

	@:hlNative("miniaudio", "sound_group_get_pitch")
	private function get_pitch():Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_group_set_pitch")
	private function set_pitch(v:Float):Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_group_get_spatialization_enabled")
	private function get_spatializationEnabled():Bool {
		return false;
	}

	@:hlNative("miniaudio", "sound_group_set_spatialization_enabled")
	private function set_spatializationEnabled(v:Bool):Bool {
		return v;
	}

	/**
		Sets the world position of the sound group.
	**/
	@:hlNative("miniaudio", "sound_group_set_position")
	public function setPosition(x:Float, y:Float, z:Float):Void {}

	/**
		Sets the world velocity of the sound group.
	**/
	@:hlNative("miniaudio", "sound_group_set_velocity")
	public function setVelocity(x:Float, y:Float, z:Float):Void {}
}

private typedef SoundGroupImpl = hl.Abstract<"ma_group_handle">;

// ─── Sound ───────────────────────────────────────────────────────────────

/**
	An instance of a sound that can be played.
**/
@:hlNative("miniaudio", "sound_")
abstract Sound(SoundImpl) from SoundImpl to SoundImpl {
	/**
		The volume of the sound (0.0 to 1.0).
	**/
	public var volume(get, set):Float;

	/**
		The pan of the sound (-1.0 to 1.0).
	**/
	public var pan(get, set):Float;

	/**
		The pan mode of the sound (Balance or Pan).
	**/
	public var panMode(get, set):PanMode;

	/**
		The pitch of the sound (1.0 is normal).
	**/
	public var pitch(get, set):Float;

	/**
		Whether spatialization is enabled for the sound.
	**/
	public var spatializationEnabled(get, set):Bool;

	/**
		Sets the world position of the sound.
	**/
	@:hlNative("miniaudio", "sound_set_position")
	public function setPosition(x:Float, y:Float, z:Float):Void {}

	/**
		Sets the world velocity of the sound.
	**/
	@:hlNative("miniaudio", "sound_set_velocity")
	public function setVelocity(x:Float, y:Float, z:Float):Void {}

	/**
		The current playback position in milliseconds.
	**/
	public var time(get, set):Float;

	/**
		The current playback position in seconds.
	**/
	public var timeSeconds(get, set):Float;

	/**
		The total duration of the sound in milliseconds.
	**/
	public var duration(get, never):Float;

	/**
		The total duration of the sound in seconds.
	**/
	public var durationSeconds(get, never):Float;

	/**
		The total length of the sound in samples.
	**/
	public var lengthSamples(get, never):Int;

	/**
		Creates a new sound from a buffer, optionally attached to a group.
	**/
	public inline function new(buffer:Buffer, ?parent:SoundGroup) {
		this = _init(buffer, parent);
	}

	/**
		Creates a new sound from a stream decoder, optionally attached to a group.
	**/
	public static inline function fromStream(stream:StreamDecoder, ?parent:SoundGroup):Sound {
		return _initStream(stream, parent);
	}

	/**
		Disposes of the sound.
	**/
	@:hlNative("miniaudio", "sound_dispose")
	public function dispose():Void {}

	/**
		Starts playback of the sound.
	**/
	@:hlNative("miniaudio", "sound_start")
	public function start():Bool {
		return false;
	}

	/**
		Stops playback of the sound.
	**/
	@:hlNative("miniaudio", "sound_stop")
	public function stop():Bool {
		return false;
	}

	/**
		Sets a callback to be called when playback of the sound completes.
	**/
	@:hlNative("miniaudio", "sound_set_end_callback")
	public function setOnComplete(callback:Void->Void):Void {}

	/**
		Clears the completion callback.
	**/
	@:hlNative("miniaudio", "sound_clear_end_callback")
	public function clearOnComplete():Void {}

	/**
		Seeks to a specific sample position.
	**/
	@:hlNative("miniaudio", "sound_seek_samples")
	public function seekSamples(v:Int):Int {
		return v;
	}

	/**
		Seeks to a specific position in milliseconds.
	**/
	@:hlNative("miniaudio", "sound_seek_milliseconds")
	public function seek(v:Float):Float {
		return v;
	}

	/**
		Seeks to a specific position in seconds.
	**/
	@:hlNative("miniaudio", "sound_seek_seconds")
	public function seekSeconds(v:Float):Float {
		return v;
	}

	/**
		Seeks to a specific position in milliseconds.
	**/
	@:hlNative("miniaudio", "sound_seek_milliseconds")
	public function seekMs(v:Float):Float {
		return v;
	}

	@:hlNative("miniaudio", "sound_set_time")
	private function _setTime(v:Float):Float {
		return v;
	}

	@:hlNative("miniaudio", "sound_get_time_seconds")
	private function _getTimeSeconds():Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_set_time_seconds")
	private function _setTimeSeconds(v:Float):Float {
		return v;
	}

	@:hlNative("miniaudio", "sound_get_cursor_samples")
	public function getCursorSamples():Int {
		return 0;
	}

	@:hlNative("miniaudio", "sound_is_playing")
	public function isPlaying():Bool {
		return false;
	}

	@:hlNative("miniaudio", "sound_init")
	@:noCompletion
	private static function _init(buffer:Buffer, ?parent:SoundGroup):Sound {
		return null;
	}

	@:hlNative("miniaudio", "sound_init_stream")
	@:noCompletion
	private static function _initStream(stream:StreamDecoder, ?parent:SoundGroup):Sound {
		return null;
	}

	@:hlNative("miniaudio", "sound_get_volume")
	@:noCompletion
	private function get_volume():Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_set_volume")
	@:noCompletion
	private function set_volume(v:Float):Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_get_pan")
	@:noCompletion
	private function get_pan():Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_set_pan")
	@:noCompletion
	private function set_pan(v:Float):Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_get_pan_mode")
	@:noCompletion
	private function get_panMode():PanMode {
		return Balance;
	}

	@:hlNative("miniaudio", "sound_set_pan_mode")
	@:noCompletion
	private function set_panMode(v:PanMode):PanMode {
		return v;
	}

	@:hlNative("miniaudio", "sound_get_pitch")
	@:noCompletion
	private function get_pitch():Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_set_pitch")
	@:noCompletion
	private function set_pitch(v:Float):Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_get_spatialization_enabled")
	@:noCompletion
	private function get_spatializationEnabled():Bool {
		return false;
	}

	@:hlNative("miniaudio", "sound_set_spatialization_enabled")
	@:noCompletion
	private function set_spatializationEnabled(v:Bool):Bool {
		return v;
	}

	@:hlNative("miniaudio", "sound_get_time")
	@:noCompletion
	private function get_time():Float {
		return 0;
	}

	@:noCompletion
	private function set_time(v:Float):Float {
		return _setTime(v);
	}

	@:noCompletion
	private function get_timeSeconds():Float {
		return _getTimeSeconds();
	}

	@:noCompletion
	private function set_timeSeconds(v:Float):Float {
		return _setTimeSeconds(v);
	}

	@:hlNative("miniaudio", "sound_get_duration")
	@:noCompletion
	private function get_duration():Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_get_duration_seconds")
	@:noCompletion
	private function get_durationSeconds():Float {
		return 0;
	}

	@:hlNative("miniaudio", "sound_get_length_samples")
	@:noCompletion
	private function get_lengthSamples():Int {
		return 0;
	}
}

private typedef SoundImpl = hl.Abstract<"ma_sound_handle">;

// ─── StreamDecoder ───────────────────────────────────────────────────────────────

/**
	A decoder for streaming audio from memory.
**/
@:hlNative("miniaudio", "stream_")
abstract StreamDecoder(StreamDecoderImpl) from StreamDecoderImpl to StreamDecoderImpl {
	/**
		The number of audio channels.
	**/
	public var channels(get, never):Int;

	/**
		The sample rate in Hz.
	**/
	public var sampleRate(get, never):Int;

	/**
		The total number of samples in the stream.
	**/
	public var samples(get, never):Int;

	/**
		Opens a new stream decoder from the provided audio file bytes.
	**/
	public static inline function open(bytes:Bytes):StreamDecoder {
		return _open(bytes, bytes.length);
	}

	@:hlNative("miniaudio", "stream_open")
	private static function _open(bytes:hl.Bytes, size:Int):StreamDecoder {
		return null;
	}

	/**
		Opens a new stream decoder from the provided audio file path.
	**/
	public static inline function fromFile(path:String):StreamDecoder {
		return _fromFile(@:privateAccess path.bytes);
	}

	@:hlNative("miniaudio", "stream_open_file")
	private static function _fromFile(path:hl.Bytes):StreamDecoder {
		return null;
	}

	/**
		Disposes of the stream decoder.
	**/
	@:hlNative("miniaudio", "stream_dispose")
	public function dispose():Void {}

	/**
		Decodes a chunk of audio data into the provided buffer.
	**/
	public inline function decode(out:Bytes, outPos:Int, sampleStart:Int, sampleCount:Int):Int {
		return _decode(out, outPos, sampleStart, sampleCount);
	}

	@:hlNative("miniaudio", "stream_decode")
	private function _decode(out:hl.Bytes, outPos:Int, sampleStart:Int, sampleCount:Int):Int {
		return 0;
	}

	@:hlNative("miniaudio", "stream_channels")
	private function get_channels():Int {
		return 0;
	}

	@:hlNative("miniaudio", "stream_sample_rate")
	private function get_sampleRate():Int {
		return 0;
	}

	@:hlNative("miniaudio", "stream_samples")
	private function get_samples():Int {
		return 0;
	}
}

private typedef StreamDecoderImpl = hl.Abstract<"ma_stream_decoder">;

// ─── Miniaudio ───────────────────────────────────────────────────────────────

/**
	The main interface for the miniaudio library.
**/
@:hlNative("miniaudio")
class Miniaudio {
	/**
		Initializes the miniaudio engine.
	**/
	@:hlNative("miniaudio", "init")
	public static function init():Bool {
		return false;
	}

	/**
		Uninitializes the miniaudio engine.
	**/
	@:hlNative("miniaudio", "uninit")
	public static function uninit():Void {}

	/**
		Updates the miniaudio engine. Should be called regularly (e.g. every frame).
	**/
	@:hlNative("miniaudio", "update")
	public static function update():Int {
		return 0;
	}

	/**
		Triggers miniaudio garbage collection.
	**/
	@:hlNative("miniaudio", "gc")
	public static function gc():Void {}

	/**
		Sets the position of the listener.
	**/
	@:hlNative("miniaudio", "listener_set_position")
	public static function setListenerPosition(index:Int, x:Float, y:Float, z:Float):Void {}

	/**
		Sets the direction of the listener.
	**/
	@:hlNative("miniaudio", "listener_set_direction")
	public static function setListenerDirection(index:Int, x:Float, y:Float, z:Float):Void {}

	/**
		Sets the world up vector of the listener.
	**/
	@:hlNative("miniaudio", "listener_set_world_up")
	public static function setListenerWorldUp(index:Int, x:Float, y:Float, z:Float):Void {}

	/**
		Decodes the provided audio file bytes into floating-point PCM data.
	**/
	public static inline function decodeToPCMFloat(bytes:Bytes):DecodedAudio {
		final decoded = _decodeToPCMFloat(bytes, bytes.length);
		final channels = _decodedChannels();

		final sampleRate = _decodedSampleRate();
		final samples = _decodedSamples();
		final byteCount = samples * channels * 4;
		if (decoded == null)
			return null;

		return {
			bytes: @:privateAccess new Bytes(decoded, byteCount),
			channels: channels,

			sampleRate: sampleRate,
			samples: samples,
			floatFormat: true,
		};
	}

	/**
		Decodes the provided audio file bytes into 16-bit integer PCM data.
	**/
	public static inline function decodeToPCM16(bytes:Bytes):DecodedAudio {
		final decoded = _decodeToPCM16(bytes, bytes.length);
		final channels = _decodedChannels();

		final sampleRate = _decodedSampleRate();
		final samples = _decodedSamples();
		final byteCount = samples * channels * 2;
		if (decoded == null)
			return null;

		return {
			bytes: @:privateAccess new Bytes(decoded, byteCount),
			channels: channels,
			sampleRate: sampleRate,

			samples: samples,
			floatFormat: false,
		};
	}

	/**
		Returns a string describing the last error that occurred in miniaudio.
	**/
	public static inline function describeLastError():String {
		return @:privateAccess String.fromUTF8(_describeLastError());
	}

	@:hlNative("miniaudio", "describe_last_error")
	@:noCompletion
	private static function _describeLastError():hl.Bytes {
		return null;
	}

	@:hlNative("miniaudio", "decode_pcm_float")
	@:noCompletion
	private static function _decodeToPCMFloat(bytes:hl.Bytes, size:Int):hl.Bytes {
		return null;
	}

	@:hlNative("miniaudio", "decode_pcm_s16")
	@:noCompletion
	private static function _decodeToPCM16(bytes:hl.Bytes, size:Int):hl.Bytes {
		return null;
	}

	@:hlNative("miniaudio", "decoded_channels")
	@:noCompletion
	private static function _decodedChannels():Int {
		return 0;
	}

	@:hlNative("miniaudio", "decoded_sample_rate")
	@:noCompletion
	private static function _decodedSampleRate():Int {
		return 0;
	}

	@:hlNative("miniaudio", "decoded_samples")
	@:noCompletion
	private static function _decodedSamples():Int {
		return 0;
	}
}
