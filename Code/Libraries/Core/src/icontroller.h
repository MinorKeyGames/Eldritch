#ifndef ICONTROLLER_H
#define ICONTROLLER_H

class IController
{
public:
	virtual void	Tick( float DeltaTime ) { Unused( DeltaTime ); }

	virtual bool	IsHigh( uint Signal ) { Unused( Signal ); return false; }
	virtual bool	IsLow( uint Signal ) { Unused( Signal ); return false; }
	virtual bool	OnRise( uint Signal ) { Unused( Signal ); return false; }
	virtual bool	OnFall( uint Signal ) { Unused( Signal ); return false; }

	virtual float	GetPosition( uint Axis ) { Unused( Axis ); return 0.0f; }
	virtual float	GetVelocity( uint Axis ) { Unused( Axis ); return 0.0f; }

	virtual void	SetFeedback( float Low, float High ) { Unused( Low ); Unused( High ); }
};

#endif // ICONTROLLER_H