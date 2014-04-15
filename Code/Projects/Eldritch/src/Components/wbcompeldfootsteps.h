#ifndef WBCOMPELDFOOTSTEPS_H
#define WBCOMPELDFOOTSTEPS_H

#include "wbeldritchcomponent.h"
#include "vector.h"

class WBCompEldFootsteps : public WBEldritchComponent
{
public:
	WBCompEldFootsteps();
	virtual ~WBCompEldFootsteps();

	DEFINE_WBCOMP( EldFootsteps, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

#if BUILD_DEV
	void			SetFootstepsDisabled( const bool Disabled ) { m_TEMPHACKFootstepsDisabled = Disabled; }
#endif

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	float	m_FootstepDistanceSq;
	Vector	m_LastFootstepLocation;

#if BUILD_DEV
	bool	m_TEMPHACKFootstepsDisabled;
#endif
};

#endif // WBCOMPELDUSABLE_H