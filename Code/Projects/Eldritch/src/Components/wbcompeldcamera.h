#ifndef WBCOMPELDCAMERA_H
#define WBCOMPELDCAMERA_H

#include "wbeldritchcomponent.h"

class WBEvent;
class Vector;
class Angles;

class WBCompEldCamera : public WBEldritchComponent
{
public:
	WBCompEldCamera();
	virtual ~WBCompEldCamera();

	DEFINE_WBCOMP( EldCamera, WBEldritchComponent );

	virtual void	Tick( float DeltaTime );
	virtual int		GetTickOrder() { return ETO_TickDefault; }	// Needs to tick after transform

	virtual void	HandleEvent( const WBEvent& Event );

	enum EViewModifiers
	{
		EVM_None	= 0x0,
		EVM_OffsetZ	= 0x1,
		EVM_Lean	= 0x2,
		EVM_Roll	= 0x4,
		EVM_All		= EVM_OffsetZ | EVM_Lean | EVM_Roll,
	};

	void			SetViewOffsetZ( const float ViewOffsetZ );
	void			SetViewAngleOffsetRoll( const float ViewAngleOffsetRoll ) { m_ViewAngleOffsetRoll = ViewAngleOffsetRoll; }

	Vector			GetViewTranslationOffset( const EViewModifiers Modifiers ) const;
	Angles			GetViewOrientationOffset( const EViewModifiers Modifiers ) const;

	void			SetLeanPosition( const float LeanPosition ) { m_LeanPosition = LeanPosition; }

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void	UpdateLean( const float TargetRoll, const float DeltaTime );
	float	GetDesiredLean( const float LeanPosition ) const;
	Vector	GetLeanOffset( const float LeanRoll ) const;

	float	m_ViewOffsetZ;			// Config/transient, should possibly be serialized
	float	m_LastViewOffsetZ;		// Transient, hack for reviving from death

	float	m_ViewAngleOffsetRoll;	// Transient

	// Seems a bit hacky to put leaning in the camera, but I'll give it a try.
	float	m_LeanRoll;				// Transient
	float	m_LeanPosition;			// Transient, maps [-1,1] into lean angle range.
	float	m_LeanVelocity;			// Config
	float	m_LeanRollMax;			// Config
	float	m_LeanRadius;			// Config
	float	m_LeanExtent;			// Config
};

#endif // WBCOMPELDCAMERA_H