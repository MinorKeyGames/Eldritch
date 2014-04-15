#ifndef XINPUTCONTROLLER_H
#define XINPUTCONTROLLER_H

#include "icontroller.h"

#if BUILD_SDL
#include "SDL2/SDL.h"
#endif

class XInputController : public IController
{
public:
	enum EAxes
	{
		EA_None,
		EA_LeftThumbX,		// [-1.0, 1.0]
		EA_LeftThumbY,		// [-1.0, 1.0]
		EA_RightThumbX,		// [-1.0, 1.0]
		EA_RightThumbY,		// [-1.0, 1.0]
		EA_LeftTrigger,		// [0.0, 1.0]
		EA_RightTrigger,	// [0.0, 1.0]
		EA_Max,
	};

	enum EButtons
	{
		EB_None,
		EB_Up,
		EB_Down,
		EB_Left,
		EB_Right,
		EB_Start,
		EB_Back,
		EB_LeftThumb,
		EB_RightThumb,
		EB_LeftBumper,
		EB_RightBumper,
		EB_A,
		EB_B,
		EB_X,
		EB_Y,

		// Float signals as bools according to configurable thresholds
		EB_LeftThumbUp,
		EB_LeftThumbDown,
		EB_LeftThumbLeft,
		EB_LeftThumbRight,
		EB_RightThumbUp,
		EB_RightThumbDown,
		EB_RightThumbLeft,
		EB_RightThumbRight,
		EB_LeftTrigger,
		EB_RightTrigger,

		EB_Max,
	};

	struct SXInputState
	{
		float	m_Axes[ EA_Max ];
		bool	m_Buttons[ EB_Max ];
	};

	XInputController( uint Port = 0 );
	~XInputController();

	virtual void	Tick( float DeltaTime );

	virtual bool	IsHigh( uint Signal );
	virtual bool	IsLow( uint Signal );
	virtual bool	OnRise( uint Signal );
	virtual bool	OnFall( uint Signal );

	virtual float	GetPosition( uint Axis );
	virtual float	GetVelocity( uint Axis );

	// Left is low, right is high
	virtual void	SetFeedback( float Low, float High );

	bool			ReceivedInputThisTick() const { return m_ReceivedInputThisTick; }

	SXInputState	m_CurrentState;
	SXInputState	m_LastState;

	uint			m_Port;
#if BUILD_SDL
	SDL_GameController*	m_Controller;
#endif

	float			m_LeftThumbDeadZone;
	float			m_RightThumbDeadZone;
	float			m_LeftThumbSaturationPoint;
	float			m_RightThumbSaturationPoint;
	float			m_TriggerDeadZone;
	float			m_TriggerSaturationPoint;

	float			m_LeftThumbBoolThreshold;
	float			m_RightThumbBoolThreshold;
	float			m_LeftTriggerBoolThreshold;
	float			m_RightTriggerBoolThreshold;

	// Used in conjunction with mouse to determine if cursor should be visible.
	// TODO LATER: Use to drive which inputs are shown on screen.
	bool			m_ReceivedInputThisTick;

private:
	void			ApplyDeadZones();
	void			ApplyDeadZone( EAxes Axis, float DeadZone, float SaturationPoint );
	void			SetVibration( uint16 Left, uint16 Right );
};

#endif // XINPUTCONTROLLER_H