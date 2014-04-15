#include "core.h"
#include "musicmanager.h"
#include "iaudiosystem.h"
#include "isound.h"
#include "isoundinstance.h"
#include "soundmanager.h"
#include "configmanager.h"

MusicManager::MusicManager( IAudioSystem* pAudioSystem )
:	m_AudioSystem( pAudioSystem )
,	m_CurrentSequence()
,	m_NextSequence()
,	m_Mood( "" )
,	m_NextSequenceLeadTime( 0.0f )
,	m_NextSequenceQueueTime( 0.0f )
{
	SInstanceDeleteCallback Callback;
	Callback.m_Callback = InstanceDeleteCallback;
	Callback.m_Void = this;
	m_AudioSystem->RegisterInstanceDeleteCallback( Callback );

	UpdateMusicLevel();

	STATICHASH( NextSequenceLeadTime );
	STATICHASH( NextSequenceQueueTime );

	m_NextSequenceLeadTime = ConfigManager::GetFloat( sNextSequenceLeadTime, 0.01f );
	m_NextSequenceQueueTime = ConfigManager::GetFloat( sNextSequenceQueueTime, 0.1f );
}

MusicManager::~MusicManager()
{
	SInstanceDeleteCallback Callback;
	Callback.m_Callback = InstanceDeleteCallback;
	Callback.m_Void = this;
	m_AudioSystem->UnregisterInstanceDeleteCallback( Callback );
}

void MusicManager::PlaySequence( const SimpleString& DefinitionName )
{
	// Interrupt the current sequences
	Flush();

	// Unify some of this with what happens in tick
	STATICHASH( File );
	MAKEHASH( DefinitionName );
	SimpleString Filename = ConfigManager::GetString( sFile, "", sDefinitionName );
	if( Filename != "" )
	{
		SSoundInit SoundInit;
		SoundInit.m_Filename = Filename.CStr();
		SoundInit.m_IsStream = true;
		SoundInit.m_IsLooping = false;
		SoundInit.m_Is3D = false;
		SoundInit.m_Category = "Music";
		SoundInit.m_Priority = ESP_High;

		m_CurrentSequence.m_Sound = m_AudioSystem->GetSoundManager()->GetSound( SoundInit, DefinitionName );
		m_CurrentSequence.m_Instance = m_CurrentSequence.m_Sound->CreateSoundInstance();
		m_AudioSystem->AddSoundInstance( m_CurrentSequence.m_Instance );
		m_CurrentSequence.m_Instance->Tick();	// Tick to apply all properties
		m_CurrentSequence.m_Instance->SetPaused( false );
		m_CurrentSequence.m_DefinitionName = DefinitionName;
		
		QueueNextSequence();
	}
}

void MusicManager::SetMood( const SimpleString& Mood )
{
	m_Mood = Mood;

	bool CanQueue = true;
	if( m_CurrentSequence.m_Instance )
	{
		const float RemainingTime = m_CurrentSequence.m_Sound->GetLength() - m_CurrentSequence.m_Instance->GetTimeElapsed();
		if( RemainingTime < m_NextSequenceQueueTime )
		{
			CanQueue = false;
		}
	}

	if( CanQueue )
	{
		QueueNextSequence();
	}
}

// This does the same thing as Tick()--it's just in here in case
void MusicManager::OnInstanceDeleted( ISoundInstance* pInstance )
{
	// If current sequence has finished, play the queued
	// sequence and queue up the next based on mood
	// If mood is not matched in sequence transitions,
	// just use next sequence.
	if( m_CurrentSequence.m_Instance == pInstance )
	{
		PlayNextSequence();
	}
	else if( m_NextSequence.m_Instance == pInstance )
	{
		// FMOD stole next instance out from under us; requeue it
		QueueNextSequence();
	}
}

/*static*/ void MusicManager::InstanceDeleteCallback( void* pVoid, ISoundInstance* pInstance )
{
	MusicManager* pManager = static_cast< MusicManager* >( pVoid );
	pManager->OnInstanceDeleted( pInstance );
}

void MusicManager::PlayNextSequence()
{
	m_CurrentSequence = m_NextSequence;
	m_NextSequence.m_Instance = NULL;	// Null this so the new current sequence isn't stopped when we free the next sequence

	if( m_CurrentSequence.m_Instance )
	{
		m_CurrentSequence.m_Instance->SetPaused( false );
	}

	QueueNextSequence();
}

void MusicManager::QueueNextSequence()
{
	FreeSequence( m_NextSequence );

	STATICHASH( NextSequence );
	STATICHASH( NumTransitions );
	STATICHASH( File );
	MAKEHASHFROM( DefinitionName, m_CurrentSequence.m_DefinitionName );

	SimpleString NextSequence = ConfigManager::GetString( sNextSequence, "", sDefinitionName );

	// Try all the mood transitions; if none match, it'll just fall back to NextSequence
	int NumTransitions = ConfigManager::GetInt( sNumTransitions, 0, sDefinitionName );
	for( int i = 0; i < NumTransitions; ++i )
	{
		SimpleString TransitionMood = ConfigManager::GetSequenceString( "TransitionMood%d", i, "", sDefinitionName );
		SimpleString TransitionSequence = ConfigManager::GetSequenceString( "TransitionSeq%d", i, "", sDefinitionName );
		if( TransitionMood == m_Mood )
		{
			NextSequence = TransitionSequence;
			break;
		}
	}

	if( NextSequence != "" )
	{
		SimpleString NextFilename = ConfigManager::GetString( sFile, "", NextSequence );
		if( NextFilename != "" )
		{
			SSoundInit SoundInit;
			SoundInit.m_Filename = NextFilename.CStr();
			SoundInit.m_IsStream = true;
			SoundInit.m_IsLooping = false;
			SoundInit.m_Is3D = false;
			SoundInit.m_Category = "Music";
			SoundInit.m_Priority = ESP_High;

			m_NextSequence.m_Sound = m_AudioSystem->GetSoundManager()->GetSound( SoundInit, NextSequence );
			m_NextSequence.m_Instance = m_NextSequence.m_Sound->CreateSoundInstance();
			m_AudioSystem->AddSoundInstance( m_NextSequence.m_Instance );
			m_NextSequence.m_Instance->Tick();	// Tick to apply all properties in advance
			m_NextSequence.m_DefinitionName = NextSequence;
		}
	}
}

void MusicManager::Tick()
{
	UpdateMusicLevel();

	// This requires non-VBR files, else the length may be calculated incorrectly.
	// This was prone to dereferencing a deleted instance if there was a hitch of
	// a substantial length, which is why the callback doubles up on this behavior.
	if( m_CurrentSequence.m_Instance )
	{
		const float RemainingTime = m_CurrentSequence.m_Sound->GetLength() - m_CurrentSequence.m_Instance->GetTimeElapsed();
		if( RemainingTime < m_NextSequenceLeadTime )
		{
			PlayNextSequence();
		}
	}
}

void MusicManager::UpdateMusicLevel()
{
	STATICHASH( MusicLevelBase );
	STATICHASH( MusicLevelStep );
	STATICHASH( MusicLevel );

	float MusicLevel =
		ConfigManager::GetFloat( sMusicLevelBase ) +
		ConfigManager::GetFloat( sMusicLevelStep ) *
		(float)( ConfigManager::GetInt( sMusicLevel ) - 1 );
	m_AudioSystem->SetCategoryVolume( "Music", MusicLevel, 0.0f );
}

SimpleString MusicManager::GetCurrentSequenceName() const
{
	return m_CurrentSequence.m_DefinitionName;
}

SimpleString MusicManager::GetMood() const
{
	return m_Mood;
}

void MusicManager::FreeSequence( SSequence& Sequence )
{
	if( Sequence.m_Instance )
	{
		Sequence.m_Instance->Stop();
	}
	Sequence.m_Sound = NULL;
	Sequence.m_Instance = NULL;
	Sequence.m_DefinitionName = "";
}

void MusicManager::Flush()
{
	FreeSequence( m_CurrentSequence );
	FreeSequence( m_NextSequence );
}