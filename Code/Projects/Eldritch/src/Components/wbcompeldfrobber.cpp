#include "core.h"
#include "wbcompeldfrobber.h"
#include "wbeventmanager.h"
#include "wbcompeldtransform.h"
#include "wbcompeldcollision.h"
#include "wbcompeldfrobbable.h"
#include "wbcompeldcamera.h"
#include "wbcomponentarrays.h"
#include "configmanager.h"
#include "segment.h"
#include "aabb.h"
#include "collisioninfo.h"
#include "eldritchworld.h"
#include "eldritchframework.h"
#include "inputsystem.h"
#include "idatastream.h"

WBCompEldFrobber::WBCompEldFrobber()
:	m_FrobDistance( 0.0f )
,	m_FrobTarget()
,	m_FrobDisabled( false )
{
}

WBCompEldFrobber::~WBCompEldFrobber()
{
}

/*virtual*/ void WBCompEldFrobber::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( FrobDistance );
	m_FrobDistance = ConfigManager::GetInheritedFloat( sFrobDistance, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldFrobber::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	Unused( DeltaTime );

	WBEntity* const pOldTarget = m_FrobTarget.Get();
	WBEntity* const pNewTarget = FindTargetFrobbable();

	if( pOldTarget && pOldTarget != pNewTarget )
	{
		OnUnsetFrobTarget( pOldTarget );
	}

	if( pNewTarget && pNewTarget != pOldTarget )
	{
		OnSetFrobTarget( pNewTarget );
	}

	m_FrobTarget = pNewTarget;
}

void WBCompEldFrobber::OnSetFrobTarget( WBEntity* const pFrobTarget )
{
	WBCompEldFrobbable* const pFrobbable = GET_WBCOMP( pFrobTarget, EldFrobbable );
	ASSERT( pFrobbable );
	pFrobbable->SetIsFrobTarget( true, GetEntity() );
}

void WBCompEldFrobber::OnUnsetFrobTarget( WBEntity* const pFrobTarget )
{
	WBCompEldFrobbable* const pFrobbable = GET_WBCOMP( pFrobTarget, EldFrobbable );
	ASSERT( pFrobbable );
	pFrobbable->SetIsFrobTarget( false, GetEntity() );
}

/*virtual*/ void WBCompEldFrobber::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnFrob );
	STATIC_HASHED_STRING( EnableFrob );
	STATIC_HASHED_STRING( DisableFrob );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnFrob )
	{
		STATIC_HASHED_STRING( InputEdge );
		const int InputEdge = Event.GetInt( sInputEdge );

		TryFrob( InputEdge );
	}
	else if( EventName == sEnableFrob )
	{
		m_FrobDisabled = false;
	}
	else if( EventName == sDisableFrob )
	{
		m_FrobDisabled = true;
	}
}

/*virtual*/ void WBCompEldFrobber::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Entity, FrobTarget, m_FrobTarget );
}

void WBCompEldFrobber::TryFrob( const int InputEdge )
{
	WBEntity* const pTargetFrobbable = m_FrobTarget.Get();
	DEBUGASSERT( pTargetFrobbable == FindTargetFrobbable() );

	if( pTargetFrobbable )
	{
		WB_MAKE_EVENT( MarshalFrob, pTargetFrobbable );
		WB_SET_AUTO( MarshalFrob, Entity, Frobber, GetEntity() );
		WB_SET_AUTO( MarshalFrob, Int, InputEdge, InputEdge );
		WB_DISPATCH_EVENT( GetEventManager(), MarshalFrob, pTargetFrobbable );

		// Untarget the frob target so we'll refresh the frob overlay on the next tick.
		OnUnsetFrobTarget( pTargetFrobbable );
		m_FrobTarget = NULL;
	}
}

WBEntity* WBCompEldFrobber::FindTargetFrobbable() const
{
	if( m_FrobDisabled )
	{
		return NULL;
	}

	InputSystem* const pInputSystem = GetFramework()->GetInputSystem();
	STATIC_HASHED_STRING( Frob );
	if( pInputSystem->IsSuppressed( sFrob ) )
	{
		return NULL;
	}

	const Array<WBCompEldFrobbable*>* const pFrobbablesArrays = WBComponentArrays::GetComponents<WBCompEldFrobbable>();
	if( !pFrobbablesArrays )
	{
		return NULL;
	}

	WBEntity* const				pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldCamera* const		pCamera		= GET_WBCOMP( pEntity, EldCamera );

	const Vector StartOffset		= pCamera ? pCamera->GetViewTranslationOffset( WBCompEldCamera::EVM_All ) : Vector();
	const Vector StartLocation		= pTransform->GetLocation() + StartOffset;
	const Vector Direction			= pTransform->GetOrientation().ToVector();
	const Vector EndLocation		= StartLocation + Direction * m_FrobDistance;
	const Segment FrobTraceSegment	= Segment( StartLocation, EndLocation );

	// First, trace against world and collidables as a baseline for our trace distance.
	CollisionInfo CollidableInfo;
	CollidableInfo.m_CollideWorld = true;
	CollidableInfo.m_CollideEntities = true;
	CollidableInfo.m_UserFlags = EECF_EntityCollision;

	GetWorld()->Trace( FrobTraceSegment, CollidableInfo );
	const float CollidableT = CollidableInfo.m_HitT;

	const Array<WBCompEldFrobbable*>& Frobbables = *pFrobbablesArrays;

	WBEntity*	pNearestEntity	= NULL;
	float		NearestT		= 0.0f;

	const uint NumFrobbables = Frobbables.Size();
	for( uint FrobbableIndex = 0; FrobbableIndex < NumFrobbables; ++FrobbableIndex )
	{
		WBCompEldFrobbable* const pFrobbable = Frobbables[ FrobbableIndex ];
		ASSERT( pFrobbable );

		if( !pFrobbable->IsFrobbable() )
		{
			continue;
		}

		WBEntity* const pFrobbableEntity = pFrobbable->GetEntity();
		ASSERT( pFrobbableEntity );

		if( pFrobbableEntity->IsDestroyed() )
		{
			continue;
		}

		const AABB FrobbableBox			= pFrobbable->GetBound();

		CollisionInfo Info;
		if( FrobTraceSegment.Intersects( FrobbableBox, &Info ) )
		{
			if( Info.m_HitT <= CollidableT ) // Check if trace was blocked by some collision
			{
				if( Info.m_HitT < NearestT || pNearestEntity == NULL )
				{
					pNearestEntity	= pFrobbableEntity;
					NearestT		= Info.m_HitT;
				}
			}
		}
	}

	return pNearestEntity;
}

#define VERSION_EMPTY			0
#define VERSION_FROBDISABLED	1
#define VERSION_CURRENT			1

uint WBCompEldFrobber::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += 1;					// m_FrobDisabled

	return Size;
}

void WBCompEldFrobber::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteBool( m_FrobDisabled );
}

void WBCompEldFrobber::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_FROBDISABLED )
	{
		m_FrobDisabled = Stream.ReadBool();
	}
}