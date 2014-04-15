#include "core.h"
#include "wbcompeldsound.h"
#include "wbeventmanager.h"
#include "hashedstring.h"
#include "eldritchframework.h"
#include "configmanager.h"
#include "isoundinstance.h"

WBCompEldSound::WBCompEldSound()
:	m_SoundInstances()
{
}

WBCompEldSound::~WBCompEldSound()
{
	IAudioSystem* const pAudioSystem = GetFramework()->GetAudioSystem();
	ASSERT( pAudioSystem );

	FOR_EACH_ARRAY( SoundInstIter, m_SoundInstances, SSoundInstance )
	{
		SSoundInstance& SoundInstance = SoundInstIter.GetValue();
		ASSERT( SoundInstance.m_SoundInstance );

		pAudioSystem->RemoveSoundInstance( SoundInstance.m_SoundInstance );
	}
	
	SInstanceDeleteCallback Callback;
	Callback.m_Callback	= InstanceDeleteCallback;
	Callback.m_Void		= this;
	pAudioSystem->UnregisterInstanceDeleteCallback( Callback );
}

/*virtual*/ void WBCompEldSound::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Sound );
	const SimpleString Sound = ConfigManager::GetInheritedString( sSound, "", sDefinitionName );
	if( Sound == "" )
	{
		// No ambient sound; this component may still be used to house attached SFX
	}
	else
	{
		ISoundInstance* const pSoundInstance = GetFramework()->GetAudioSystem()->CreateSoundInstance( Sound );
		ASSERT( pSoundInstance );

		SSoundInstance& SoundInstance	= m_SoundInstances.PushBack();
		SoundInstance.m_SoundInstance	= pSoundInstance;
		SoundInstance.m_Attached		= true;
	}

	SInstanceDeleteCallback Callback;
	Callback.m_Callback	= InstanceDeleteCallback;
	Callback.m_Void		= this;
	GetFramework()->GetAudioSystem()->RegisterInstanceDeleteCallback( Callback );
}

/*virtual*/ void WBCompEldSound::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnMoved );
	STATIC_HASHED_STRING( OnLoaded );
	STATIC_HASHED_STRING( PlaySound );
	STATIC_HASHED_STRING( StopSound );
	STATIC_HASHED_STRING( PlayBark );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnMoved || EventName == sOnLoaded )
	{
		STATIC_HASHED_STRING( Location );
		const Vector Location = Event.GetVector( sLocation );

		FOR_EACH_ARRAY( SoundInstIter, m_SoundInstances, SSoundInstance )
		{
			SSoundInstance& SoundInstance = SoundInstIter.GetValue();
			ASSERT( SoundInstance.m_SoundInstance );
			
			if( SoundInstance.m_Attached )
			{
				SoundInstance.m_SoundInstance->SetLocation( Location );
			}

			if( !SoundInstance.m_SoundInstance->IsPlaying() )
			{
				SoundInstance.m_SoundInstance->Tick();	// Tick to commit location change
				SoundInstance.m_SoundInstance->Play();
			}
		}
	}
	else if( EventName == sPlaySound )
	{
		STATIC_HASHED_STRING( Sound );
		const SimpleString SoundDef = Event.GetString( sSound );

		STATIC_HASHED_STRING( Location );
		const Vector Location = Event.GetVector( sLocation );

		STATIC_HASHED_STRING( Volume );
		const float VolumeOverride = Event.GetFloat( sVolume );

		STATIC_HASHED_STRING( Attached );
		const bool Attached = Event.GetBool( sAttached );

		PlaySoundDef( SoundDef, Location, Attached, VolumeOverride );
	}
	else if( EventName == sStopSound )
	{
		STATIC_HASHED_STRING( Category );
		const HashedString Category = Event.GetHash( sCategory );

		StopCategory( Category );
	}
	else if( EventName == sPlayBark )
	{
		STATIC_HASHED_STRING( Category );
		const HashedString Category = Event.GetHash( sCategory );

		STATIC_HASHED_STRING( Sound );
		const SimpleString SoundDef = Event.GetString( sSound );

		STATIC_HASHED_STRING( Location );
		const Vector Location = Event.GetVector( sLocation );

		STATIC_HASHED_STRING( Volume );
		const float VolumeOverride = Event.GetFloat( sVolume );

		StopCategory( Category );
		PlaySoundDef( SoundDef, Location, true, VolumeOverride );
	}
}

void WBCompEldSound::StopCategory( const HashedString& Category )
{
	FOR_EACH_ARRAY( SoundInstIter, m_SoundInstances, SSoundInstance )
	{
		SSoundInstance& SoundInstance = SoundInstIter.GetValue();
		ASSERT( SoundInstance.m_SoundInstance );

		if( SoundInstance.m_SoundInstance->GetCategory() == Category )
		{
			SoundInstance.m_SoundInstance->Stop();
		}
	}
}

void WBCompEldSound::PlaySoundDef( const SimpleString& SoundDef, const Vector& Location, const bool Attached, const float VolumeOverride )
{
	XTRACE_FUNCTION;

	if( SoundDef == "" )
	{
		return;
	}

	IAudioSystem* const pAudioSystem = GetFramework()->GetAudioSystem();
	ISoundInstance* const pSoundInstance = pAudioSystem->CreateSoundInstance( SoundDef );
	ASSERT( pSoundInstance );

	pSoundInstance->SetBaseVolume( ( VolumeOverride > 0.0f ) ? VolumeOverride : 1.0f );
	pSoundInstance->SetLocation( Location );
	pAudioSystem->ConditionalApplyReverb( pSoundInstance );
	pSoundInstance->Tick();
	pSoundInstance->Play();

	SSoundInstance& SoundInstance	= m_SoundInstances.PushBack();
	SoundInstance.m_SoundInstance	= pSoundInstance;
	SoundInstance.m_Attached		= Attached;
}

/*static*/ void WBCompEldSound::InstanceDeleteCallback( void* pVoid, ISoundInstance* pInstance )
{
	WBCompEldSound* pSound = static_cast<WBCompEldSound*>( pVoid );
	ASSERT( pSound );

	pSound->OnInstanceDeleted( pInstance );
}

void WBCompEldSound::OnInstanceDeleted( ISoundInstance* const pInstance )
{
	FOR_EACH_ARRAY_REVERSE( SoundInstIter, m_SoundInstances, SSoundInstance )
	{
		SSoundInstance& SoundInstance = SoundInstIter.GetValue();
		ASSERT( SoundInstance.m_SoundInstance );

		if( pInstance == SoundInstance.m_SoundInstance )
		{
			m_SoundInstances.FastRemove( SoundInstIter );
		}
	}
}