#ifndef FMODSOUNDINSTANCE_H
#define FMODSOUNDINSTANCE_H

#include "soundinstancecommon.h"
#include "iaudiosystem.h"
#include "vector.h"

#include "FMOD/fmod.hpp"

class FMODSoundInstance : public SoundInstanceCommon
{
public:
	FMODSoundInstance( ISound* const pSound );
	virtual ~FMODSoundInstance();

	virtual void				Play() { SetPaused( false ); }
	virtual void				Stop();
	virtual void				SetPaused( bool Paused );
	virtual void				SetVolume( float Volume );
	virtual void				SetPan( float Pan );
	virtual void				SetPriority( ESoundPriority Priority ) const;

	virtual bool				IsPlaying() const;
	virtual bool				IsFinished() const;
	virtual float				GetTimeElapsed() const;

	FMOD::Channel*	m_Channel;
};

#endif // FMODSOUNDINSTANCE_H