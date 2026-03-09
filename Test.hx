import sys.io.File;
import haxe.io.Bytes;
import miniaudio.Miniaudio;

function main()
{
	if (!Miniaudio.init())
		trace(Miniaudio.describeLastError());

	final group:SoundGroup = new SoundGroup();

	final bytes:Bytes = File.getBytes('introTHREE.ogg');
	final buffer:Buffer = Buffer.fromBytes(bytes);
	final sound:Sound = new Sound(buffer, group);

	Sys.println('Press W to close');
	Sys.println('Press any key to play sound');

	while (true)
	{
		final char:Int = Sys.getChar(false);

		switch char
		{
			case 119: // W
				break;

			case 72: // Up
				var vol:Int = Std.int(sound.volume * 10);
				vol++;
				sound.volume = vol / 10;
				Sys.println('  Volume: ${sound.volume}');
			case 80: // Down
				var vol:Int = Std.int(sound.volume * 10);
				if (vol-- < 0)
					vol = 0;
				sound.volume = vol / 10;
				Sys.println('  Volume: ${sound.volume}');

			case 75: // Left
				var pit:Int = Std.int(sound.pitch * 10);
				if (pit-- < 1)
					pit = 1;
				sound.pitch = pit / 10;
				Sys.println('  Pitch: ${sound.pitch}');
			case 77: // Right
				var pit:Int = Std.int(sound.pitch * 10);
				pit++;
				sound.pitch = pit / 10;
				Sys.println('  Pitch: ${sound.pitch}');
		}

		if (!sound.start())
			trace(Miniaudio.describeLastError());
	}

	buffer.dispose();
	Miniaudio.uninit();
}
