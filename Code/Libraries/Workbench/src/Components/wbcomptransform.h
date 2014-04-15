#ifndef WBCOMPTRANSFORM_H
#define WBCOMPTRANSFORM_H

#include "../wbcomponent.h"
#include "vector.h"
#include "angles.h"

class WBCompTransform : public WBComponent
{
public:
	WBCompTransform();
	virtual ~WBCompTransform();

	DEFINE_WBCOMP( Transform, WBComponent );

	virtual void	ServerTick( float DeltaTime );
	virtual int		GetTickOrder() { return ETO_TickFirst; }

	Vector	GetLocation() const { return m_Location; }
	void	SetLocation( const Vector& NewLocation );
	void	MoveBy( const Vector& Offset );

	Vector	GetVelocity() const { return m_Velocity; }
	void	SetVelocity( const Vector& NewVelocity ) { m_Velocity = NewVelocity; }

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	virtual void	Report() const;

private:
	Vector		m_Location;
	Vector		m_Velocity;
};

#endif // WBCOMPTRANSFORM_H