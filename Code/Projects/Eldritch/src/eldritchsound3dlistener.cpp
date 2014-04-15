#include "core.h"
#include "eldritchsound3dlistener.h"
#include "eldritchworld.h"
#include "collisioninfo.h"
#include "mathcore.h"
#include "isoundinstance.h"
#include "configmanager.h"

EldritchSound3DListener::EldritchSound3DListener()
:	m_World( NULL )
,	m_VerticalScalar( 0.0f )
,	m_OccludedFalloffRadius( 0.0f )
{
}

EldritchSound3DListener::~EldritchSound3DListener()
{
}

void EldritchSound3DListener::Initialize()
{
	STATICHASH( AudioSystem );

	STATICHASH( VerticalScalar );
	m_VerticalScalar = ConfigManager::GetFloat( sVerticalScalar, 1.0f, sAudioSystem );

	STATICHASH( OccludedFalloffRadius );
	m_OccludedFalloffRadius = ConfigManager::GetFloat( sOccludedFalloffRadius, 0.0f, sAudioSystem );
}

/*virtual*/ void EldritchSound3DListener::ModifyAttenuation( ISoundInstance* const pSoundInstance, float& Attenuation ) const
{
	PROFILE_FUNCTION;

	if( !m_World )
	{
		return;
	}

	if( !pSoundInstance->ShouldCalcOcclusion() )
	{
		return;
	}

	CollisionInfo FromListenerInfo;
	FromListenerInfo.m_CollideWorld			= true;
	FromListenerInfo.m_CollideEntities		= true;
	FromListenerInfo.m_UserFlags			= EECF_Occlusion;
	FromListenerInfo.m_StopAtAnyCollision	= true;

	CollisionInfo FromSoundInfo;
	FromSoundInfo.m_CollideWorld			= true;
	FromSoundInfo.m_CollideEntities			= true;
	FromSoundInfo.m_UserFlags				= EECF_Occlusion;
	FromSoundInfo.m_StopAtAnyCollision		= true;

	const Vector	SoundLocation	= pSoundInstance->GetLocation();
	const bool		Occluded		= m_World->LineCheck( m_Location, SoundLocation, FromListenerInfo ) &&
									  m_World->LineCheck( SoundLocation, m_Location, FromSoundInfo );

	if( Occluded )
	{
		// Use the ratio between the distances to sound source and to occlusion as a factor in attenuation.
		Vector				ToOcclusionNear			= FromListenerInfo.m_Intersection - m_Location;
		ToOcclusionNear.z							*= m_VerticalScalar;

		Vector				ToOcclusionFar			= FromSoundInfo.m_Intersection - SoundLocation;
		ToOcclusionFar.z							*= m_VerticalScalar;

		Vector				ToSound					= SoundLocation - m_Location;
		ToSound.z									*= m_VerticalScalar;

		const float			DistanceToOcclusionNear	= ToOcclusionNear.Length();
		const float			DistanceToOcclusionFar	= ToOcclusionFar.Length();
		const float			DistanceToSound			= ToSound.Length();
		const float			DistanceRatio			= ( DistanceToOcclusionNear + DistanceToOcclusionFar ) / DistanceToSound;

		// And attenuate occluded sounds more if they're more distant.
		const float			OcclusionAttenuation	= Attenuate( DistanceToSound, m_OccludedFalloffRadius );

		Attenuation *= DistanceRatio * OcclusionAttenuation;
	}
}