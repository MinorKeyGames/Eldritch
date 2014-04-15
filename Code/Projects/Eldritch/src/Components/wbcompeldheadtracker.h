#ifndef WBCOMPELDHEADTRACKER_H
#define WBCOMPELDHEADTRACKER_H

#include "wbeldritchcomponent.h"
#include "ibonemodifier.h"
#include "wbentityref.h"
#include "vector.h"
#include "quat.h"
#include "hashedstring.h"
#include "angles.h"

class BoneArray;
class Matrix;

class WBCompEldHeadTracker : public WBEldritchComponent, public IBoneModifier
{
public:
	WBCompEldHeadTracker();
	virtual ~WBCompEldHeadTracker();

	DEFINE_WBCOMP( EldHeadTracker, WBEldritchComponent );

	virtual void	Tick( float DeltaTime );

	virtual void	HandleEvent( const WBEvent& Event );

#if BUILD_DEV
	virtual void	DebugRender() const;
#endif

	// IBoneModifier interface
	virtual void	Modify( const BoneArray* pBones, Matrix* pInOutBoneMatrices );

	Vector			GetEyesLocation() const;
	Vector			GetLookDirection() const;

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			LookAtEntity( WBEntity* const pLookAtTarget );
	void			LookAtLocation( const Vector& LookAtTarget );
	void			StopLooking();

	enum ETrackMode
	{
		ETM_None,
		ETM_Location,
		ETM_Entity,
	};

	ETrackMode		m_TrackMode;
	HashedString	m_HeadBoneName;				// Config
	Vector			m_HeadOffset;				// Config; I think this is supposed to be the object space location of the head bone
	Vector			m_EyesOffset;				// Config; object space location of the eyes
	float			m_MaxRotationRadians;		// Config
	float			m_LookVelocity;				// Config, radians per second
	WBEntityRef		m_LookAtTargetEntity;
	Vector			m_LookAtTargetLocation;
	Quat			m_LookRotationOS;			// Object space (relative to owner's rotation)
};

#endif // WBCOMPELDHEADTRACKER_H