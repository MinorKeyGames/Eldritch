#include "core.h"
#include "audiosystemcommon.h"
#include "configmanager.h"
#include "soundmanager.h"
#include "isound.h"
#include "mathfunc.h"
#include "packstream.h"
#include "isoundinstance.h"

AudioSystemCommon::AudioSystemCommon()
:	m_SoundManager( NULL )
,	m_Sound3DListener( NULL )
,	m_SoundInstances()
,	m_InstanceDeleteCallbacks()
,	m_MasterVolume( 1.0f )
,	m_CategoryVolumes()
,	m_PauseCategories()
{
	STATICHASH( AudioSystem );
	STATICHASH( NumPauseCategories );
	const uint NumPauseCategories = ConfigManager::GetInt( sNumPauseCategories, 0, sAudioSystem );
	for( uint PauseCategoryIndex = 0; PauseCategoryIndex < NumPauseCategories; ++PauseCategoryIndex )
	{
		const HashedString PauseCategory = ConfigManager::GetSequenceHash( "PauseCategory%d", PauseCategoryIndex, HashedString::NullString, sAudioSystem );
		m_PauseCategories.PushBack( PauseCategory );
	}
}

AudioSystemCommon::~AudioSystemCommon()
{
	FreeSoundInstances();
	SafeDelete( m_SoundManager );
}


void AudioSystemCommon::Tick( float DeltaTime, bool GamePaused )
{
	XTRACE_FUNCTION;
	PROFILE_FUNCTION;

	FOR_EACH_MAP( CategoryVolumeIter, m_CategoryVolumes, HashedString, Interpolator<float> )
	{
		Interpolator<float>& VolumeInterp = CategoryVolumeIter.GetValue();
		VolumeInterp.Tick( DeltaTime );
	}

	for( Set<ISoundInstance*>::Iterator SoundIter = m_SoundInstances.Begin(); SoundIter != m_SoundInstances.End(); )
	{
		ISoundInstance* pInstance = *SoundIter;
		DEVASSERT( pInstance );

		// Delete any expired sound instances (which also deletes
		// associated sounds if they're streams).
		if( pInstance->IsFinished() )
		{
			FreeSoundInstance( pInstance );
			m_SoundInstances.Remove( SoundIter );
			continue;
		}

		pInstance->Tick();

		// TODO AUDIO: It would be nice to get this out of tick and into an event or something.
		for( uint PauseCategoryIndex = 0; PauseCategoryIndex < m_PauseCategories.Size(); ++PauseCategoryIndex )
		{
			const HashedString& PauseCategory = m_PauseCategories[ PauseCategoryIndex ];
			if( pInstance->GetCategory() == PauseCategory )
			{
				pInstance->SetPaused( GamePaused );
			}
		}

		++SoundIter;
	}
}

/*virtual*/ SoundManager* AudioSystemCommon::GetSoundManager()
{
	if( !m_SoundManager )
	{
		m_SoundManager = new SoundManager( this );
	}

	return m_SoundManager;
}

/*virtual*/ ISound* AudioSystemCommon::GetSound( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Archetype );
	const SimpleString ArchetypeName = ConfigManager::GetString( sArchetype, "", sDefinitionName );

	MAKEHASH( ArchetypeName );

	SSoundInit SoundInit;

	STATICHASH( Source );
	SoundInit.m_Filename = ConfigManager::GetArchetypeString( sSource, sArchetypeName, "", sDefinitionName );

	STATICHASH( Stream );
	SoundInit.m_IsStream = ConfigManager::GetArchetypeBool( sStream, sArchetypeName, false, sDefinitionName );

	STATICHASH( Looping );
	SoundInit.m_IsLooping = ConfigManager::GetArchetypeBool( sLooping, sArchetypeName, false, sDefinitionName );

	STATICHASH( 3DSound );
	SoundInit.m_Is3D = ConfigManager::GetArchetypeBool( s3DSound, sArchetypeName, false, sDefinitionName );

	STATICHASH( Volume );
	SoundInit.m_Volume = ConfigManager::GetArchetypeFloat( sVolume, sArchetypeName, 1.0f, sDefinitionName );

	STATICHASH( FalloffRadius );
	SoundInit.m_FalloffDistance = ConfigManager::GetArchetypeFloat( sFalloffRadius, sArchetypeName, 0.0f, sDefinitionName );

	STATICHASH( PanBiasNear );
	SoundInit.m_PanBiasNear = ConfigManager::GetArchetypeFloat( sPanBiasNear, sArchetypeName, 0.0f, sDefinitionName );

	STATICHASH( PanBiasFar );
	SoundInit.m_PanBiasFar = ConfigManager::GetArchetypeFloat( sPanBiasFar, sArchetypeName, 0.0f, sDefinitionName );

	STATICHASH( PanPower );
	SoundInit.m_PanPower = ConfigManager::GetArchetypeFloat( sPanPower, sArchetypeName, 1.0f, sDefinitionName );

	STATICHASH( MinimumAttenuation );
	SoundInit.m_MinimumAttenuation = ConfigManager::GetArchetypeFloat( sMinimumAttenuation, sArchetypeName, 0.0f, sDefinitionName );

	STATICHASH( RearAttenuation );
	SoundInit.m_RearAttenuation = ConfigManager::GetArchetypeFloat( sRearAttenuation, sArchetypeName, 0.0f, sDefinitionName );

	STATICHASH( ShouldCalcOcclusion );
	SoundInit.m_ShouldCalcOcclusion = ConfigManager::GetArchetypeBool( sShouldCalcOcclusion, sArchetypeName, false, sDefinitionName );

	STATICHASH( Category );
	SoundInit.m_Category = ConfigManager::GetArchetypeHash( sCategory, sArchetypeName, HashedString::NullString, sDefinitionName );

	STATICHASH( Localized );
	bool Localized = ConfigManager::GetArchetypeBool( sLocalized, sArchetypeName, false, sDefinitionName );

	// If a single source isn't specified, use weighted random selection
	if( SoundInit.m_Filename == "" )
	{
		SoundInit.m_Filename = GetRandomSource( DefinitionName, ArchetypeName );
	}

	// Look up the localized filename from the source name as needed
	if( Localized )
	{
		MAKEHASHFROM( SourceName, SoundInit.m_Filename );
		SoundInit.m_Filename = ConfigManager::GetLocalizedString( sSourceName, "" );
	}

	ASSERT( SoundInit.m_Filename != "" );

	ISound* const pSound = GetSoundManager()->GetSound( SoundInit, DefinitionName );
	ASSERT( pSound );

	return pSound;
}

/*virtual*/ ISoundInstance* AudioSystemCommon::CreateSoundInstance( const SimpleString& DefinitionName )
{
	XTRACE_FUNCTION;

	ISound* const			pSound		= GetSound( DefinitionName );
	ASSERT( pSound );

	ISoundInstance* const	pInstance	= pSound->CreateSoundInstance();
	ASSERT( pInstance );

	AddSoundInstance( pInstance );

	return pInstance;
}

SimpleString AudioSystemCommon::GetRandomSource( const SimpleString& DefinitionName, const SimpleString& ArchetypeName ) const
{
	MAKEHASH( DefinitionName );
	MAKEHASH( ArchetypeName );

	STATICHASH( NumSources );
	const uint NumSources = ConfigManager::GetArchetypeInt( sNumSources, sArchetypeName, 0, sDefinitionName );

#if BUILD_DEV
	if( NumSources == 0 )
	{
		PRINTF( "No audio sources specified for sound %s.\n", DefinitionName.CStr() );
	}
	ASSERTDESC( NumSources, "No Source or NumSources specified for audio definition." );
#endif

	float WeightSum = 0.0f;
	Array<float> CumulativeWeightArray;
	CumulativeWeightArray.ResizeZero( NumSources );
	for( uint SourceIndex = 0; SourceIndex < NumSources; ++SourceIndex )
	{
		WeightSum += ConfigManager::GetArchetypeSequenceFloat( "Weight%d", SourceIndex, sArchetypeName, 1.0f, sDefinitionName );
		CumulativeWeightArray[ SourceIndex ] = WeightSum;
	}

	const float RolledWeight = Math::Random( 0.0f, WeightSum );
	for( uint SourceIndex = 0; SourceIndex < NumSources; ++SourceIndex )
	{
		if( RolledWeight <= CumulativeWeightArray[ SourceIndex ] )
		{
			return ConfigManager::GetArchetypeSequenceString( "Source%d", SourceIndex, sArchetypeName, NULL, sDefinitionName );
			break;
		}
	}

	// We should never get here!
	WARN;
	return NULL;
}

/*virtual*/ void AudioSystemCommon::AddSoundInstance( ISoundInstance* pSoundInstance )
{
	ASSERT( pSoundInstance );
	DEBUGASSERT( m_SoundInstances.Search( pSoundInstance ).IsNull() );

	m_SoundInstances.Insert( pSoundInstance );
}

/*virtual*/ void AudioSystemCommon::RemoveSoundInstance( ISoundInstance* pSoundInstance )
{
	ASSERT( pSoundInstance );
	DEBUGASSERT( m_SoundInstances.Search( pSoundInstance ).IsValid() );

	FreeSoundInstance( pSoundInstance );
	m_SoundInstances.Remove( pSoundInstance );
}

void AudioSystemCommon::FreeSoundInstance( ISoundInstance* pSoundInstance )
{
	ASSERT( pSoundInstance );

	// Maybe make this a separate callback in a common struct (a la SAnimationListener)?
	const uint NumCallbacks = m_InstanceDeleteCallbacks.Size();
	for( uint CallbackIndex = 0; CallbackIndex < NumCallbacks; ++CallbackIndex )
	{
		const SInstanceDeleteCallback& Callback = m_InstanceDeleteCallbacks[ CallbackIndex ];
		Callback.m_Callback( Callback.m_Void, pSoundInstance );
	}

	SafeDelete( pSoundInstance );
}

/*virtual*/ bool AudioSystemCommon::IsValid( ISoundInstance* pSoundInstance ) const
{
	Set<ISoundInstance*>::Iterator SoundIter = m_SoundInstances.Search( pSoundInstance );
	return SoundIter.IsValid();
}

/*virtual*/ void AudioSystemCommon::FreeSoundInstances()
{
	XTRACE_FUNCTION;

	FOR_EACH_SET( SoundIter, m_SoundInstances, ISoundInstance* )
	{
		ISoundInstance* pInstance = SoundIter.GetValue();
		FreeSoundInstance( pInstance );
	}

	m_SoundInstances.Clear();
}

/*virtual*/ float AudioSystemCommon::GetCategoryVolume( const HashedString& Category ) const
{
	Map<HashedString, Interpolator<float> >::Iterator CategoryIterator = m_CategoryVolumes.Search( Category );
	return CategoryIterator.IsValid() ? CategoryIterator.GetValue().GetValue() : 1.0f;
}

/*virtual*/ void AudioSystemCommon::SetCategoryVolume( const HashedString& Category, float Volume, float InterpolationTime )
{
	const float CurrentValue = GetCategoryVolume( Category );
	m_CategoryVolumes[ Category ].Reset( Interpolator<float>::EIT_Linear, CurrentValue, Volume, InterpolationTime );
}

/*virtual*/ void AudioSystemCommon::RegisterInstanceDeleteCallback( const SInstanceDeleteCallback& Callback )
{
	m_InstanceDeleteCallbacks.PushBack( Callback );
}

/*virtual*/ void AudioSystemCommon::UnregisterInstanceDeleteCallback( const SInstanceDeleteCallback& Callback )
{
	m_InstanceDeleteCallbacks.RemoveItem( Callback );
}