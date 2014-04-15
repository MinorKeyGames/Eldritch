#ifndef WBCOMPELDSLEEPER_H
#define WBCOMPELDSLEEPER_H

#include "wbeldritchcomponent.h"
#include "vector.h"

class WBCompEldSleeper : public WBEldritchComponent
{
public:
	WBCompEldSleeper();
	virtual ~WBCompEldSleeper();

	DEFINE_WBCOMP( EldSleeper, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void	Wake();
	void	HandleNoise( const Vector& NoiseLocation, const float NoiseRadius );

	bool	m_IsAwake;			// Config/Serialized
	float	m_NoiseThreshold;	// Config
};

#endif // WBCOMPELDSLEEPER_H