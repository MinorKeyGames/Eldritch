#ifndef WBACTIONELDDAMAGEWORLD_H
#define WBACTIONELDDAMAGEWORLD_H

// Apply radial damage to world from entity location.
// (To remove a single voxel, use EldRemoveBlock.)

#include "wbaction.h"
#include "wbparamevaluator.h"

class WBActionEldDamageWorld : public WBAction
{
public:
	WBActionEldDamageWorld();
	virtual ~WBActionEldDamageWorld();

	DEFINE_WBACTION_FACTORY( EldDamageWorld );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Execute();

private:
	float	m_Radius;
};

#endif // WBACTIONELDDAMAGEWORLD_H