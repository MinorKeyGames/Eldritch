#ifndef WBCOMPELDLIGHT_H
#define WBCOMPELDLIGHT_H

#include "wbeldritchcomponent.h"
#include "vector.h"
#include "vector4.h"

class WBCompEldLight : public WBEldritchComponent
{
public:
	WBCompEldLight();
	virtual ~WBCompEldLight();

	DEFINE_WBCOMP( EldLight, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			AddLight();
	void			RemoveLight();

	float	m_Radius;			// Config
	Vector4	m_Color;			// Config

	bool	m_HasAddedLight;	// Serialized
	Vector	m_LightLocation;	// Serialized

	bool	m_DeferAddLight;	// Config
};

#endif // WBCOMPELDLIGHT_H