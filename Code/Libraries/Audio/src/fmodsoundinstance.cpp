#include "core.h"
#include "fmodsoundinstance.h"
#include "isound.h"
#include "iaudiosystem.h"
#include "configmanager.h"
#include "mathcore.h"
#include "sound3dlistener.h"

FMODSoundInstance::FMODSoundInstance( ISound* const pSound )
:	m_Channel( NULL )
{
	SetSound( pSound );
}

FMODSoundInstance::~FMODSoundInstance()
{
	if( !IsFinished() )
	{
		Stop();
	}

	// Delete the corresponding sound iff it's a stream (incidentally, stream sounds
	// only get automatically deleted if they're ever actually played).
	if( GetSound()->GetIsStream() )
	{
		SafeDelete( m_Sound );
	}

	m_Channel = NULL;
}

void FMODSoundInstance::Stop()
{
	m_Channel->stop();
}

void FMODSoundInstance::SetPriority( ESoundPriority Priority ) const
{
	if( Priority == ESP_Default )
	{
		m_Channel->setPriority( 128 );
	}
	else if( Priority == ESP_High )
	{
		m_Channel->setPriority( 0 );
	}
}

void FMODSoundInstance::SetPaused( bool Paused )
{
	m_Channel->setPaused( Paused );
}

void FMODSoundInstance::SetVolume( float Volume )
{
	m_Channel->setVolume( Volume );
}

void FMODSoundInstance::SetPan( float Pan )
{
	m_Channel->setPan( Pan );
}

/*virtual*/ bool FMODSoundInstance::IsPlaying() const
{
	bool Playing = false;
	m_Channel->isPlaying( &Playing );

	return Playing;
}

bool FMODSoundInstance::IsFinished() const
{
	bool Paused = false;
	bool Playing = false;

	m_Channel->getPaused( &Paused );
	m_Channel->isPlaying( &Playing );

	return !( Paused || Playing );
}

float FMODSoundInstance::GetTimeElapsed() const
{
	uint Milliseconds = 0;
	m_Channel->getPosition( &Milliseconds, FMOD_TIMEUNIT_MS );
	return (float)Milliseconds / 1000.0f;
}