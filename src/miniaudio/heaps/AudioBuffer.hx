package miniaudio.heaps;

import hxd.fs.FileEntry;
import hxd.res.Resource;

class AudioBuffer extends Resource
{
	private var loaded:Bool = false;

	private var buffer:Miniaudio.Buffer;

	public function new(entry:FileEntry)
	{
		super(entry);

		entry.watch(() ->
		{
			buffer.dispose();
			loaded = false;
		});
	}

	public function load():Miniaudio.Buffer
	{
		if (!loaded)
		{
			buffer = Miniaudio.Buffer.fromBytes(entry.getBytes());
			if (buffer == null)
				throw '${entry.path}: ${Miniaudio.describeLastError()}';
		}

		return buffer;
	}
}
