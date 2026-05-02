package miniaudio.types;

import haxe.io.Bytes;

/**
	Information about a decoded audio stream.
**/
typedef DecodedAudio = {
	/**
		The raw PCM bytes.
	**/
	bytes:Bytes,

	/**
		The number of audio channels.
	**/
	channels:Int,

	/**
		The sample rate in Hz.
	**/
	sampleRate:Int,

	/**
		The total number of samples.
	**/
	samples:Int,

	/**
		Whether the data is in floating-point format (true) or 16-bit integer format (false).
	**/
	floatFormat:Bool,
}
