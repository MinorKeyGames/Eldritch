#ifndef FMODAUDIOSYSTEM_H
#define FMODAUDIOSYSTEM_H

#include "audiosystemcommon.h"
#include "map.h"
#include "set.h"
#include "list.h"
#include "hashedstring.h"
#include "array.h"
#include "interpolator.h"

#include "FMOD/fmod.hpp"

class FMODAudioSystem : public AudioSystemCommon
{
public:
	FMODAudioSystem();
	virtual ~FMODAudioSystem();

	virtual void			Tick( float DeltaTime, bool GamePaused );
	virtual ISound*			CreateSound( const SSoundInit& SoundInit );
	virtual ISoundInstance*	Play( const SimpleString& DefinitionName, const Vector& Location );
	virtual void			SetReverbParams( const SimpleString& DefinitionName ) const;
	virtual void			ConditionalApplyReverb( ISoundInstance* const pSoundInstance ) const;

#if BUILD_WINDOWS
	bool					ProcessCommand( const SimpleString& Command );	// Return true if command is handled
#endif
	FMOD::System*			GetFMODSystem() const { return m_FMODSystem; }

private:
	FMOD::System*		m_FMODSystem;
	FMOD::ChannelGroup*	m_ReverbGroup;
	FMOD::DSP*			m_ReverbDSP;

	Array<HashedString>	m_ReverbCategories;	// Categories that has teh reverbs (probably generally the same as pause)
};

#endif // FMODAUDIOSYSTEM_H