#ifndef WBPECONSTANTINT_H
#define WBPECONSTANTINT_H

#include "wbpe.h"

class WBPEConstantInt : public WBPE
{
public:
	WBPEConstantInt();
	virtual ~WBPEConstantInt();

	DEFINE_WBPE_FACTORY( ConstantInt );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	int	m_Value;
};

#endif // WBPECONSTANTINT_H
