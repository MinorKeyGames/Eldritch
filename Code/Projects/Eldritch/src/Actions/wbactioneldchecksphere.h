#ifndef WBACTIONELDCHECKSPHERE_H
#define WBACTIONELDCHECKSPHERE_H

#include "wbaction.h"
#include "wbparamevaluator.h"

class WBActionEldCheckSphere : public WBAction
{
public:
	WBActionEldCheckSphere();
	virtual ~WBActionEldCheckSphere();

	DEFINE_WBACTION_FACTORY( EldCheckSphere );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	void			GetSphereTransform( Vector& OutLocation ) const;

	float			m_RadiusSq;
	HashedString	m_CheckTag;
};

#endif // WBACTIONELDCHECKSPHERE_H