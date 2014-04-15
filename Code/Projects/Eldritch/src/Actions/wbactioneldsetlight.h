#ifndef WBACTIONELDSETLIGHT_H
#define WBACTIONELDSETLIGHT_H

#include "wbaction.h"

class WBActionEldSetLight : public WBAction
{
public:
	WBActionEldSetLight();
	virtual ~WBActionEldSetLight();

	DEFINE_WBACTION_FACTORY( EldSetLight );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	bool			m_AddLight;
};

#endif // WBACTIONELDPLAYANIM_H