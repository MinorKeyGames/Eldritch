#ifndef WBPECONSTANTFLOAT_H
#define WBPECONSTANTFLOAT_H

#include "wbpe.h"

class WBPEConstantFloat : public WBPE
{
public:
	WBPEConstantFloat();
	virtual ~WBPEConstantFloat();

	DEFINE_WBPE_FACTORY( ConstantFloat );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	float	m_Value;
};

#endif // WBPECONSTANTFLOAT_H
