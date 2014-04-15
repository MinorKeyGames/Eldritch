#include "core.h"
#include "wbcompeldparticles.h"
#include "eldritchparticles.h"
#include "configmanager.h"
#include "wbcompeldtransform.h"
#include "wbcompeldcamera.h"
#include "wbeventmanager.h"
#include "idatastream.h"
#include "mathcore.h"
#include "eldritchgame.h"

WBCompEldParticles::WBCompEldParticles()
:	m_ParticleSystems()
,	m_Hidden( false )
,	m_CullDistanceSq( 0.0f )
{
	STATIC_HASHED_STRING( OnWorldLoaded );
	GetEventManager()->AddObserver( sOnWorldLoaded, this );
}

WBCompEldParticles::~WBCompEldParticles()
{
	FOR_EACH_ARRAY( PSIter, m_ParticleSystems, SParticleSystem )
	{
		SParticleSystem& PS = PSIter.GetValue();
		SafeDelete( PS.m_System );
	}

	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( OnWorldLoaded );
		pEventManager->RemoveObserver( sOnWorldLoaded, this );
	}
}

/*virtual*/ void WBCompEldParticles::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( EldParticles );

	const bool		Serialize			= false;
	const bool		Attached			= true;
	const Vector*	pLocationOverride	= NULL;

	STATICHASH( Particles );
	const SimpleString ParticlesDef = ConfigManager::GetInheritedString( sParticles, "", sDefinitionName );
	if( ParticlesDef != "" )
	{
		AddParticleSystem( ParticlesDef, Serialize, Attached, pLocationOverride );
	}

	STATICHASH( NumParticleSystems );
	const uint NumParticleSystems = ConfigManager::GetInheritedInt( sNumParticleSystems, 0, sDefinitionName );
	for( uint ParticleSystemIndex = 0; ParticleSystemIndex < NumParticleSystems; ++ParticleSystemIndex )
	{
		const SimpleString ParticleSystemDef = ConfigManager::GetInheritedSequenceString( "ParticleSystem%d", ParticleSystemIndex, "", sDefinitionName );
		AddParticleSystem( ParticleSystemDef, Serialize, Attached, pLocationOverride );
	}

	STATICHASH( CullDistance );
	const float DefaultCullDistance = ConfigManager::GetFloat( sCullDistance, 0.0f, sEldParticles );
	m_CullDistanceSq = Square( ConfigManager::GetInheritedFloat( sCullDistance, DefaultCullDistance, sDefinitionName ) );
}

void WBCompEldParticles::AddParticleSystem( const SimpleString& DefinitionName, const bool Serialize, const bool Attached, const Vector* const pLocationOverride )
{
	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector Location		= pLocationOverride ? ( *pLocationOverride ) : pTransform->GetLocation();
	const Angles Orientation	= pTransform->GetOrientation();

	AddParticleSystem( DefinitionName, Serialize, Attached, Location, Orientation );
}

void WBCompEldParticles::AddParticleSystem( const SimpleString& DefinitionName, const bool Serialize, const bool Attached, const Vector& Location, const Angles& Orientation )
{
	ASSERT( DefinitionName != "" );

	SParticleSystem& NewPS		= m_ParticleSystems.PushBack();

	NewPS.m_Attached			= Attached;
	NewPS.m_Serialize			= Serialize;
	NewPS.m_DefinitionName		= DefinitionName;
	NewPS.m_Location			= Location;
	NewPS.m_Orientation			= Orientation;
	NewPS.m_DefinitionNameHash	= DefinitionName;

	NewPS.m_System				= new EldritchParticles;
	NewPS.m_System->InitializeFromDefinition(	DefinitionName );
	NewPS.m_System->SetLocation(				Location );
	NewPS.m_System->SetOrientation(				Orientation );
}

void WBCompEldParticles::StopParticleSystem( const HashedString& DefinitionNameHash )
{
	FOR_EACH_ARRAY_REVERSE( PSIter, m_ParticleSystems, SParticleSystem )
	{
		SParticleSystem& PS = PSIter.GetValue();
		if( PS.m_DefinitionNameHash == DefinitionNameHash )
		{
			SafeDelete( PS.m_System );
			m_ParticleSystems.FastRemove( PSIter );
		}
	}
}

/*virtual*/ void WBCompEldParticles::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	FOR_EACH_ARRAY_REVERSE( PSIter, m_ParticleSystems, SParticleSystem )
	{
		SParticleSystem& PS = PSIter.GetValue();
		DEVASSERT( PS.m_System );

		if( PS.m_System->IsFinished() )
		{
			SafeDelete( PS.m_System );
			m_ParticleSystems.FastRemove( PSIter );
		}
		else
		{
			PS.m_System->Tick( DeltaTime );
		}
	}
}

/*virtual*/ void WBCompEldParticles::Render()
{
	XTRACE_FUNCTION;

	if( m_ParticleSystems.Empty() )
	{
		return;
	}

	if( m_Hidden )
	{
		return;
	}

	if( m_CullDistanceSq > 0.0f )
	{
		const Vector	ViewLocation	= EldritchGame::GetPlayerViewLocation();

		// If any the systems are within the cull distance, do draw
		bool AllOutside = true;

		FOR_EACH_ARRAY( PSIter, m_ParticleSystems, SParticleSystem )
		{
			const SParticleSystem& PS = PSIter.GetValue();
			DEVASSERT( PS.m_System );

			const Vector	ViewOffset	= PS.m_System->GetLocation() - ViewLocation;
			const float		DistanceSq	= ViewOffset.LengthSquared();

			if( DistanceSq <= m_CullDistanceSq )
			{
				AllOutside = false;
				break;
			}
		}

		if( AllOutside )
		{
			return;
		}
	}

	FOR_EACH_ARRAY( PSIter, m_ParticleSystems, SParticleSystem )
	{
		const SParticleSystem& PS = PSIter.GetValue();
		DEVASSERT( PS.m_System );
		PS.m_System->Render();
	}
}

/*virtual*/ void WBCompEldParticles::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnMoved );
	STATIC_HASHED_STRING( OnTurned );
	STATIC_HASHED_STRING( OnWorldLoaded );
	STATIC_HASHED_STRING( PlayParticleSystem );
	STATIC_HASHED_STRING( StopParticleSystem );
	STATIC_HASHED_STRING( Hide );
	STATIC_HASHED_STRING( Show );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnMoved || EventName == sOnWorldLoaded || EventName == sOnTurned )
	{
		WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
		DEVASSERT( pTransform );

		const Vector Location = pTransform->GetLocation();
		const Angles Orientation = pTransform->GetOrientation();

		FOR_EACH_ARRAY( PSIter, m_ParticleSystems, SParticleSystem )
		{
			const SParticleSystem& PS = PSIter.GetValue();
			if( PS.m_Attached )
			{
				DEVASSERT( PS.m_System );

				const WBCompEldCamera* const pCamera = GET_WBCOMP( GetEntity(), EldCamera );
				if( pCamera )
				{
					PS.m_System->SetLocation( Location + pCamera->GetViewTranslationOffset( WBCompEldCamera::EVM_All ) );
					PS.m_System->SetOrientation( Orientation + pCamera->GetViewOrientationOffset( WBCompEldCamera::EVM_All ) );
				}
				else
				{
					PS.m_System->SetLocation( Location );
					PS.m_System->SetOrientation( Orientation );
				}
			}
		}
	}
	else if( EventName == sPlayParticleSystem )
	{
		STATIC_HASHED_STRING( ParticleSystem );
		const SimpleString ParticleSystemDef = Event.GetString( sParticleSystem );

		STATIC_HASHED_STRING( Attached );
		const bool Attached = Event.GetBool( sAttached );

		STATIC_HASHED_STRING( UseLocationOverride );
		const bool UseLocationOverride = Event.GetBool( sUseLocationOverride );

		STATIC_HASHED_STRING( LocationOverride );
		const Vector LocationOverride = Event.GetVector( sLocationOverride );

		const bool		Serialize			= true;
		const Vector*	pLocationOverride	= UseLocationOverride ? ( &LocationOverride ) : NULL;

		AddParticleSystem( ParticleSystemDef, Serialize, Attached, pLocationOverride );
	}
	else if( EventName == sStopParticleSystem )
	{
		STATIC_HASHED_STRING( ParticleSystem );
		const HashedString ParticleSystemDef = Event.GetHash( sParticleSystem );

		StopParticleSystem( ParticleSystemDef );
	}
	else if( EventName == sHide )
	{
		m_Hidden = true;
	}
	else if( EventName == sShow )
	{
		m_Hidden = false;
	}
}

#define VERSION_EMPTY	0
#define VERSION_HIDDEN	1
#define VERSION_SYSTEMS	2
#define VERSION_CURRENT	2

uint WBCompEldParticles::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;	// Version

	Size += 1;	// m_Hidden

	Size += 4;	// m_ParticleSystems.Size()
	FOR_EACH_ARRAY( ParticleSystemIter, m_ParticleSystems, SParticleSystem )
	{
		const SParticleSystem& ParticleSystem = ParticleSystemIter.GetValue();
		Size += 1;	// m_Serialize
		if( ParticleSystem.m_Serialize )
		{
			Size += 1;					// m_Attached
			Size += sizeof( Vector );	// m_Location
			Size += sizeof( Angles );	// m_Orientation
			Size += IDataStream::SizeForWriteString( ParticleSystem.m_DefinitionName );
		}
	}

	return Size;
}

void WBCompEldParticles::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteBool( m_Hidden );

	Stream.WriteUInt32( m_ParticleSystems.Size() );
	FOR_EACH_ARRAY( ParticleSystemIter, m_ParticleSystems, SParticleSystem )
	{
		const SParticleSystem& ParticleSystem = ParticleSystemIter.GetValue();
		Stream.WriteBool( ParticleSystem.m_Serialize );
		if( ParticleSystem.m_Serialize )
		{
			Stream.WriteBool( ParticleSystem.m_Attached );
			Stream.Write( sizeof( Vector ), &ParticleSystem.m_Location );
			Stream.Write( sizeof( Angles ), &ParticleSystem.m_Orientation );
			Stream.WriteString( ParticleSystem.m_DefinitionName );
		}
	}
}

void WBCompEldParticles::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_HIDDEN )
	{
		m_Hidden = Stream.ReadBool();
	}

	if( Version >= VERSION_SYSTEMS )
	{
		const uint NumParticleSystems = Stream.ReadUInt32();
		for( uint ParticleSystemIndex = 0; ParticleSystemIndex < NumParticleSystems; ++ParticleSystemIndex )
		{
			if( Stream.ReadBool() )	// Serialized
			{
				const bool Attached = Stream.ReadBool();

				Vector Location;
				Stream.Read( sizeof( Vector ), &Location );

				Angles Orientation;
				Stream.Read( sizeof( Angles ), &Orientation );

				const SimpleString DefinitionName = Stream.ReadString();

				const bool Serialize = true;

				AddParticleSystem( DefinitionName, Serialize, Attached, Location, Orientation );
			}
		}
	}
}