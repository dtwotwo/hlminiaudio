package miniaudio.heaps;

import haxe.io.Bytes;
import hxd.snd.Data;
import hxd.snd.Data.SampleFormat;

/**
	A Heaps sound resource that uses miniaudio for decoding and playback.
**/
class AudioBuffer extends hxd.res.Sound {
	private var decodedData:Null<Data>;
	private var buffer:Null<Miniaudio.Buffer>;

	override public function getData():Data {
		return getDecodedData();
	}

	/**
		Returns the decoded PCM data for this sound.
	**/
	public function getDecodedData():Data {
		if (decodedData != null)
			return decodedData;

		final decoded = Miniaudio.decodeToPCMFloat(entry.getBytes());
		if (decoded == null)
			throw '${entry.path}: ${Miniaudio.describeLastError()}';

		decodedData = new DecodedAudioData(decoded.bytes, decoded.channels, decoded.sampleRate, decoded.samples);
		watch(watchCallb);
		return decodedData;
	}

	/**
		Returns a miniaudio Buffer for this sound.
	**/
	public function getBuffer():Miniaudio.Buffer {
		buffer ??= Miniaudio.Buffer.fromBytes(entry.getBytes());
		if (buffer == null)
			throw '${entry.path}: ${Miniaudio.describeLastError()}';

		return buffer;
	}

	/**
		Returns a new miniaudio StreamDecoder for this sound.
	**/
	public function getStream():Miniaudio.StreamDecoder {
		final stream = Miniaudio.StreamDecoder.open(entry.getBytes());
		if (stream == null)
			throw '${entry.path}: ${Miniaudio.describeLastError()}';

		return stream;
	}

	/**
		Returns a new streaming miniaudio Sound for this sound.
	**/
	public function getStreamSound(?parent:Miniaudio.SoundGroup):Miniaudio.Sound {
		final stream = getStream();
		final sound = Miniaudio.Sound.fromStream(stream, parent);
		if (sound == null) {
			stream.dispose();
			throw '${entry.path}: ${Miniaudio.describeLastError()}';
		}

		return sound;
	}

	/**
		Disposes of the sound and its resources.
	**/
	override public function dispose() {
		stop();

		decodedData = null;
		if (buffer != null) {
			buffer.dispose();
			buffer = null;
		}

		#if hl
		Miniaudio.gc();
		#end
	}
}

private class DecodedAudioData extends Data {
	final pcm:Bytes;

	public function new(pcm:Bytes, channels:Int, sampleRate:Int, samples:Int) {
		this.pcm = pcm;
		@:privateAccess this.channels = channels;
		@:privateAccess this.samplingRate = sampleRate;
		@:privateAccess this.samples = samples;
		@:privateAccess this.sampleFormat = SampleFormat.F32;
	}

	override function decodeBuffer(out:Bytes, outPos:Int, sampleStart:Int, sampleCount:Int):Void {
		final bpp = getBytesPerSample();
		out.blit(outPos, pcm, sampleStart * bpp, sampleCount * bpp);
	}
}
