package miniaudio.heaps;

/**
	Initialization macro for Heaps integration.
**/
class Macro {
	/**
		Registers miniaudio extensions in Heaps resource manager.
	**/
	public static macro function main() {
		#if heaps
		final p = "miniaudio.heaps.AudioBuffer";
		hxd.res.Config.addExtension("aiff", p);
		hxd.res.Config.addExtension("wav", p);
		hxd.res.Config.addExtension("flac", p);
		hxd.res.Config.addExtension("ogg", p);
		hxd.res.Config.addExtension("opus", p);
		hxd.res.Config.addExtension("mp3", p);
		#end

		return null;
	}
}
