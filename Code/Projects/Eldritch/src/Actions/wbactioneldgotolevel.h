#ifndef WBACTIONELDGOTOLEVEL_H
#define WBACTIONELDGOTOLEVEL_H

#include "wbaction.h"
#include "simplestring.h"

class WBActionEldGoToLevel : public WBAction
{
public:
	WBActionEldGoToLevel();
	virtual ~WBActionEldGoToLevel();

	DEFINE_WBACTION_FACTORY( EldGoToLevel );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Execute();

private:
	bool			m_GoToNextLevel;
	bool			m_GoToPrevLevel;

	bool			m_ReturnToHub;
	bool			m_Restart;		// If m_ReturnToHub, flush worlds?
	bool			m_FullRestart;	// If m_ReturnToHub, flush worlds and hub?
	bool			m_Immediate;	// Go to level immediately. Only safe to call outside world tick (e.g., from UI tick).

	SimpleString	m_Level;
	HashedString	m_WorldDef;
};

#endif // WBACTIONELDGOTOLEVEL_H