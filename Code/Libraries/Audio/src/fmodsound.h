#ifndef FMODSOUND_H
#define FMODSOUND_H

#include "soundcommon.h"
#include "FMOD/fmod.hpp"

class IDataStream;
class PackStream;
struct SSoundInit;

class FMODSound : public SoundCommon
{
public:
	FMODSound(
		IAudioSystem* const pSystem,
		const SSoundInit& SoundInit );
	virtual ~FMODSound();

	virtual ISoundInstance*		CreateSoundInstance();

	virtual float				GetLength() const;

private:
	void			CreateSample( const IDataStream& Stream, bool Looping );
	void			CreateStream( const PackStream& Stream, bool Looping );

	FMOD::Sound*		m_Sound;
	FMOD::System*		m_FMODSystem;

	// It's maybe a bit hack to store the FMOD::System* instead of
	// an FMODAudioSystem*, but it's just easier to call directly to
	// it instead of routing everything through the ISoundSystem*,
	// and there's no way I'd be using an FMODSound with any other
	// sound system, so the OO implications are moot.
};

#endif // FMODSOUND_H