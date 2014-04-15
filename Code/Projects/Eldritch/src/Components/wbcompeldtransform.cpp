#include "core.h"
#include "wbcompeldtransform.h"
#include "configmanager.h"
#include "mathcore.h"
#include "idatastream.h"
#include "wbeventmanager.h"
#include "eldritchworld.h"
#include "wbcompeldplayer.h"
#include "wbcompeldcollision.h"
#include "wbcompeldcamera.h"
#include "Components/wbcompstatmod.h"
#include "Components/wbcompowner.h"
#include "quat.h"
#include "irenderer.h"
#include "eldritchframework.h"
#include "matrix.h"

WBCompEldTransform::WBCompEldTransform()
:	m_Location()
,	m_Velocity()
,	m_Acceleration()
,	m_Gravity( 0.0f )
,	m_UseSpeedLimit( false )
,	m_SpeedLimit( 0.0f )
,	m_AllowImpulses( false )
,	m_Orientation()
,	m_RotationalVelocity()
,	m_CanMove( false )
,	m_IsAttachedToOwner( false )
,	m_OwnerOffset()
,	m_IsSettled( false )
{
}

WBCompEldTransform::~WBCompEldTransform()
{
}

/*virtual*/ void WBCompEldTransform::Initialize()
{
	Super::Initialize();

	WBEntity* const pEntity = GetEntity();
	DEVASSERT( pEntity );
	pEntity->SetTransformComponent( this );
}

/*virtual*/ void WBCompEldTransform::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( EldTransform );

	STATICHASH( CanMove );
	m_CanMove = ConfigManager::GetInheritedBool( sCanMove, true, sDefinitionName );

	STATICHASH( UseGravity );
	const bool UseGravity = ConfigManager::GetInheritedBool( sUseGravity, false, sDefinitionName );

	STATICHASH( Gravity );
	const float DefaultGravity = ConfigManager::GetFloat( sGravity, 0.0f, sEldTransform );
	m_Gravity = -ConfigManager::GetInheritedFloat( sGravity, UseGravity ? DefaultGravity : 0.0f, sDefinitionName );

	STATICHASH( SpeedLimit );
	m_SpeedLimit = ConfigManager::GetInheritedFloat( sSpeedLimit, 0.0f, sDefinitionName );
	m_UseSpeedLimit = ( m_SpeedLimit > 0.0f );

	STATICHASH( AllowImpulses );
	m_AllowImpulses = ConfigManager::GetInheritedBool( sAllowImpulses, true, sDefinitionName );

	STATICHASH( Yaw );
	m_Orientation.Yaw = DEGREES_TO_RADIANS( ConfigManager::GetInheritedFloat( sYaw, 0.0f, sDefinitionName ) );

	STATICHASH( IsAttachedToOwner );
	m_IsAttachedToOwner = ConfigManager::GetInheritedBool( sIsAttachedToOwner, false, sDefinitionName );

	STATICHASH( OwnerOffsetX );
	m_OwnerOffset.x = ConfigManager::GetInheritedFloat( sOwnerOffsetX, 0.0f, sDefinitionName );

	STATICHASH( OwnerOffsetY );
	m_OwnerOffset.y = ConfigManager::GetInheritedFloat( sOwnerOffsetY, 0.0f, sDefinitionName );

	STATICHASH( OwnerOffsetZ );
	m_OwnerOffset.z = ConfigManager::GetInheritedFloat( sOwnerOffsetZ, 0.0f, sDefinitionName );
}

void WBCompEldTransform::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	if( m_IsAttachedToOwner )
	{
		MoveWithOwner();
	}
	else if( m_CanMove )
	{
		TickMotion( DeltaTime );
	}
}

void WBCompEldTransform::TickAcceleration( float DeltaTime )
{
	XTRACE_FUNCTION;

	DEVASSERT( m_CanMove );

	if( m_UseSpeedLimit && m_Acceleration.LengthSquared() > EPSILON )
	{
		WBCompStatMod* const pStatMod = GET_WBCOMP( GetEntity(), StatMod );
		WB_MODIFY_FLOAT_SAFE( SpeedLimit, m_SpeedLimit, pStatMod );

		Vector	ComponentDirection;
		float	AccelerationLength;
		float	AccelerationLengthOverOne;
		m_Acceleration.GetNormalized( ComponentDirection, AccelerationLength, AccelerationLengthOverOne );

		const Vector	ComponentVelocity	= m_Velocity.ProjectionOnto( ComponentDirection );
		const Vector	ComplementVelocity	= m_Velocity - ComponentVelocity;
		const float		ComponentSpeed		= ComponentVelocity.Length();
		const float		SignedSpeed			= ( ComponentVelocity.Dot( ComponentDirection ) > 0.0f ) ? ComponentSpeed : -ComponentSpeed;

		// Allow component velocity to exceed the limit, but *not* by adding to it with acceleration.
		if( SignedSpeed < WB_MODDED( SpeedLimit ) )
		{
			const float NewComponentSpeed = Min( SignedSpeed + AccelerationLength * DeltaTime, WB_MODDED( SpeedLimit ) );
			m_Velocity = ComplementVelocity + ComponentDirection * NewComponentSpeed;
		}
	}
	else
	{
		m_Velocity += m_Acceleration * DeltaTime;
	}

	m_Velocity.z += m_Gravity * DeltaTime;
}

void WBCompEldTransform::TickMotion( float DeltaTime )
{
	XTRACE_FUNCTION;

	DEVASSERT( m_CanMove );

	TickAcceleration( DeltaTime );

	if( m_Velocity.LengthSquared() > EPSILON )	// Optimization, don't want to pay MoveBy costs for static entities
	{
		MoveBy( m_Velocity * DeltaTime );
	}

	if( !m_RotationalVelocity.IsZero() )
	{
		Angles NewOrientation =  m_Orientation + m_RotationalVelocity * DeltaTime;
		NewOrientation.Pitch = Clamp( NewOrientation.Pitch, -PI * 0.49f, PI * 0.49f );
		SetOrientation( NewOrientation );
	}
}

void WBCompEldTransform::SetLocation( const Vector& NewLocation )
{
	XTRACE_FUNCTION;

	if( NewLocation.Equals( m_Location, 0.0f ) )
	{
		// Do nothing
	}
	else
	{
		m_Location = NewLocation;

		WB_MAKE_EVENT( OnMoved, GetEntity() );
		WB_DISPATCH_EVENT( GetEventManager(), OnMoved, GetEntity() );
	}
}

void WBCompEldTransform::SetOrientation( const Angles& NewOrientation )
{
	XTRACE_FUNCTION;

	m_Orientation = NewOrientation;

	WB_MAKE_EVENT( OnTurned, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), OnTurned, GetEntity() );
}

void WBCompEldTransform::MoveBy( const Vector& Offset )
{
	XTRACE_FUNCTION;

	DEVASSERT( m_CanMove );

	WBCompEldCollision* const pCollision = GET_WBCOMP( GetEntity(), EldCollision );

	if( pCollision )
	{
		Vector ModifiedOffset = Offset;
		pCollision->Collide( m_Location, ModifiedOffset );
		SetLocation( m_Location + ModifiedOffset );

		if( ModifiedOffset.LengthSquared() < EPSILON )
		{
			if( !m_IsSettled )
			{
				m_IsSettled = true;
				WB_MAKE_EVENT( OnSettled, GetEntity() );
				WB_DISPATCH_EVENT( GetEventManager(), OnSettled, GetEntity() );
			}
		}
		else
		{
			m_IsSettled = false;
		}
	}
	else
	{
		SetLocation( m_Location + Offset );
	}
}

void WBCompEldTransform::MoveWithOwner()
{
	XTRACE_FUNCTION;

	DEVASSERT( m_IsAttachedToOwner );

	WBEntity* const pEntity = GetEntity();
	DEVASSERT( pEntity );

	WBCompOwner* const pOwner = GET_WBCOMP( pEntity, Owner );
	DEVASSERT( pOwner );

	WBEntity* const pOwnerEntity = pOwner->GetOwner();
	if( pOwnerEntity )
	{
		WBCompEldTransform* const pOwnerTransform = pOwnerEntity->GetTransformComponent<WBCompEldTransform>();
		DEVASSERT( pOwnerTransform );

		// TEMPHACK: What I'll probably want is a flag that says whether this is attached to the camera or to a bone on the owner's mesh.
		// If it's attached to a camera, I'll set the foreground draw flag on it.

		WBCompEldCamera* const pOwnerCamera = GET_WBCOMP( pOwnerEntity, EldCamera );
		DEVASSERT( pOwnerCamera );

		const Angles Orientation = pOwnerTransform->GetOrientation() + pOwnerCamera->GetViewOrientationOffset( WBCompEldCamera::EVM_All );
		const Vector TransformedOffset = m_OwnerOffset * Orientation.ToMatrix();
		const Vector Location = pOwnerTransform->GetLocation() + pOwnerCamera->GetViewTranslationOffset( WBCompEldCamera::EVM_All ) + TransformedOffset;

		SetLocation( Location );
		SetOrientation( Orientation );
	}
}

/*virtual*/ void WBCompEldTransform::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( ApplyImpulse );
	STATIC_HASHED_STRING( SetDefaultGravity );
	STATIC_HASHED_STRING( SetAcceleration );
	STATIC_HASHED_STRING( SetCanMove );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sApplyImpulse )
	{
		if( m_AllowImpulses )
		{
			STATIC_HASHED_STRING( Impulse );
			const Vector Impulse = Event.GetVector( sImpulse );
			ApplyImpulse( Impulse );

			m_IsSettled = false;
		}
	}
	else if( EventName == sSetDefaultGravity )
	{
		STATICHASH( EldTransform );
		STATICHASH( Gravity );
		const float DefaultGravity = ConfigManager::GetFloat( sGravity, 0.0f, sEldTransform );

		m_Gravity = -DefaultGravity;
	}
	else if( EventName == sSetAcceleration )
	{
		STATIC_HASHED_STRING( Acceleration );
		const Vector Acceleration = Event.GetVector( sAcceleration );

		SetAcceleration( Acceleration );
	}
	else if( EventName == sSetCanMove )
	{
		STATICHASH( CanMove );
		const bool CanMove = Event.GetBool( sCanMove );

		SetCanMove( CanMove );
	}
}

/*virtual*/ void WBCompEldTransform::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Vector, Location, m_Location );
	WB_SET_CONTEXT( Event, Angles, Orientation, m_Orientation );
	WB_SET_CONTEXT( Event, Vector, Velocity, m_Velocity );
	WB_SET_CONTEXT( Event, Float, SpeedSq, m_Velocity.LengthSquared() );
}

#if BUILD_DEV
/*virtual*/ void WBCompEldTransform::DebugRender() const
{
	GetFramework()->GetRenderer()->DEBUGDrawCross( m_Location, 0.25f, ARGB_TO_COLOR( 255, 255, 255, 255 ) );
}
#endif

#define VERSION_EMPTY			0
#define VERSION_LOCATION		1
#define VERSION_VELOCITY		2
#define VERSION_ORIENTATION		3
#define VERSION_ACCELERATION	4
#define VERSION_ISSETTLED		5
#define VERSION_GRAVITY			6
#define VERSION_CANMOVE			7
#define VERSION_CURRENT			7

uint WBCompEldTransform::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += sizeof( Vector );	// m_Location
	Size += sizeof( Vector );	// m_Velocity
	Size += sizeof( Vector );	// m_Acceleration
	Size += sizeof( Angles );	// m_Orientation
	Size += 1;					// m_IsSettled
	Size += 4;					// m_Gravity
	Size += 1;					// m_CanMove

	return Size;
}

void WBCompEldTransform::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.Write( sizeof( Vector ), &m_Location );
	Stream.Write( sizeof( Vector ), &m_Velocity );
	Stream.Write( sizeof( Vector ), &m_Acceleration );
	Stream.Write( sizeof( Angles ), &m_Orientation );
	Stream.WriteBool( m_IsSettled );
	Stream.WriteFloat( m_Gravity );
	Stream.WriteBool( m_CanMove );
}

void WBCompEldTransform::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_LOCATION )
	{
		Stream.Read( sizeof( Vector ), &m_Location );
	}

	if( Version >= VERSION_VELOCITY )
	{
		Stream.Read( sizeof( Vector ), &m_Velocity );
	}

	if( Version >= VERSION_ACCELERATION )
	{
		Stream.Read( sizeof( Vector ), &m_Acceleration );
	}

	if( Version >= VERSION_ORIENTATION )
	{
		Stream.Read( sizeof( Vector ), &m_Orientation );
	}

	if( Version >= VERSION_ISSETTLED )
	{
		m_IsSettled = Stream.ReadBool();
	}

	if( Version >= VERSION_GRAVITY )
	{
		m_Gravity = Stream.ReadFloat();
	}

	if( Version >= VERSION_CANMOVE )
	{
		m_CanMove = Stream.ReadBool();
	}
}

void WBCompEldTransform::Report() const
{
	Super::Report();

	PRINTF( WBPROPERTY_REPORT_PREFIX "Location: %s\n", m_Location.GetString().CStr() );
	PRINTF( WBPROPERTY_REPORT_PREFIX "Orientation: %s\n", m_Orientation.GetString().CStr() );
	if( !m_Velocity.IsZero() ) { PRINTF( WBPROPERTY_REPORT_PREFIX "Velocity: %s\n", m_Velocity.GetString().CStr() ); }
	if( !m_Acceleration.IsZero() ) { PRINTF( WBPROPERTY_REPORT_PREFIX "Acceleration: %s\n", m_Acceleration.GetString().CStr() ); }
}