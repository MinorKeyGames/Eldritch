#include "core.h"
#include "wbcompeldtrapbolt.h"
#include "wbcompeldtransform.h"
#include "wbcompeldcollision.h"
#include "wbcompeldmesh.h"
#include "ray.h"
#include "collisioninfo.h"
#include "eldritchworld.h"
#include "configmanager.h"
#include "idatastream.h"
#include "wbeventmanager.h"
#include "mathcore.h"

WBCompEldTrapBolt::WBCompEldTrapBolt()
:	m_CollisionFatten( 0.0f )
,	m_MeshFatten( 0.0f )
,	m_EndpointSpacing( 0.0f )
,	m_AnchorDepth( 0.0f )
,	m_Start()
,	m_End()
,	m_AnchorStart()
,	m_AnchorEnd()
,	m_Launched( false )
,	m_Triggered( false )
{
	STATIC_HASHED_STRING( OnWorldChanged );
	GetEventManager()->AddObserver( sOnWorldChanged, this );

	STATIC_HASHED_STRING( OnStaticCollisionChanged );
	GetEventManager()->AddObserver( sOnStaticCollisionChanged, this );
}

WBCompEldTrapBolt::~WBCompEldTrapBolt()
{
	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( OnWorldChanged );
		pEventManager->RemoveObserver( sOnWorldChanged, this );

		STATIC_HASHED_STRING( OnStaticCollisionChanged );
		pEventManager->RemoveObserver( sOnStaticCollisionChanged, this );
	}
}

/*virtual*/ void WBCompEldTrapBolt::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( CollisionFatten );
	m_CollisionFatten = ConfigManager::GetInheritedFloat( sCollisionFatten, 0.0f, sDefinitionName );

	STATICHASH( MeshFatten );
	m_MeshFatten = ConfigManager::GetInheritedFloat( sMeshFatten, 0.0f, sDefinitionName );

	STATICHASH( EndpointSpacing );
	m_EndpointSpacing = ConfigManager::GetInheritedFloat( sEndpointSpacing, 0.0f, sDefinitionName );

	STATICHASH( AnchorDepth );
	m_AnchorDepth = ConfigManager::GetInheritedFloat( sAnchorDepth, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldTrapBolt::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnWorldChanged );
	STATIC_HASHED_STRING( OnStaticCollisionChanged );
	STATIC_HASHED_STRING( OnTouched );
	STATIC_HASHED_STRING( OnInitialOrientationSet );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnWorldChanged || EventName == sOnStaticCollisionChanged )
	{
		if( m_Launched )
		{
			// Check anchors and if either of them is gone, trigger the bolt
			// Also trigger if anything occludes the line

			CollisionInfo Info;
			Info.m_CollideWorld = true;
			Info.m_CollideEntities = true;
			Info.m_CollidingEntity = GetEntity();
			Info.m_UserFlags = EECF_CollideAsEntity | EECF_CollideStaticEntities;

			EldritchWorld* const pWorld = GetWorld();
			if( !pWorld->PointCheck( m_AnchorStart, Info ) ||
				!pWorld->PointCheck( m_AnchorEnd, Info ) ||
				pWorld->LineCheck( m_Start, m_End, Info ) )
			{
				TriggerTrapBolt();
			}
		}
	}
	else if( EventName == sOnTouched )
	{
		if( m_Launched )
		{
			TriggerTrapBolt();
		}
	}
	else if( EventName == sOnInitialOrientationSet )
	{
		LaunchTrapBolt();
	}
}

/*virtual*/ void WBCompEldTrapBolt::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Vector, Endpoint1, m_Start );
	WB_SET_CONTEXT( Event, Vector, Endpoint2, m_End );
}

Vector WBCompEldTrapBolt::Quantize( const Vector& V ) const
{
	Vector RetVal;

	for( uint i = 0; i < 3; ++i )
	{
		if( Abs( V.v[i] ) < EPSILON )
		{
			RetVal.v[i] = 0.0f;
		}
		else
		{
			RetVal.v[i] = Sign( V.v[i] );
		}
	}

	return RetVal;
}

void WBCompEldTrapBolt::LaunchTrapBolt()
{
	WBEntity* const				pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector				TrapVector	= Quantize( pTransform->GetOrientation().ToVector() );

	// Since we're using AABBs for collision, we can only support trap bolts on primary axes.
	ASSERT( TrapVector.LengthSquared() == 1.0f );
	ASSERT(	( TrapVector.y == 0.0f && TrapVector.z == 0.0f ) ||
			( TrapVector.x == 0.0f && TrapVector.z == 0.0f ) ||
			( TrapVector.x == 0.0f && TrapVector.y == 0.0f ) );

	WBCompEldCollision* const	pCollision	= GET_WBCOMP( pEntity, EldCollision );
	ASSERT( pCollision );

	WBCompEldMesh* const		pMesh		= GET_WBCOMP( pEntity, EldMesh );
	ASSERT( pMesh );

	EldritchWorld* const pWorld = GetWorld();

	const Vector	StartLocation	= pTransform->GetLocation();
	const Ray		ForwardRay		= Ray( StartLocation, TrapVector );
	const Ray		BackRay			= Ray( StartLocation, -TrapVector );

	CollisionInfo ForwardInfo;
	ForwardInfo.m_CollideWorld		= true;
	ForwardInfo.m_CollideEntities	= true;
	ForwardInfo.m_CollidingEntity	= pEntity;
	ForwardInfo.m_UserFlags			= EECF_CollideAsEntity | EECF_CollideStaticEntities;

	CollisionInfo BackInfo;
	BackInfo.m_CollideWorld		= true;
	BackInfo.m_CollideEntities	= true;
	BackInfo.m_CollidingEntity	= pEntity;
	BackInfo.m_UserFlags		= EECF_CollideAsEntity | EECF_CollideStaticEntities;

	if( pWorld->Trace( ForwardRay, ForwardInfo ) &&
		pWorld->Trace( BackRay, BackInfo ) )
	{
		// Set launched before setting extents, so anything we touch when extents are set will immediately trigger this.
		m_Launched = true;

		// Reset orientation now that it is being "baked" into extents.
		pTransform->SetOrientation( Angles() );

		const Vector ForwardEndpoint	= ForwardInfo.m_Intersection	+ ForwardInfo.m_Plane.m_Normal	* m_EndpointSpacing;
		const Vector BackEndpoint		= BackInfo.m_Intersection		+ BackInfo.m_Plane.m_Normal		* m_EndpointSpacing;
		pTransform->SetLocation( 0.5f * ( ForwardEndpoint + BackEndpoint ) );

		const Vector HalfExtents = ( 0.5f * ( ForwardEndpoint - BackEndpoint ) ).GetAbs();
		const Vector CollisionFatten = Vector( m_CollisionFatten, m_CollisionFatten, m_CollisionFatten );
		pCollision->SetExtents( HalfExtents + CollisionFatten );

		const Vector MeshFatten = Vector( m_MeshFatten, m_MeshFatten, m_MeshFatten );
		pMesh->SetMeshScale( HalfExtents + MeshFatten );

		m_Start	= BackEndpoint;
		m_End	= ForwardEndpoint;

		m_AnchorStart	= BackEndpoint		- TrapVector	* m_AnchorDepth;
		m_AnchorEnd		= ForwardEndpoint	+ TrapVector	* m_AnchorDepth;
	}
	else
	{
		WARNDESC( "Trap bolt could not be launched." );
		pEntity->Destroy();
	}
}

void WBCompEldTrapBolt::TriggerTrapBolt()
{
	ASSERT( m_Launched );

	if( m_Triggered )
	{
		return;
	}

	m_Triggered = true;

	WBEntity* const pEntity = GetEntity();
	WBCompEldCollision* const pCollision = GET_WBCOMP( pEntity, EldCollision );
	ASSERT( pCollision );

	// Send event to touching entities
	// I could *probably* just do this all with touch events now...
	Array<WBEntity*> TouchingEntities;
	pCollision->GetTouchingEntities( TouchingEntities );

	const uint NumTouchingEntities = TouchingEntities.Size();
	for( uint TouchingEntityIndex = 0; TouchingEntityIndex < NumTouchingEntities; ++TouchingEntityIndex )
	{
		WBEntity* pTouchingEntity = TouchingEntities[ TouchingEntityIndex ];

		WB_MAKE_EVENT( OnTriggeredTrapBolt, pEntity );
		WB_SET_AUTO( OnTriggeredTrapBolt, Entity, Triggered, pTouchingEntity );
		WB_DISPATCH_EVENT( GetEventManager(), OnTriggeredTrapBolt, pEntity );
	}

	// Send a singular event to self for destruction, etc.
	WB_MAKE_EVENT( OnTriggeredTrapBoltFinished, pEntity );
	WB_DISPATCH_EVENT( GetEventManager(), OnTriggeredTrapBoltFinished, pEntity );
}

#define VERSION_EMPTY		0
#define VERSION_ANCHORS		1
#define VERSION_ENDPOINTS	2
#define VERSION_TRIGGERED	3
#define VERSION_LAUNCHED	4
#define VERSION_CURRENT		4

uint WBCompEldTrapBolt::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;						// Version
	Size += sizeof( Vector ) * 2;	// m_AnchorStart/End
	Size += sizeof( Vector ) * 2;	// m_Start/End
	Size += 1;						// m_Triggered
	Size += 1;						// m_Launched

	return Size;
}

void WBCompEldTrapBolt::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.Write( sizeof( Vector ), &m_AnchorStart );
	Stream.Write( sizeof( Vector ), &m_AnchorEnd );
	Stream.Write( sizeof( Vector ), &m_Start );
	Stream.Write( sizeof( Vector ), &m_End );

	Stream.WriteBool( m_Triggered );

	Stream.WriteBool( m_Launched );
}

void WBCompEldTrapBolt::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_ANCHORS )
	{
		Stream.Read( sizeof( Vector ), &m_AnchorStart );
		Stream.Read( sizeof( Vector ), &m_AnchorEnd );
	}

	if( Version >= VERSION_ENDPOINTS )
	{
		Stream.Read( sizeof( Vector ), &m_Start );
		Stream.Read( sizeof( Vector ), &m_End );
	}

	if( Version >= VERSION_TRIGGERED )
	{
		m_Triggered = Stream.ReadBool();
	}

	if( Version >= VERSION_LAUNCHED )
	{
		m_Launched = Stream.ReadBool();
	}
}