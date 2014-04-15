#include "core.h"
#include "fmodaudiosystem.h"
#include "soundmanager.h"
#include "fmodsound.h"
#include "fmodsoundinstance.h"
#include "configmanager.h"
#include "mathfunc.h"
#if BUILD_WINDOWS
#include "consolemanager.h"
#endif
#include "interpolator.h"

#define FMOD_ERROR_CHECK( func )						\
	ASSERT( Result == FMOD_OK );						\
	if( Result != FMOD_OK )								\
	{													\
		PRINTF( "FMOD: " #func ": 0x%08X\n", Result );	\
	}

FMODAudioSystem::FMODAudioSystem()
:	m_FMODSystem( NULL )
,	m_ReverbGroup( NULL )
,	m_ReverbDSP( NULL )
,	m_ReverbCategories()
{
	DEBUGPRINTF( "Initializing FMOD audio system...\n" );

	STATICHASH( AudioSystem );

	STATICHASH( MaxChannels );
	const int MaxChannels = ConfigManager::GetInt( sMaxChannels, 32, sAudioSystem );

	// Create FMOD system
	FMOD_RESULT Result;
	Result = FMOD::System_Create( &m_FMODSystem );
	FMOD_ERROR_CHECK( System_Create );

	FMOD_OUTPUTTYPE FMODOutput = FMOD_OUTPUTTYPE_AUTODETECT;
	Result = m_FMODSystem->getOutput( &FMODOutput );
	FMOD_ERROR_CHECK( getOutput );
	PRINTF( "FMOD: Default output type: 0x%08X\n", FMODOutput );

	int NumDrivers = 0;
	Result = m_FMODSystem->getNumDrivers( &NumDrivers );
	FMOD_ERROR_CHECK( getNumDrivers );

#if BUILD_LINUX
	// Seems like FMOD's attempt to evaluate audio outputs isn't working right. So try ALSA, then OSS.
	ASSERT( FMODOutput == FMOD_OUTPUTTYPE_PULSEAUDIO );

	if( Result != FMOD_OK )
	{
		Result = m_FMODSystem->setOutput( FMOD_OUTPUTTYPE_ALSA );
		FMOD_ERROR_CHECK( setOutput );

		Result = m_FMODSystem->getNumDrivers( &NumDrivers );
		FMOD_ERROR_CHECK( getNumDrivers );
	}

	if( Result != FMOD_OK )
	{
		Result = m_FMODSystem->setOutput( FMOD_OUTPUTTYPE_OSS );
		FMOD_ERROR_CHECK( setOutput );

		Result = m_FMODSystem->getNumDrivers( &NumDrivers );
		FMOD_ERROR_CHECK( getNumDrivers );
	}
#endif

	ASSERT( NumDrivers > 0 );
	if( NumDrivers <= 0 )
	{
		PRINTF( "FMOD: No sound drivers available.\n" );
		Result = m_FMODSystem->setOutput( FMOD_OUTPUTTYPE_NOSOUND );
		FMOD_ERROR_CHECK( setOutput );
	}

	Result = m_FMODSystem->init( MaxChannels, FMOD_INIT_NORMAL, NULL );	// First parameter is maxchannels, up to 4093
	FMOD_ERROR_CHECK( init );

	// Create reverb channel group
	Result = m_FMODSystem->createChannelGroup( "Reverb", &m_ReverbGroup );
	FMOD_ERROR_CHECK( createChannelGroup );

	// Create reverb DSP unit
	Result = m_FMODSystem->createDSPByType( FMOD_DSP_TYPE_SFXREVERB, &m_ReverbDSP );
	FMOD_ERROR_CHECK( createDSPByType );

	// Set up DSP chain
	Result = m_ReverbGroup->addDSP( m_ReverbDSP, NULL );
	FMOD_ERROR_CHECK( addDSP );

	STATICHASH( DefaultReverb );
	const SimpleString DefaultReverb = ConfigManager::GetString( sDefaultReverb, "", sAudioSystem );
	SetReverbParams( DefaultReverb );

	STATICHASH( NumReverbCategories );
	const uint NumReverbCategories = ConfigManager::GetInt( sNumReverbCategories, 0, sAudioSystem );
	for( uint ReverbCategoryIndex = 0; ReverbCategoryIndex < NumReverbCategories; ++ReverbCategoryIndex )
	{
		const HashedString ReverbCategory = ConfigManager::GetSequenceHash( "ReverbCategory%d", ReverbCategoryIndex, "", sAudioSystem );
		m_ReverbCategories.PushBack( ReverbCategory );
	}
}

FMODAudioSystem::~FMODAudioSystem()
{
	// Free sound instances before deleting sound manager, else
	// querying if sounds are streams when deleting instances will fail
	FreeSoundInstances();

	// Parent class owns this, but order of deletion matters...
	SafeDelete( m_SoundManager );

	m_ReverbDSP->release();
	m_ReverbGroup->release();
	m_FMODSystem->close();
	m_FMODSystem->release();
	m_FMODSystem = NULL;
}

void FMODAudioSystem::Tick( float DeltaTime, bool GamePaused )
{
	XTRACE_FUNCTION;

	AudioSystemCommon::Tick( DeltaTime, GamePaused );

	m_FMODSystem->update();
}

ISound* FMODAudioSystem::CreateSound( const SSoundInit& SoundInit )
{
	FMODSound* const pSound = new FMODSound( this, SoundInit );
	pSound->Initialize( SoundInit );
	return pSound;
}

ISoundInstance* FMODAudioSystem::Play( const SimpleString& DefinitionName, const Vector& Location )
{
	ISoundInstance* const pInstance = CreateSoundInstance( DefinitionName );
	ASSERT( pInstance );

	// Set variables, then unpause
	pInstance->SetLocation( Location );

	// Apply reverb
	for( uint i = 0; i < m_ReverbCategories.Size(); ++i )
	{
		if( pInstance->GetCategory() == m_ReverbCategories[i] )
		{
			// HACKY, but I don't really need to wrap channel groups in my own
			// custom interface when I'm using them for like ONE THING.
			( ( FMODSoundInstance* )pInstance )->m_Channel->setChannelGroup( m_ReverbGroup );
			break;
		}
	}

	pInstance->Tick();	// To make sure all changes are applied
	pInstance->SetPaused( false );

	return pInstance;
}

/*virtual*/ void FMODAudioSystem::ConditionalApplyReverb( ISoundInstance* const pSoundInstance ) const
{
	ASSERT( pSoundInstance );

	const HashedString& SoundCategory = pSoundInstance->GetCategory();
	for( uint ReverbCategoryIndex = 0; ReverbCategoryIndex < m_ReverbCategories.Size(); ++ReverbCategoryIndex )
	{
		const HashedString& ReverbCategory = m_ReverbCategories[ ReverbCategoryIndex ];
		if( SoundCategory == ReverbCategory )
		{
			FMODSoundInstance* const pFMODInstance = static_cast<FMODSoundInstance*>( pSoundInstance );
			pFMODInstance->m_Channel->setChannelGroup( m_ReverbGroup );
			break;
		}
	}
}

void FMODAudioSystem::SetReverbParams( const SimpleString& DefinitionName ) const
{
	// Can use getNumParameters/getParameterInfo to enumerate parameters, but
	// these SHOULD all work correctly because they're pre-enumerated in FMOD.

	STATICHASH( DryLevel );
	STATICHASH( Room );
	STATICHASH( RoomHF );
	STATICHASH( DecayTime );
	STATICHASH( DecayHFRatio );
	STATICHASH( ReflectionsLevel );
	STATICHASH( ReflectionsDelay );
	STATICHASH( ReverbLevel );
	STATICHASH( ReverbDelay );
	STATICHASH( Diffusion );
	STATICHASH( Density );
	STATICHASH( HFReference );
	STATICHASH( RoomLF );
	STATICHASH( LFReference );
	MAKEHASH( DefinitionName );

	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_DRYLEVEL, ConfigManager::GetFloat( sDryLevel, 0.0f, sDefinitionName ) );
	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_ROOM, ConfigManager::GetFloat( sRoom, -10000.0f, sDefinitionName ) );
	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_ROOMHF, ConfigManager::GetFloat( sRoomHF, 0.0f, sDefinitionName ) );
	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_DECAYTIME, ConfigManager::GetFloat( sDecayTime, 1.0f, sDefinitionName ) );
	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_DECAYHFRATIO, ConfigManager::GetFloat( sDecayHFRatio, 0.5f, sDefinitionName ) );
	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_REFLECTIONSLEVEL, ConfigManager::GetFloat( sReflectionsLevel, -10000.0f, sDefinitionName ) );
	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_REFLECTIONSDELAY, ConfigManager::GetFloat( sReflectionsDelay, 0.02f, sDefinitionName ) );
	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_REVERBLEVEL, ConfigManager::GetFloat( sReverbLevel, 0.0f, sDefinitionName ) );
	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_REVERBDELAY, ConfigManager::GetFloat( sReverbDelay, 0.04f, sDefinitionName ) );
	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_DIFFUSION, ConfigManager::GetFloat( sDiffusion, 100.0f, sDefinitionName ) );
	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_DENSITY, ConfigManager::GetFloat( sDensity, 100.0f, sDefinitionName ) );
	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_HFREFERENCE, ConfigManager::GetFloat( sHFReference, 5000.0f, sDefinitionName ) );
	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_ROOMLF, ConfigManager::GetFloat( sRoomLF, 0.0f, sDefinitionName ) );
	m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_LFREFERENCE, ConfigManager::GetFloat( sLFReference, 250.0f, sDefinitionName ) );
}

#if BUILD_WINDOWS
bool FMODAudioSystem::ProcessCommand( const SimpleString& Command )
{
	if( Command == "duckcategory" )
	{
		HashedString Category = ConsoleManager::GetString();
		float Volume = ConsoleManager::GetFloat();
		float InterpolationTime = ConsoleManager::GetFloat();
		SetCategoryVolume( Category, Volume, InterpolationTime );
		return true;
	}
	else if( Command == "playsound" )
	{
		SimpleString Definition = ConsoleManager::GetString();
		float X = ConsoleManager::GetFloat();
		float Y = ConsoleManager::GetFloat();
		float Z = ConsoleManager::GetFloat();
		Play( Definition, Vector( X, Y, Z ) );
		return true;
	}
	else if( Command == "setreverb" )
	{
		SetReverbParams( ConsoleManager::GetString() );
		return true;
	}
#if BUILD_DEV
	else if( Command == "testaudiolimits" )
	{
		SimpleString Definition = ConsoleManager::GetString();
		float X = ConsoleManager::GetFloat();
		float Y = ConsoleManager::GetFloat();
		float Z = ConsoleManager::GetFloat();
		int NumInstances = ConsoleManager::GetInt();
		Vector Origin( X, Y, Z );
		for( int SoundIndex = 0; SoundIndex < NumInstances; ++SoundIndex )
		{
			Play( Definition, Origin );
		}
		return true;
	}
#endif
	else
	{
		return false;
	}
}
#endif