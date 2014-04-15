#ifndef WBPECONSTANTVECTOR_H
#define WBPECONSTANTVECTOR_H

#include "wbpe.h"
#include "vector.h"

class WBPEConstantVector : public WBPE
{
public:
	WBPEConstantVector();
	virtual ~WBPEConstantVector();

	DEFINE_WBPE_FACTORY( ConstantVector );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	Vector	m_Value;
};

#endif // WBPECONSTANTVECTOR_H
