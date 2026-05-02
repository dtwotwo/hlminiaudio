package miniaudio;

#if !js
#error "MiniaudioJs is for the js target only."
#end
import haxe.io.Bytes;
import js.lib.Uint8Array;
import miniaudio.types.DecodedAudio;
import miniaudio.types.PanMode;
import miniaudio.types.BuildGuards;

// ─── WASM glue (private) ──────────────────────────────────────────────────────

private class Wasm {
	public static var mod:Dynamic = null;

	// end-callback dispatch table
	public static var callbacks:Map<Int, Void->Void> = new Map();
	public static var soundCbIds:Map<Int, Int> = new Map(); // soundPtr → cbId
	public static var nextCbId:Int = 1;

	public static final MAX_CBS = 64;
	public static var cbOutPtr:Int = 0;

	public static function call(fn:String, ret:String, argTypes:Array<String>, args:Array<Dynamic>):Dynamic {
		return mod.ccall(fn, ret, argTypes, args);
	}

	/**
		Copy haxe.io.Bytes into WASM heap; returns malloc'd pointer.
		Caller must free.
	**/
	public static function upload(bytes:Bytes):{ptr:Int, len:Int} {
		final len = bytes.length;
		final ptr:Int = call("malloc", "number", ["number"], [len]);
		final heap:js.lib.Uint8Array = mod.HEAPU8;
		heap.set(new Uint8Array(@:privateAccess bytes.b.buffer, @:privateAccess bytes.b.byteOffset, len), ptr);
		return {ptr: ptr, len: len};
	}

	public static function free(ptr:Int) {
		call("free", null, ["number"], [ptr]);
	}

	public static function removeCallbackForSound(sndPtr:Int) {
		final id = soundCbIds.get(sndPtr);
		if (id != null) {
			callbacks.remove(id);
			soundCbIds.remove(sndPtr);
		}
	}
}

// ─── Buffer ───────────────────────────────────────────────────────────────────

/**
	A pre-decoded audio buffer that can be shared between multiple sounds.
**/
abstract Buffer(Int) to Int {
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

	inline function new(ptr:Int)
		this = ptr;

	/**
		Disposes of the buffer and frees its memory.
	**/
	public function dispose():Void {
		Wasm.call("ma_js_buffer_dispose", null, ["number"], [this]);
	}

	/**
		Creates a new buffer by decoding the provided audio file bytes.
	**/
	public static inline function fromBytes(bytes:Bytes):Buffer {
		final u = Wasm.upload(bytes);
		final ptr:Int = Wasm.call("ma_js_buffer_from_bytes", "number", ["number", "number"], [u.ptr, u.len]);
		Wasm.free(u.ptr);
		return new Buffer(ptr);
	}

	/**
		Creates a new buffer from raw floating-point PCM data.
	**/
	public static inline function fromPCMFloat(bytes:Bytes, channels:Int, sampleRate:Int):Buffer {
		final u = Wasm.upload(bytes);
		final ptr:Int = Wasm.call("ma_js_buffer_from_pcm_float", "number", ["number", "number", "number", "number"], [u.ptr, u.len, channels, sampleRate]);
		Wasm.free(u.ptr);
		return new Buffer(ptr);
	}

	/**
		Creates a new buffer from raw 16-bit integer PCM data.
	**/
	public static inline function fromPCM16(bytes:Bytes, channels:Int, sampleRate:Int):Buffer {
		final u = Wasm.upload(bytes);
		final ptr:Int = Wasm.call("ma_js_buffer_from_pcm_s16", "number", ["number", "number", "number", "number"], [u.ptr, u.len, channels, sampleRate]);
		Wasm.free(u.ptr);
		return new Buffer(ptr);
	}

	private function get_lengthSamples():Int {
		return Wasm.call("ma_js_buffer_get_length_samples", "number", ["number"], [this]);
	}

	private function get_duration():Float {
		return Wasm.call("ma_js_buffer_get_duration", "number", ["number"], [this]);
	}

	private function get_durationSeconds():Float {
		return Wasm.call("ma_js_buffer_get_duration_seconds", "number", ["number"], [this]);
	}
}

// ─── SoundGroup ───────────────────────────────────────────────────────────────

/**
	A group of sounds that can be controlled together.
**/
abstract SoundGroup(Int) to Int {
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
		this = Wasm.call("ma_js_sound_group_init", "number", ["number"], [parent == null ? 0 : (parent : Int)]);
	}

	/**
		Disposes of the sound group.
	**/
	public function dispose():Void {
		Wasm.call("ma_js_sound_group_dispose", null, ["number"], [this]);
	}

	/**
		Starts playback of all sounds in the group.
	**/
	public function start():Bool {
		return Wasm.call("ma_js_sound_group_start", "number", ["number"], [this]) != 0;
	}

	/**
		Stops playback of all sounds in the group.
	**/
	public function stop():Bool {
		return Wasm.call("ma_js_sound_group_stop", "number", ["number"], [this]) != 0;
	}

	private function get_volume():Float {
		return Wasm.call("ma_js_sound_group_get_volume", "number", ["number"], [this]);
	}

	private function set_volume(v:Float):Float {
		return Wasm.call("ma_js_sound_group_set_volume", "number", ["number", "number"], [this, v]);
	}

	private function get_pan():Float {
		return Wasm.call("ma_js_sound_group_get_pan", "number", ["number"], [this]);
	}

	private function set_pan(v:Float):Float {
		return Wasm.call("ma_js_sound_group_set_pan", "number", ["number", "number"], [this, v]);
	}

	private function get_panMode():PanMode {
		return Wasm.call("ma_js_sound_group_get_pan_mode", "number", ["number"], [this]);
	}

	private function set_panMode(v:PanMode):PanMode {
		Wasm.call("ma_js_sound_group_set_pan_mode", null, ["number", "number"], [this, (v : Int)]);
		return v;
	}

	private function get_pitch():Float {
		return Wasm.call("ma_js_sound_group_get_pitch", "number", ["number"], [this]);
	}

	private function set_pitch(v:Float):Float {
		return Wasm.call("ma_js_sound_group_set_pitch", "number", ["number", "number"], [this, v]);
	}

	private function get_spatializationEnabled():Bool {
		return Wasm.call("ma_js_sound_group_get_spatialization_enabled", "number", ["number"], [this]) != 0;
	}

	private function set_spatializationEnabled(v:Bool):Bool {
		Wasm.call("ma_js_sound_group_set_spatialization_enabled", null, ["number", "number"], [this, v ? 1 : 0]);
		return v;
	}
}

// ─── Sound ────────────────────────────────────────────────────────────────────

/**
	An instance of a sound that can be played.
**/
abstract Sound(Int) to Int {
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
		this = Wasm.call("ma_js_sound_init", "number", ["number", "number"], [(buffer : Int), parent == null ? 0 : (parent : Int)]);
	}

	/**
		Disposes of the sound.
	**/
	public function dispose():Void {
		Wasm.removeCallbackForSound(this);
		Wasm.call("ma_js_sound_dispose", null, ["number"], [this]);
	}

	/**
		Starts playback of the sound.
	**/
	public function start():Bool {
		return Wasm.call("ma_js_sound_start", "number", ["number"], [this]) != 0;
	}

	/**
		Stops playback of the sound.
	**/
	public function stop():Bool {
		return Wasm.call("ma_js_sound_stop", "number", ["number"], [this]) != 0;
	}

	/**
		Sets a callback to be called when playback of the sound completes.
	**/
	public function setOnComplete(callback:Void->Void):Void {
		Wasm.removeCallbackForSound(this);

		final id = Wasm.nextCbId++;
		Wasm.soundCbIds.set(this, id);
		Wasm.callbacks.set(id, callback);
		Wasm.call("ma_js_sound_set_end_callback_js", null, ["number", "number"], [this, id]);
	}

	/**
		Clears the completion callback.
	**/
	public function clearOnComplete():Void {
		Wasm.removeCallbackForSound(this);
		Wasm.call("ma_js_sound_clear_end_callback", null, ["number"], [this]);
	}

	/**
		Seeks to a specific sample position.
	**/
	public function seekSamples(v:Int):Int
		return Wasm.call("ma_js_sound_seek_samples", "number", ["number", "number"], [this, v]);

	/**
		Seeks to a specific position in milliseconds.
	**/
	public function seek(v:Float):Float
		return Wasm.call("ma_js_sound_seek_milliseconds", "number", ["number", "number"], [this, v]);

	/**
		Seeks to a specific position in seconds.
	**/
	public function seekSeconds(v:Float):Float
		return Wasm.call("ma_js_sound_seek_seconds", "number", ["number", "number"], [this, v]);

	/**
		Seeks to a specific position in milliseconds.
	**/
	public function seekMs(v:Float):Float
		return Wasm.call("ma_js_sound_seek_milliseconds", "number", ["number", "number"], [this, v]);

	/**
		Returns the current cursor position in samples.
	**/
	public function getCursorSamples():Int
		return Wasm.call("ma_js_sound_get_cursor_samples", "number", ["number"], [this]);

	/**
		Returns whether the sound is currently playing.
	**/
	public function isPlaying():Bool
		return Wasm.call("ma_js_sound_is_playing", "number", ["number"], [this]) != 0;

	@:noCompletion private function get_volume():Float
		return Wasm.call("ma_js_sound_get_volume", "number", ["number"], [this]);

	@:noCompletion private function set_volume(v:Float):Float
		return Wasm.call("ma_js_sound_set_volume", "number", ["number", "number"], [this, v]);

	@:noCompletion private function get_pan():Float
		return Wasm.call("ma_js_sound_get_pan", "number", ["number"], [this]);

	@:noCompletion private function set_pan(v:Float):Float
		return Wasm.call("ma_js_sound_set_pan", "number", ["number", "number"], [this, v]);

	@:noCompletion private function get_panMode():PanMode
		return Wasm.call("ma_js_sound_get_pan_mode", "number", ["number"], [this]);

	@:noCompletion private function set_panMode(v:PanMode):PanMode {
		Wasm.call("ma_js_sound_set_pan_mode", null, ["number", "number"], [this, (v : Int)]);
		return v;
	}

	@:noCompletion private function get_pitch():Float
		return Wasm.call("ma_js_sound_get_pitch", "number", ["number"], [this]);

	@:noCompletion private function set_pitch(v:Float):Float
		return Wasm.call("ma_js_sound_set_pitch", "number", ["number", "number"], [this, v]);

	@:noCompletion private function get_spatializationEnabled():Bool
		return Wasm.call("ma_js_sound_get_spatialization_enabled", "number", ["number"], [this]) != 0;

	@:noCompletion private function set_spatializationEnabled(v:Bool):Bool {
		Wasm.call("ma_js_sound_set_spatialization_enabled", null, ["number", "number"], [this, v ? 1 : 0]);
		return v;
	}

	@:noCompletion private function get_time():Float
		return Wasm.call("ma_js_sound_get_time", "number", ["number"], [this]);

	@:noCompletion private function set_time(v:Float):Float
		return Wasm.call("ma_js_sound_set_time", "number", ["number", "number"], [this, v]);

	@:noCompletion private function get_timeSeconds():Float
		return Wasm.call("ma_js_sound_get_time_seconds", "number", ["number"], [this]);

	@:noCompletion private function set_timeSeconds(v:Float):Float
		return Wasm.call("ma_js_sound_set_time_seconds", "number", ["number", "number"], [this, v]);

	@:noCompletion private function get_duration():Float
		return Wasm.call("ma_js_sound_get_duration", "number", ["number"], [this]);

	@:noCompletion private function get_durationSeconds():Float
		return Wasm.call("ma_js_sound_get_duration_seconds", "number", ["number"], [this]);

	@:noCompletion private function get_lengthSamples():Int
		return Wasm.call("ma_js_sound_get_length_samples", "number", ["number"], [this]);
}

// ─── Miniaudio ────────────────────────────────────────────────────────────────

class Miniaudio {
	/**
		Dynamically injects miniaudio.js into the page (like ImGuiJsApp.loadScript),
		waits for the WASM module to initialize, then calls onReady.
		miniaudio.js and miniaudio.wasm must be located next to index.html.
	**/
	public static function load(onReady:Void->Void, ?onError:Void->Void):Void {
		loadScript("./miniaudio.js", ok -> {
			if (!ok) {
				if (onError != null)
					onError()
				else
					throw "Failed to load miniaudio.js";
				return;
			}
			final factory:Dynamic = js.Syntax.code("window.MiniaudioModule");
			(factory() : js.lib.Promise<Dynamic>).then((m:Dynamic) -> {
				Wasm.mod = m;
				Wasm.cbOutPtr = m.ccall("malloc", "number", ["number"], [Wasm.MAX_CBS * 4]);
				onReady();
			}, _ -> {
				if (onError != null)
					onError()
				else
					throw "Failed to initialize miniaudio WASM";
			});
		});
	}

	private static function loadScript(src:String, done:Bool->Void):Void {
		var called = false;
		final script:js.html.ScriptElement = js.Browser.document.createScriptElement();
		script.setAttribute("type", "text/javascript");
		script.addEventListener("load", () -> {
			if (!called) {
				called = true;
				done(true);
			}
		});
		script.addEventListener("error", () -> {
			if (!called) {
				called = true;
				done(false);
			}
		});
		script.setAttribute("src", src);
		js.Browser.document.head.appendChild(script);
	}

	public static function init():Bool {
		return Wasm.call("ma_js_init", "number", [], []) != 0;
	}

	/**
		Uninitializes the miniaudio engine.
	**/
	public static function uninit():Void {
		Wasm.call("ma_js_uninit", null, [], []);
	}

	/**
		Dispatches pending sound-end callbacks.
		Returns number of callbacks dispatched (same as HL update() return value).
		Call this every frame.
	**/
	public static function update():Int {
		final n:Int = Wasm.call("ma_js_update", "number", ["number", "number"], [Wasm.cbOutPtr, Wasm.MAX_CBS]);
		for (i in 0...n) {
			final id:Int = Wasm.mod.getValue(Wasm.cbOutPtr + i * 4, "i32");
			final cb = Wasm.callbacks.get(id);
			if (cb != null)
				cb();
		}
		return n;
	}

	/**
		Returns a string describing the last error that occurred in miniaudio.
	**/
	public static function describeLastError():String {
		final ptr:Int = Wasm.call("ma_js_describe_last_error", "number", [], []);
		return Wasm.mod.UTF8ToString(ptr);
	}

	/**
		Decodes the provided audio file bytes into floating-point PCM data.
	**/
	public static function decodeToPCMFloat(bytes:Bytes):DecodedAudio {
		final u = Wasm.upload(bytes);
		final outPtr:Int = Wasm.call("ma_js_decode_pcm_float", "number", ["number", "number"], [u.ptr, u.len]);
		Wasm.free(u.ptr);
		if (outPtr == 0)
			return null;

		final channels = Wasm.call("ma_js_decoded_channels", "number", [], []);
		final sampleRate = Wasm.call("ma_js_decoded_sample_rate", "number", [], []);
		final samples = Wasm.call("ma_js_decoded_samples", "number", [], []);
		final byteCount = samples * channels * 4;

		// copy from WASM heap into a haxe Bytes (avoids dangling pointer after free)
		final heap:js.lib.Uint8Array = Wasm.mod.HEAPU8;
		final result = Bytes.ofData(heap.slice(outPtr, outPtr + byteCount).buffer);
		Wasm.call("free", null, ["number"], [outPtr]);

		return {
			bytes: result,
			channels: channels,
			sampleRate: sampleRate,
			samples: samples,
			floatFormat: true
		};
	}

	/**
		Decodes the provided audio file bytes into 16-bit integer PCM data.
	**/
	public static function decodeToPCM16(bytes:Bytes):DecodedAudio {
		final u = Wasm.upload(bytes);
		final outPtr:Int = Wasm.call("ma_js_decode_pcm_s16", "number", ["number", "number"], [u.ptr, u.len]);
		Wasm.free(u.ptr);
		if (outPtr == 0)
			return null;

		final channels = Wasm.call("ma_js_decoded_channels", "number", [], []);
		final sampleRate = Wasm.call("ma_js_decoded_sample_rate", "number", [], []);
		final samples = Wasm.call("ma_js_decoded_samples", "number", [], []);
		final byteCount = samples * channels * 2;

		final heap:js.lib.Uint8Array = Wasm.mod.HEAPU8;
		final result = Bytes.ofData(heap.slice(outPtr, outPtr + byteCount).buffer);
		Wasm.call("free", null, ["number"], [outPtr]);

		return {
			bytes: result,
			channels: channels,
			sampleRate: sampleRate,
			samples: samples,
			floatFormat: false
		};
	}
}
