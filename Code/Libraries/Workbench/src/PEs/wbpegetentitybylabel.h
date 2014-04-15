#ifndef WBPEGETENTITYBYLABEL_H
#define WBPEGETENTITYBYLABEL_H

#include "wbpe.h"

class WBPEGetEntityByLabel : public WBPE
{
public:
	WBPEGetEntityByLabel();
	virtual ~WBPEGetEntityByLabel();

	DEFINE_WBPE_FACTORY( GetEntityByLabel );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	HashedString	m_Label;
};

#endif // WBPEGETENTITYBYLABEL_H
