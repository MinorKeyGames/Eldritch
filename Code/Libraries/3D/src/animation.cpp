#include "core.h"
#include "animation.h"
#include "animevent.h"
#include "configmanager.h"
#include "animeventfactory.h"
#include "mathcore.h"

// For the Euler integration hack in GetDisplacement
//#include "matrix.h"

Animation::Animation()
:	m_HashedName( HashedString::NullString )
,	m_StartFrame( 0 )
,	m_Length( 0 )
,	m_AnimEvents()
,	m_Velocity()
,	m_RotationalVelocity()
{
	m_Name[0] = '\0';
}

Animation::~Animation()
{
	for( uint i = 0; i < m_AnimEvents.Size(); ++i )
	{
		SafeDelete( m_AnimEvents[i] );
	}
	m_AnimEvents.Clear();
}

// Initialize anim events and other non-baked properties from config.
// (I don't want to make a habit of this, but since AnimEvents need
// to be created at runtime, might as well do the rest here and keep
// it consolidated.)
void Animation::InitializeFromDefinition( const SimpleString& QualifiedAnimationName )
{
	STATICHASH( Animation );
	STATICHASH( AnimEventVersion );
	const uint AnimEventVersion = ConfigManager::GetInt( sAnimEventVersion, 0, sAnimation );

	STATICHASH( NumAnimEvents );
	STATICHASH( VelocityX );
	STATICHASH( VelocityY );
	STATICHASH( VelocityYaw );
	MAKEHASH( QualifiedAnimationName );

	m_AnimEvents.Clear();

	int NumAnimEvents = ConfigManager::GetInt( sNumAnimEvents, 0, sQualifiedAnimationName );
	for( int AnimEventIndex = 0; AnimEventIndex < NumAnimEvents; ++AnimEventIndex )
	{
		AnimEvent* pAnimEvent = NULL;

		if( AnimEventVersion < 2 )
		{
			// OLD SYSTEM
			const SimpleString AnimEventType	= ConfigManager::GetSequenceString( "AnimEventType%d", AnimEventIndex, "", sQualifiedAnimationName );
			const SimpleString AnimEventName	= ConfigManager::GetSequenceString( "AnimEventName%d", AnimEventIndex, "", sQualifiedAnimationName );
			const int AnimEventFrame			= ConfigManager::GetSequenceInt( "AnimEventFrame%d", AnimEventIndex, 0, sQualifiedAnimationName );
			const float AnimEventTime			= ConfigManager::GetSequenceFloat( "AnimEventTime%d", AnimEventIndex, 0.0f, sQualifiedAnimationName );

			pAnimEvent = AnimEventFactory::GetInstance()->Create( AnimEventType, AnimEventName );
			pAnimEvent->m_Time = AnimEventFrame ? ( AnimEventFrame / ANIM_FRAMERATE ) : AnimEventTime;
		}
		else
		{
			// NEW SYSTEM
			const SimpleString AnimEventDef		= ConfigManager::GetSequenceString( "AnimEvent%dDef", AnimEventIndex, "", sQualifiedAnimationName );
			const int AnimEventFrame			= ConfigManager::GetSequenceInt( "AnimEvent%dFrame", AnimEventIndex, 0, sQualifiedAnimationName );
			pAnimEvent = AnimEventFactory::GetInstance()->Create( AnimEventDef );
			pAnimEvent->m_Time = ( static_cast<float>( AnimEventFrame ) / ANIM_FRAMERATE );
		}

		if( pAnimEvent->m_Time < 0.0f )
		{
			pAnimEvent->m_Time = GetNonLoopingLengthSeconds() + pAnimEvent->m_Time;
		}

		m_AnimEvents.PushBack( pAnimEvent );
	}

	m_Velocity.x = ConfigManager::GetFloat( sVelocityX, 0.0f, sQualifiedAnimationName );
	m_Velocity.y = ConfigManager::GetFloat( sVelocityY, 0.0f, sQualifiedAnimationName );
	m_RotationalVelocity.Yaw = DEGREES_TO_RADIANS( ConfigManager::GetFloat( sVelocityYaw, 0.0f, sQualifiedAnimationName ) );
}

uint Animation::GetLengthFrames() const
{
	return m_Length;
}

float Animation::GetLengthSeconds() const
{
	return m_Length / ANIM_FRAMERATE;
}

float Animation::GetNonLoopingLengthSeconds() const
{
	return ( m_Length - 1 ) / ANIM_FRAMERATE;
}

void Animation::GetVelocity( Vector& OutVelocity, Angles& OutRotationalVelocity ) const
{
	OutVelocity = m_Velocity;
	OutRotationalVelocity = m_RotationalVelocity;
}

// Calculate displacement at time Time given 2D velocity and yaw
Vector Animation::GetDisplacement( float Time ) const
{
	// Leaving this around in case I need to test it again. Euler integration
	// because I couldn't solve the actual equation at first.
	//const uint Steps = 500;
	//const float TimeStep = Time / Steps;
	//Vector RetVal;
	//float Yaw = 0.0f;
	//for( uint i = 0; i < Steps; ++i )
	//{
	//	Matrix Rotation = Matrix::CreateRotationAboutZ( Yaw );
	//	RetVal += ( Rotation * m_Velocity ) * TimeStep;
	//	Yaw += m_RotationalVelocity.Yaw * TimeStep;
	//}

	if( Abs( m_RotationalVelocity.Yaw ) > EPSILON )
	{
		// Solve the radius of the curved motion from the known
		// arc length and angular delta (separately per axis).
		Vector SeparableArcLength = m_Velocity * Time;
		float Theta = m_RotationalVelocity.Yaw * Time;
		Vector SeparableRadius = SeparableArcLength / Theta;
		Vector RetVal;
		RetVal.x += SeparableRadius.x * Sin( Theta );
		RetVal.y += SeparableRadius.x * ( 1.0f - Cos( Theta ) );
		RetVal.x += SeparableRadius.y * -( 1.0f - Cos( Theta ) );
		RetVal.y += SeparableRadius.y * Sin( Theta );
		return RetVal;
	}
	else
	{
		return m_Velocity * Time;
	}
}