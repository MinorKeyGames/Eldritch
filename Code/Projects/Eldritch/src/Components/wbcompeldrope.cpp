#include "core.h"
#include "wbcompeldrope.h"
#include "wbcompeldtransform.h"
#include "wbcompeldcollision.h"
#include "wbcompeldmesh.h"
#include "wbcompeldanchor.h"
#include "ray.h"
#include "collisioninfo.h"
#include "eldritchworld.h"
#include "configmanager.h"
#include "idatastream.h"
#include "wbeventmanager.h"
#include "mathcore.h"

WBCompEldRope::WBCompEldRope()
:	m_CollisionFatten( 0.0f )
,	m_MeshFatten( 0.0f )
,	m_EndpointSpacing( 0.0f )
,	m_AnchorDepth( 0.0f )
,	m_HookLength( 0.0f )
,	m_DangleHeight( 0.0f )
,	m_HookEntity( "" )
,	m_Anchor()
,	m_Dropped( false )
{
	STATIC_HASHED_STRING( OnWorldChanged );
	GetEventManager()->AddObserver( sOnWorldChanged, this );

	STATIC_HASHED_STRING( OnStaticCollisionChanged );
	GetEventManager()->AddObserver( sOnStaticCollisionChanged, this );
}

WBCompEldRope::~WBCompEldRope()
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

/*virtual*/ void WBCompEldRope::InitializeFromDefinition( const SimpleString& DefinitionName )
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

	STATICHASH( HookLength );
	m_HookLength = ConfigManager::GetInheritedFloat( sHookLength, 0.0f, sDefinitionName );

	STATICHASH( DangleHeight );
	m_DangleHeight = ConfigManager::GetInheritedFloat( sDangleHeight, 0.0f, sDefinitionName );

	ASSERT( m_DangleHeight < m_HookLength );

	STATICHASH( HookEntity );
	m_HookEntity = ConfigManager::GetInheritedString( sHookEntity, "", sDefinitionName );
	ASSERT( m_HookEntity != "" );
}

/*virtual*/ void WBCompEldRope::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnWorldChanged );
	STATIC_HASHED_STRING( OnStaticCollisionChanged );
	STATIC_HASHED_STRING( OnInitialOrientationSet );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnWorldChanged || EventName == sOnStaticCollisionChanged )
	{
		if( m_Dropped )
		{
			// If anchor is removed, destroy the rope (and drop anyone climbing on it!)

			CollisionInfo Info;
			Info.m_CollideWorld		= true;
			Info.m_CollideEntities	= true;
			Info.m_CollidingEntity	= GetEntity();
			Info.m_UserFlags		= EECF_CollideAsEntity | EECF_CollideStaticEntities;

			EldritchWorld* const pWorld = GetWorld();
			if( !pWorld->PointCheck( m_Anchor, Info ) )
			{
				GetEntity()->Destroy();
			}
		}
	}
	else if( EventName == sOnInitialOrientationSet )
	{
		DropRope();
	}
}

void WBCompEldRope::DropRope()
{
	WBEntity* const				pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	// HookVector is directed away from the surface (same as hit normal)
	const Vector				HookVector	= Quantize( pTransform->GetOrientation().ToVector() );
	const Vector				DropVector	= Vector( 0.0f, 0.0f, -1.0f );

	ASSERT( HookVector.LengthSquared() == 1.0f );
	ASSERT(	( HookVector.y == 0.0f && HookVector.z == 0.0f ) ||
			( HookVector.x == 0.0f && HookVector.z == 0.0f ) ||
			( HookVector.x == 0.0f && HookVector.y == 0.0f ) );

	WBCompEldCollision* const	pCollision	= GET_WBCOMP( pEntity, EldCollision );
	DEVASSERT( pCollision );

	WBCompEldMesh* const		pMesh		= GET_WBCOMP( pEntity, EldMesh );
	DEVASSERT( pMesh );

	EldritchWorld* const		pWorld		= GetWorld();

	const Vector	HitLocation		= pTransform->GetLocation();
	const Vector	StartLocation	= HitLocation + ( HookVector * m_HookLength );
	const Ray		TraceRay		= Ray( StartLocation, DropVector );

	CollisionInfo Info;
	Info.m_CollideWorld		= true;
	Info.m_CollideEntities	= true;
	Info.m_CollidingEntity	= pEntity;
	Info.m_UserFlags		= EECF_CollideAsEntity | EECF_CollideStaticEntities;

	if( pWorld->Trace( TraceRay, Info ) )
	{
		m_Dropped = true;

		// Reset orientation now that it is being "baked" into extents.
		pTransform->SetOrientation( Angles() );

		// StartLocation is where the rope attaches to the hook
		// EndLocation is where the rope dangles above the ground
		ASSERT( Info.m_Plane.m_Normal.Equals( Vector::Up, EPSILON ) );
		const Vector EndLocation = Info.m_Intersection + Info.m_Plane.m_Normal * m_DangleHeight;
		pTransform->SetLocation( 0.5f * ( StartLocation + EndLocation ) );

		const Vector HalfExtents = ( 0.5f * ( EndLocation - StartLocation ) ).GetAbs();
		const Vector CollisionFatten = Vector( m_CollisionFatten, m_CollisionFatten, m_CollisionFatten );
		pCollision->SetExtents( HalfExtents + CollisionFatten );

		const Vector MeshFatten = Vector( m_MeshFatten, m_MeshFatten, m_MeshFatten );
		pMesh->SetMeshScale( HalfExtents + MeshFatten );

		m_Anchor = HitLocation - ( HookVector * m_AnchorDepth );

		// Spawn rope hook entity and set up its transform and anchor
		{
			WBEntity* const				pHookEntity		= WBWorld::GetInstance()->CreateEntity( m_HookEntity );
			ASSERT( pHookEntity );

			WBCompEldTransform* const	pHookTransform	= pHookEntity->GetTransformComponent<WBCompEldTransform>();
			ASSERT( pHookTransform );

			WBCompEldAnchor* const		pHookAnchor		= GET_WBCOMP( pHookEntity, EldAnchor );
			ASSERT( pHookAnchor );

			pHookTransform->SetLocation( StartLocation );
			pHookTransform->SetOrientation( ( -HookVector ).ToAngles() );
		}
	}
	else
	{
		WARNDESC( "Rope could not be launched." );
		pEntity->Destroy();
	}
}

Vector WBCompEldRope::Quantize( const Vector& V ) const
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

#define VERSION_EMPTY	0
#define VERSION_ANCHOR	1
#define VERSION_DROPPED	2
#define VERSION_CURRENT	2

uint WBCompEldRope::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += sizeof( Vector );	// m_Anchor
	Size += 1;					// m_Dropped

	return Size;
}

void WBCompEldRope::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.Write( sizeof( Vector ), &m_Anchor );

	Stream.WriteBool( m_Dropped );
}

void WBCompEldRope::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_ANCHOR )
	{
		Stream.Read( sizeof( Vector ), &m_Anchor );
	}

	if( Version >= VERSION_DROPPED )
	{
		m_Dropped = Stream.ReadBool();
	}
}