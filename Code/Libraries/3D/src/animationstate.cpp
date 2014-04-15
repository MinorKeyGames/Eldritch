#include "core.h"
#include "animationstate.h"
#include "animation.h"
#include "bonearray.h"
#include "mathcore.h"
#include "matrix.h"
#include "bonearray.h"
#include "animevent.h"

AnimationState::AnimationState()
:	m_AnimationLayer()
,	m_Mesh( NULL )
,	m_SuppressAnimEvents( false )
,	m_AnimationListeners()
{
}

AnimationState::~AnimationState()
{
}

void AnimationState::Tick( float DeltaTime )
{
	SAnimationLayer* pLayer = GetAnimationLayer();
	DEVASSERT( pLayer );

	if( !pLayer->m_Animation )
	{
		return;
	}

	const float OldTime = pLayer->m_Time;
	const float LayerDeltaTime = DeltaTime * pLayer->m_PlayRate;

	if( pLayer->m_EndBehavior == EAEB_Stop && !pLayer->m_Stopped )
	{
		pLayer->m_Time += LayerDeltaTime;
		const float NonLoopingLength = pLayer->m_Animation->GetNonLoopingLengthSeconds();
		if( pLayer->m_Time >= NonLoopingLength )
		{
			pLayer->m_Stopped = true;
			pLayer->m_Time = NonLoopingLength;
			
			// Notify any listeners that the animation has finished
			FOR_EACH_LIST( NotifyIter, m_AnimationListeners, SAnimationListener )
			{
				SAnimationListener& AnimationListener = *NotifyIter;
				if( AnimationListener.m_NotifyFinishedFunc )
				{
					AnimationListener.m_NotifyFinishedFunc( AnimationListener.m_Void, m_Mesh, pLayer->m_Animation, false );
				}
			}
		}
	}
	else if( pLayer->m_EndBehavior == EAEB_Loop )
	{
		pLayer->m_Time = Mod( pLayer->m_Time + LayerDeltaTime, pLayer->m_Animation->GetLengthSeconds() );
	}

	DEVASSERT( pLayer->m_Time >= 0.0f );
	DEVASSERT( pLayer->m_Time < pLayer->m_Animation->GetLengthSeconds() );
	DEVASSERT( static_cast<uint>( pLayer->m_Time * ANIM_FRAMERATE ) < pLayer->m_Animation->GetLengthFrames() );

	if( !m_SuppressAnimEvents )
	{
		TickAnimEvents( OldTime );
	}
}

void AnimationState::TickAnimEvents( const float OldTime )
{
	SAnimationLayer* pLayer = GetAnimationLayer();
	DEVASSERT( pLayer );

	// NOTE: This logic precludes events on the last frame for non-looping
	// animations (which is okay, because we don't want to call it every
	// frame when the animation has ended, and we notify listeners about
	// animation finishing separately).
	for( uint AnimEventIndex = 0; AnimEventIndex < pLayer->m_Animation->m_AnimEvents.Size(); ++AnimEventIndex )
	{
		AnimEvent* pAnimEvent = pLayer->m_Animation->m_AnimEvents[ AnimEventIndex ];
		const bool IsPostAnimEventTime	= pLayer->m_Time > pAnimEvent->m_Time;
		const bool WasPreAnimEventTime	= OldTime <= pAnimEvent->m_Time && pAnimEvent->m_Time > 0.0f;	// Because 0-frame events are triggered immediately
		const bool HasLooped			= OldTime > pLayer->m_Time;										// Catch anim events when animation loops (including 0-frame)
		const bool ShouldCallAnimEvent	= IsPostAnimEventTime && ( WasPreAnimEventTime || HasLooped );
		if( ShouldCallAnimEvent )
		{
			CallAnimEvent( pAnimEvent, pLayer );
		}
	}
}

void AnimationState::PlayAnimation( Animation* pAnimation, const SPlayAnimationParams& PlayParams )
{
	SAnimationLayer* pLayer = GetAnimationLayer();
	DEVASSERT( pLayer );

	if( pLayer->m_Animation == pAnimation && PlayParams.m_IgnoreIfAlreadyPlaying )
	{
		// We're already playing this animation and don't want to restart.
		// But do update the play rate if it has changed. Poorly documented side effect!
		pLayer->m_PlayRate = PlayParams.m_PlayRate;
		return;
	}

	// Notify listeners that the current animation was interrupted
	if( pLayer->m_Animation && !pLayer->m_Stopped )
	{
		FOR_EACH_LIST( NotifyIter, m_AnimationListeners, SAnimationListener )
		{
			SAnimationListener& AnimationListener = *NotifyIter;
			if( AnimationListener.m_NotifyFinishedFunc )
			{
				AnimationListener.m_NotifyFinishedFunc( AnimationListener.m_Void, m_Mesh, pLayer->m_Animation, true );
			}
		}
	}

	pLayer->m_Animation		= pAnimation;
	pLayer->m_PlayRate		= PlayParams.m_PlayRate;
	pLayer->m_Time			= 0.0f;
	pLayer->m_EndBehavior	= PlayParams.m_EndBehavior;
	pLayer->m_Stopped		= false;

	// Immediately play any anim events on the first frame
	if( !PlayParams.m_SuppressAnimEvents && pLayer->m_Animation )
	{
		for( uint AnimEventIndex = 0; AnimEventIndex < pLayer->m_Animation->m_AnimEvents.Size(); ++AnimEventIndex )
		{
			AnimEvent* pAnimEvent = pLayer->m_Animation->m_AnimEvents[ AnimEventIndex ];
			if( pAnimEvent->m_Time == 0.0f )
			{
				CallAnimEvent( pAnimEvent, pLayer );
			}
		}
	}
}

void AnimationState::StopAnimation()
{
	SAnimationLayer* pLayer = GetAnimationLayer();
	pLayer->m_Animation = NULL;
}

const Animation* AnimationState::GetPlayingAnimation() const
{
	const SAnimationLayer* pLayer = GetConstAnimationLayer();
	if( pLayer && !pLayer->m_Stopped )
	{
		return pLayer->m_Animation;
	}
	else
	{
		return NULL;
	}
}


const Animation* AnimationState::GetAnimation() const
{
	const SAnimationLayer* pLayer = GetConstAnimationLayer();
	if( pLayer )
	{
		return pLayer->m_Animation;
	}
	else
	{
		return NULL;
	}
}

float AnimationState::GetAnimationTime() const
{
	const SAnimationLayer* pLayer = GetConstAnimationLayer();
	if( pLayer )
	{
		return pLayer->m_Time;
	}
	else
	{
		return 0.0f;
	}
}

void AnimationState::SetAnimationTime( const float AnimationTime )
{
	SAnimationLayer* pLayer = GetAnimationLayer();
	if( pLayer )
	{
		pLayer->m_Time = AnimationTime;
	}
}

float AnimationState::GetAnimationPlayRate() const
{
	const SAnimationLayer* pLayer = GetConstAnimationLayer();
	if( pLayer )
	{
		return pLayer->m_PlayRate;
	}
	else
	{
		return 1.0f;
	}
}

void AnimationState::SetAnimationPlayRate( const float AnimationPlayRate )
{
	SAnimationLayer* pLayer = GetAnimationLayer();
	if( pLayer )
	{
		pLayer->m_PlayRate = AnimationPlayRate;
	}
}

AnimationState::EAnimationEndBehavior AnimationState::GetAnimationEndBehavior() const
{
	const SAnimationLayer* pLayer = GetConstAnimationLayer();
	if( pLayer )
	{
		return pLayer->m_EndBehavior;
	}
	else
	{
		return EAEB_Stop;
	}
}

void AnimationState::GetAnimationVelocity( Vector& OutVelocity, Angles& OutRotationalVelocity )
{
	const SAnimationLayer* pLayer = GetConstAnimationLayer();
	if( pLayer && pLayer->m_Animation && !pLayer->m_Stopped ) 
	{
		pLayer->m_Animation->GetVelocity( OutVelocity, OutRotationalVelocity );
	}
}

void AnimationState::SetMesh( Mesh* pMesh )
{
	m_Mesh = pMesh;
}

const AnimationState::SAnimationLayer* AnimationState::GetConstAnimationLayer() const
{
	return &m_AnimationLayer;
}

AnimationState::SAnimationLayer* AnimationState::GetAnimationLayer()
{
	return &m_AnimationLayer;
}

/*static*/ uint AnimationState::GetFrameFromTime( const SAnimationLayer* pLayer )
{
	if( pLayer && pLayer->m_Animation )
	{
		uint Frame = 0;
		if( pLayer->m_Animation->m_Length > 0 )
		{
			Frame = (uint)( pLayer->m_Time * ANIM_FRAMERATE );
			DEVASSERT( Frame < pLayer->m_Animation->GetLengthFrames() );
		}
		Frame += pLayer->m_Animation->m_StartFrame;
		return Frame;
	}
	else
	{
		return 0;	// Always return to T-pose if there's not an animation playing on this layer
	}
}

/*static*/ uint AnimationState::GetNextFrame( const SAnimationLayer* pLayer, uint Frame )
{
	if( pLayer && pLayer->m_Animation )
	{
		if( pLayer->m_Animation->m_Length > 0 )
		{
			Frame -= pLayer->m_Animation->m_StartFrame;
			Frame++;

			if( Frame >= pLayer->m_Animation->m_Length )
			{
				if( pLayer->m_EndBehavior == EAEB_Stop )
				{
					Frame--;	// Keep interpolating into the same frame (see also ::Tick())
				}
				else if( pLayer->m_EndBehavior == EAEB_Loop )
				{
					Frame %= pLayer->m_Animation->m_Length;
				}
			}

			Frame += pLayer->m_Animation->m_StartFrame;
		}
		return Frame;
	}
	else
	{
		return 0;	// Always return T-pose if there's not an animation playing
	}
}

/*static*/ float AnimationState::GetFrameDelta( const SAnimationLayer* pLayer )
{
	if( pLayer )
	{
		return Mod( pLayer->m_Time * ANIM_FRAMERATE, 1.0f );
	}
	else
	{
		return 0.0f;
	}
}

void AnimationState::UpdateBones( const BoneArray* pBones, Matrix* pBoneMatrices ) const
{
	// TODO: Need to refactor this a ton if I try layered animation again
	const SAnimationLayer* pLayer = GetConstAnimationLayer();
	uint CurrentFrame = GetFrameFromTime( pLayer );
	uint NextFrame = GetNextFrame( pLayer, CurrentFrame );
	float FrameDelta = GetFrameDelta( pLayer );

	int NumBones = pBones->GetNumBones();
	for( int i = 0; i < NumBones; ++i )
	{
		SBone InterpolatedBone;
		pBones->GetInterpolatedBone( i, CurrentFrame, NextFrame, FrameDelta, InterpolatedBone );

		pBoneMatrices[i] = InterpolatedBone.m_Quat.ToMatrix();
		pBoneMatrices[i].SetTranslationElements( InterpolatedBone.m_Vector );
		// NOTE: Setting the translation elements produces the same results in
		//// the simple case as multiplying by a translation matrix, but is faster.
	}
}

void AnimationState::AddAnimationListener( const SAnimationListener& AnimationListener )
{
	m_AnimationListeners.PushBack( AnimationListener );
}

void AnimationState::RemoveAnimationListener( const SAnimationListener& AnimationListener )
{
	m_AnimationListeners.Remove( AnimationListener );
}

void AnimationState::CallAnimEvent( AnimEvent* const pAnimEvent, const SAnimationLayer* const pLayer )
{
	pAnimEvent->Call( m_Mesh, pLayer->m_Animation );

	// Notify any listeners about this event
	FOR_EACH_LIST( NotifyIter, m_AnimationListeners, SAnimationListener )
	{
		SAnimationListener& AnimationListener = *NotifyIter;
		if( AnimationListener.m_NotifyAnimEventFunc )
		{
			AnimationListener.m_NotifyAnimEventFunc( AnimationListener.m_Void, pAnimEvent, m_Mesh, pLayer->m_Animation );
		}
	}
}