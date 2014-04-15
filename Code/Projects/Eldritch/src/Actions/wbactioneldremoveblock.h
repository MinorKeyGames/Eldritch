#ifndef WBACTIONELDREMOVEBLOCK_H
#define WBACTIONELDREMOVEBLOCK_H

#include "wbaction.h"
#include "wbparamevaluator.h"

class WBActionEldRemoveBlock : public WBAction
{
public:
	WBActionEldRemoveBlock();
	virtual ~WBActionEldRemoveBlock();

	DEFINE_WBACTION_FACTORY( EldRemoveBlock );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	void			TraceFromSourceEntity( WBEntity* const pSourceEntity ) const;
	void			RemoveAtSourceVector( const Vector& SourceLocation ) const;

	WBParamEvaluator	m_SourcePE;
};

#endif // WBACTIONELDREMOVEBLOCK_H