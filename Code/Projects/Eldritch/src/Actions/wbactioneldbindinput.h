#ifndef WBACTIONELDBINDINPUT_H
#define WBACTIONELDBINDINPUT_H

#include "wbaction.h"
#include "simplestring.h"

class WBActionEldBindInput : public WBAction
{
public:
	WBActionEldBindInput();
	virtual ~WBActionEldBindInput();

	DEFINE_WBACTION_FACTORY( EldBindInput );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	SimpleString	m_Input;
};

#endif // WBACTIONELDBINDINPUT_H