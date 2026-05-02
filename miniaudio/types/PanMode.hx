package miniaudio.types;

/**
	The panning mode for sounds and groups.
**/
enum abstract PanMode(Int) from Int to Int {
	/**
		Standard balance panning (attenuates one channel while keeping the other at full volume).
	**/
	final Balance = 0;

	/**
		True panning (mixes channels together).
	**/
	final Pan = 1;
}
