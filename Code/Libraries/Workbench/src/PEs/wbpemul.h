#ifndef WBPEMUL_H
#define WBPEMUL_H

#include "wbpebinaryop.h"

class WBPEMul : public WBPEBinaryOp
{
public:
	WBPEMul();
	virtual ~WBPEMul();

	DEFINE_WBPE_FACTORY( Mul );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPEMUL_H
