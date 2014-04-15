#ifndef WBACTIONELDSPAWNENTITY_H
#define WBACTIONELDSPAWNENTITY_H

#include "wbaction.h"
#include "wbparamevaluator.h"

class WBActionEldSpawnEntity : public WBAction
{
public:
	WBActionEldSpawnEntity();
	virtual ~WBActionEldSpawnEntity();

	DEFINE_WBACTION_FACTORY( EldSpawnEntity );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	void			GetSpawnTransform( WBEntity* const pSpawnedEntity, Vector& OutLocation, Vector& OutImpulse, Angles& OutOrientation );

	SimpleString		m_EntityDef;
	WBParamEvaluator	m_EntityDefPE;

	// If overrides are not present, the active entity's transform is used.
	WBParamEvaluator	m_LocationOverridePE;
	WBParamEvaluator	m_OrientationOverridePE;

	bool				m_YawOnly;

	float				m_SpawnImpulseZ;
	float				m_SpawnImpulse;
	WBParamEvaluator	m_SpawnImpulsePE;

	float				m_SpawnOffsetZ;
};

#endif // WBACTIONELDSPAWNENTITY_H