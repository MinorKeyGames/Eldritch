#ifndef WBPEPOW_H
#define WBPEPOW_H

#include "wbpebinaryop.h"

class WBPEPow : public WBPEBinaryOp
{
public:
	WBPEPow();
	virtual ~WBPEPow();

	DEFINE_WBPE_FACTORY( Pow );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPEPOW_H
