#ifndef ANIMATIONSTATE_H
#define ANIMATIONSTATE_H

#include "list.h"

class Animation;
class BoneArray;
class Matrix;
class Mesh;
class AnimEvent;
class Vector;
class Angles;

typedef void ( *NotifyAnimationFinishedFunc )( void*, Mesh*, Animation*, bool );
typedef void ( *NotifyAnimEventFunc )( void*, AnimEvent*, Mesh*, Animation* );
struct SAnimationListener
{
	SAnimationListener()
		:	m_NotifyFinishedFunc( NULL )
		,	m_NotifyAnimEventFunc( NULL )
		,	m_Void( NULL )
	{
	}

	NotifyAnimationFinishedFunc	m_NotifyFinishedFunc;
	NotifyAnimEventFunc			m_NotifyAnimEventFunc;
	void*						m_Void;

	bool operator==( const SAnimationListener& Other )
	{
		return (
			m_NotifyFinishedFunc == Other.m_NotifyFinishedFunc &&
			m_NotifyAnimEventFunc == Other.m_NotifyAnimEventFunc &&
			m_Void == Other.m_Void );
	}
};

class AnimationState
{
public:
	enum EAnimationEndBehavior
	{
		EAEB_Stop,	// Stop on last frame
		EAEB_Loop,	// Loop from last frame back to first
	};

	struct SPlayAnimationParams
	{
		SPlayAnimationParams()
		:	m_EndBehavior( EAEB_Stop )
		,	m_IgnoreIfAlreadyPlaying( false )
		,	m_SuppressAnimEvents( false )
		,	m_PlayRate( 1.0f )
		{
		}

		EAnimationEndBehavior	m_EndBehavior;
		bool					m_IgnoreIfAlreadyPlaying;
		bool					m_SuppressAnimEvents;
		float					m_PlayRate;
	};

	AnimationState();
	~AnimationState();

	void					Tick( float DeltaTime );
	void					PlayAnimation( Animation* pAnimation, const SPlayAnimationParams& PlayParams );
	void					StopAnimation();

	const Animation*		GetPlayingAnimation() const;	// For external callers
	const Animation*		GetAnimation() const;			// For internal use
	float					GetAnimationTime() const;
	void					SetAnimationTime( const float AnimationTime );
	float					GetAnimationPlayRate() const;
	void					SetAnimationPlayRate( const float AnimationPlayRate );
	EAnimationEndBehavior	GetAnimationEndBehavior() const;

	void					GetAnimationVelocity( Vector& OutVelocity, Angles& OutRotationalVelocity );

	// This function kind of belongs to the Mesh, BoneArray, and AnimationState classes...
	void					UpdateBones( const BoneArray* pBones, Matrix* pBoneMatrices ) const;

	void					SetMesh( Mesh* pMesh );

	void					SuppressAnimEvents( const bool Suppress ) { m_SuppressAnimEvents = Suppress; }

	void					AddAnimationListener( const SAnimationListener& AnimationListener );
	void					RemoveAnimationListener( const SAnimationListener& AnimationListener );

private:
	// TODO: Ever revisit the idea of actually using blended layers
	struct SAnimationLayer
	{
		SAnimationLayer()
		:	m_Animation( NULL )
		,	m_PlayRate( 1.0f )
		,	m_Time( 0.0f )
		,	m_EndBehavior( EAEB_Stop )
		,	m_Stopped( false )
		{
		}

		Animation*				m_Animation;
		float					m_PlayRate;
		float					m_Time;
		EAnimationEndBehavior	m_EndBehavior;
		bool					m_Stopped;
	};

	void					TickAnimEvents( const float OldTime );

	const SAnimationLayer*	GetConstAnimationLayer() const;
	SAnimationLayer*		GetAnimationLayer();
	static uint				GetFrameFromTime( const SAnimationLayer* pLayer );
	static uint				GetNextFrame( const SAnimationLayer* pLayer, uint Frame );
	static float			GetFrameDelta( const SAnimationLayer* pLayer );

	void					CallAnimEvent( AnimEvent* const pAnimEvent, const SAnimationLayer* const pLayer );

	SAnimationLayer		m_AnimationLayer;
	Mesh*				m_Mesh;

	bool				m_SuppressAnimEvents;

	List< SAnimationListener >	m_AnimationListeners;
};

#endif // ANIMATIONSTATE_H