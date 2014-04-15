#ifndef WBACTIONELDADDBLOCK_H
#define WBACTIONELDADDBLOCK_H

#include "wbaction.h"
#include "wbparamevaluator.h"

class WBActionEldAddBlock : public WBAction
{
public:
	WBActionEldAddBlock();
	virtual ~WBActionEldAddBlock();

	DEFINE_WBACTION_FACTORY( EldAddBlock );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	WBParamEvaluator	m_SourcePE;
	byte				m_VoxelValue;
};

#endif // WBACTIONELDADDBLOCK_H