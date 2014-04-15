#ifndef WBPEELDHARDSCALAR_H
#define WBPEELDHARDSCALAR_H

#include "PEs/wbpeunaryop.h"

class WBPEEldHardScalar : public WBPEUnaryOp
{
public:
	WBPEEldHardScalar();
	virtual ~WBPEEldHardScalar();

	DEFINE_WBPE_FACTORY( EldHardScalar );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	float	m_Scalar;
};

#endif // WBPEELDHARDSCALAR_H
