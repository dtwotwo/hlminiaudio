import haxe.io.Bytes;
import haxe.io.Path;
import hxd.Res;
import hxd.fs.LocalFileSystem;
import hxd.res.Loader;
import hxd.snd.Data;
import hxd.snd.Data.SampleFormat;
import miniaudio.Miniaudio;
import miniaudio.Miniaudio.SoundGroup;
import miniaudio.Miniaudio.StreamDecoder;
import miniaudio.heaps.AudioBuffer;

class TestHeaps {
	static function main() {
		Res.loader = new Loader(new LocalFileSystem("audio/formats", null));
		if (!Miniaudio.init()) {
			Sys.println("FAIL init: " + Miniaudio.describeLastError());
			Sys.exit(1);
		}

		var failed = false;
		try {
			for (path in TestSupport.fixtures) {
				testResource(path);
				TestSupport.printOk(TestSupport.fixtureLabel(path));
			}
		}
		catch (e) {
			failed = true;
			Sys.println("FAIL " + Std.string(e));
		}
		Miniaudio.uninit();

		if (failed)
			Sys.exit(1);
		Sys.println("All deterministic Heaps tests passed.");
	}

	static function testResource(path:String):Void {
		final resourcePath = Path.withoutDirectory(path);
		final resource = Res.loader.loadCache(resourcePath, AudioBuffer);
		TestSupport.assert(resource != null, TestSupport.fixtureLabel(path) + ": resource load failed");
		final direct = TestSupport.decodeFixture(path);
		final decodedA = resource.getData();
		final decodedB = resource.getData();
		TestSupport.assert(decodedA == decodedB, TestSupport.fixtureLabel(path) + ": decoded data cache should be reused");
		assertDataFormatMatches(direct, decodedA, path);
		assertDataStartMatches(direct, decodedA, path, "decoded audio mismatch");

		final stream = resource.getStream();
		TestSupport.assertEquals(direct.floatDecoded.channels, stream.channels, TestSupport.fixtureLabel(path) + ": stream channel count mismatch");
		TestSupport.assertEquals(direct.floatDecoded.sampleRate, stream.sampleRate, TestSupport.fixtureLabel(path) + ": stream sample rate mismatch");
		TestSupport.assertEquals(direct.floatDecoded.samples, stream.samples, TestSupport.fixtureLabel(path) + ": stream sample count mismatch");
		assertStreamDecodeMatches(direct, stream, path);
		stream.dispose();

		final group = new SoundGroup();
		final streamSound = resource.getStreamSound(group);
		TestSupport.assert(streamSound != null, TestSupport.fixtureLabel(path) + ": stream sound creation failed");
		TestSupport.assertEquals(direct.floatDecoded.samples, streamSound.lengthSamples, TestSupport.fixtureLabel(path) + ": stream sound length mismatch");
		TestSupport.assertNear(direct.floatDecoded.samples / direct.floatDecoded.sampleRate, streamSound.durationSeconds, 0.03,
			TestSupport.fixtureLabel(path) + ": stream sound duration mismatch");
		TestSupport.assert(streamSound.start(), TestSupport.fixtureLabel(path) + ": stream sound start failed");
		TestSupport.assert(waitUntil(() -> return streamSound.isPlaying(), 0.25), TestSupport.fixtureLabel(path) + ": stream sound never started");
		streamSound.stop();
		streamSound.dispose();
		group.dispose();

		resource.dispose();
		final decodedC = resource.getData();
		TestSupport.assert(decodedC != decodedA, TestSupport.fixtureLabel(path) + ": dispose should clear cached decoded data");
	}

	static function assertDataFormatMatches(direct:Dynamic, data:Data, path:String):Void {
		TestSupport.assertEquals(direct.floatDecoded.channels, data.channels, TestSupport.fixtureLabel(path) + ": channel count mismatch");
		TestSupport.assertEquals(direct.floatDecoded.sampleRate, data.samplingRate, TestSupport.fixtureLabel(path) + ": sample rate mismatch");
		TestSupport.assertEquals(direct.floatDecoded.samples, data.samples, TestSupport.fixtureLabel(path) + ": sample count mismatch");
		TestSupport.assertEquals(SampleFormat.F32, data.sampleFormat, TestSupport.fixtureLabel(path) + ": heaps data should be float");
	}

	static function assertDataStartMatches(direct:Dynamic, data:Data, path:String, message:String):Void {
		final samplesToCheck = direct.floatDecoded.samples < 8 ? direct.floatDecoded.samples : 8;
		final decodedSlice = Bytes.alloc(samplesToCheck * data.getBytesPerSample());
		data.decode(decodedSlice, 0, 0, samplesToCheck);
		TestSupport.assertBytesEqual(direct.floatDecoded.bytes.sub(0, decodedSlice.length), decodedSlice, TestSupport.fixtureLabel(path) + ": " + message);
	}

	static function assertDataMiddleMatches(direct:Dynamic, data:Data, path:String, message:String):Void {
		final samplesToCheck = direct.floatDecoded.samples < 8 ? direct.floatDecoded.samples : 8;
		final middleSample = Std.int(direct.floatDecoded.samples * 0.5);
		final middleSlice = Bytes.alloc(samplesToCheck * data.getBytesPerSample());
		data.decode(middleSlice, 0, middleSample, samplesToCheck);
		assertFloatBytesNear(direct.floatDecoded.bytes.sub(middleSample * data.getBytesPerSample(), middleSlice.length), middleSlice,
			TestSupport.fixtureLabel(path) + ": " + message);
	}

	static function assertStreamDecodeMatches(direct:Dynamic, stream:StreamDecoder, path:String):Void {
		final samplesToCheck = Std.int(direct.floatDecoded.samples < 8 ? direct.floatDecoded.samples : 8);
		final bpp = Std.int(direct.floatDecoded.channels * 4);
		final middleSample = Std.int(direct.floatDecoded.samples * 0.5);
		final middleSlice = Bytes.alloc(samplesToCheck * bpp);
		final read = stream.decode(middleSlice, 0, middleSample, samplesToCheck);
		TestSupport.assertEquals(samplesToCheck, read, TestSupport.fixtureLabel(path) + ": stream decoder read mismatch");
		assertFloatBytesNear(direct.floatDecoded.bytes.sub(middleSample * bpp, middleSlice.length), middleSlice,
			TestSupport.fixtureLabel(path) + ": stream decoder audio mismatch");
	}

	static function waitUntil(check:Void->Bool, timeoutSeconds:Float):Bool {
		final deadline = haxe.Timer.stamp() + timeoutSeconds;
		while (haxe.Timer.stamp() < deadline) {
			Miniaudio.update();
			if (check())
				return true;
			Sys.sleep(0.01);
		}
		Miniaudio.update();
		return check();
	}

	static function assertFloatBytesNear(expected:Bytes, actual:Bytes, message:String):Void {
		TestSupport.assertEquals(expected.length, actual.length, message + ": byte length mismatch");
		var pos = 0;
		while (pos < expected.length) {
			TestSupport.assertNear(expected.getFloat(pos), actual.getFloat(pos), 1e-3, message + " at byte " + pos);
			pos += 4;
		}
	}
}
