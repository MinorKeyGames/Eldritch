#include "core.h"
#include "fmodsound.h"
#include "packstream.h"
#include "fmodsoundinstance.h"
#include "fmodaudiosystem.h"

FMODSound::FMODSound(
	IAudioSystem* const pSystem,
	const SSoundInit& SoundInit )
:	m_Sound( NULL )
,	m_FMODSystem( NULL )
{
	ASSERT( pSystem );
	SetAudioSystem( pSystem );

	FMODAudioSystem* const pFMODAudioSystem = static_cast<FMODAudioSystem*>( pSystem );
	m_FMODSystem = pFMODAudioSystem->GetFMODSystem();

	ASSERT( SoundInit.m_Filename != "" );
	ASSERT( m_FMODSystem );

	if( SoundInit.m_IsStream )
	{
		CreateStream( PackStream( SoundInit.m_Filename.CStr() ), SoundInit.m_IsLooping );
	}
	else
	{
		CreateSample( PackStream( SoundInit.m_Filename.CStr() ), SoundInit.m_IsLooping );
	}
}

FMODSound::~FMODSound()
{
	m_FMODSystem = NULL;

	m_Sound->release();
	m_Sound = NULL;
}

/*virtual*/ ISoundInstance* FMODSound::CreateSoundInstance()
{
	FMODSoundInstance* const	pInstance	= new FMODSoundInstance( this );
	const FMOD_RESULT			Result		= m_FMODSystem->playSound( FMOD_CHANNEL_FREE, m_Sound, true, &pInstance->m_Channel );

	ASSERT( Result == FMOD_OK );
	Unused( Result );

	pInstance->SetPriority( GetPriority() );

	return pInstance;
}

void FMODSound::CreateSample( const IDataStream& Stream, bool Looping )
{
	int Length = Stream.Size();
	byte* pBuffer = new byte[ Length ];
	Stream.Read( Length, pBuffer );

	FMOD_CREATESOUNDEXINFO ExInfo = { 0 };
	ExInfo.cbsize = sizeof( FMOD_CREATESOUNDEXINFO );
	ExInfo.length = Length;

	FMOD_RESULT Result;
	FMOD_MODE Mode = FMOD_OPENMEMORY | FMOD_SOFTWARE;	// Use software so we can apply reverb
	Mode |= Looping ? FMOD_LOOP_NORMAL : 0;
	Result = m_FMODSystem->createSound( ( const char* )pBuffer, Mode, &ExInfo, &m_Sound );
	Unused( Result );

	SafeDeleteArray( pBuffer );
}

void FMODSound::CreateStream( const PackStream& Stream, bool Looping )
{
	const char*	pFilename = Stream.GetPhysicalFilename();
	uint		Offset = Stream.GetSubfileOffset();
	uint		Length = Stream.GetSubfileLength();

	FMOD_CREATESOUNDEXINFO ExInfo = { 0 };
	ExInfo.cbsize = sizeof( FMOD_CREATESOUNDEXINFO );
	ExInfo.length = Length;
	ExInfo.fileoffset = Offset;

	FMOD_MODE Mode = FMOD_CREATESTREAM | FMOD_SOFTWARE;	// Use software so we can apply reverb
	Mode |= Looping ? FMOD_LOOP_NORMAL : 0;
	FMOD_RESULT Result = m_FMODSystem->createSound( pFilename, Mode, &ExInfo, &m_Sound );
	ASSERT( Result == FMOD_OK );
	Unused( Result );
}

float FMODSound::GetLength() const
{
	uint Milliseconds = 0;
	m_Sound->getLength( &Milliseconds, FMOD_TIMEUNIT_MS );
	return (float)Milliseconds / 1000.0f;
}