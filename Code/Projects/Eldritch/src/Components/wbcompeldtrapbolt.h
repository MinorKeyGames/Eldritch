#ifndef WBCOMPELDTRAPBOLT_H
#define WBCOMPELDTRAPBOLT_H

#include "wbeldritchcomponent.h"
#include "vector.h"

class WBCompEldTrapBolt : public WBEldritchComponent
{
public:
	WBCompEldTrapBolt();
	virtual ~WBCompEldTrapBolt();

	DEFINE_WBCOMP( EldTrapBolt, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			TriggerTrapBolt();	// Deal damage to touching entities and destroy self
	void			LaunchTrapBolt();

	Vector			Quantize( const Vector& V ) const;

	float	m_CollisionFatten;	// Config
	float	m_MeshFatten;		// Config

	float	m_EndpointSpacing;	// Config
	float	m_AnchorDepth;		// Config

	Vector	m_Start;			// Serialized
	Vector	m_End;				// Serialized

	Vector	m_AnchorStart;		// Serialized
	Vector	m_AnchorEnd;		// Serialized

	bool	m_Launched;			// Serialized
	bool	m_Triggered;		// Serialized
};

#endif // WBCOMPELDDROPSHADOW_H