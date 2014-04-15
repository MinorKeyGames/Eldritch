#ifndef WBACTIONCOMPOSITE_H
#define WBACTIONCOMPOSITE_H

#include "wbaction.h"
#include "array.h"

class WBActionComposite : public WBAction
{
public:
	WBActionComposite();
	virtual ~WBActionComposite();

	DEFINE_WBACTION_FACTORY( Composite );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Execute();

private:
	Array<WBAction*>	m_Actions;
};

#endif // WBACTIONCOMPOSITE_H