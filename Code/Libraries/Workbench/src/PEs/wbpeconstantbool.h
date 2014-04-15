#ifndef WBPECONSTANTBOOL_H
#define WBPECONSTANTBOOL_H

#include "wbpe.h"

class WBPEConstantBool : public WBPE
{
public:
	WBPEConstantBool();
	virtual ~WBPEConstantBool();

	DEFINE_WBPE_FACTORY( ConstantBool );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	bool	m_Value;
};

#endif // WBPECONSTANTBOOL_H
