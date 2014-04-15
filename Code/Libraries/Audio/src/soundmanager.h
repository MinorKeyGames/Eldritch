#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

// SoundManager is intended to keep samples in memory in a place where
// they can be reaccessed by filename (so that I don't have to open the
// file repeatedly or worse, load the sound multiple times). Streams
// are also "getted" through this interface, but are not stored in the
// map because a stream can only be played once at a time. (Incidentally,
// the sound/sound instance concept kind of breaks down for streams,
// because there is always just one sound to one instance for streams.)

#include "map.h"
#include "hashedstring.h"

class IAudioSystem;
class ISound;
struct SSoundInit;

class SoundManager
{
public:
	SoundManager( IAudioSystem* pAudioSystem );
	~SoundManager();

	void	FreeSounds();

	ISound*	GetSound( const SSoundInit& SoundInit, const SimpleString& SoundDefinitionName );

private:
	typedef Map<HashedString, ISound*> TSoundMap;
	TSoundMap		m_SoundTable;
	IAudioSystem*	m_AudioSystem;
};

#endif // SOUNDMANAGER_H