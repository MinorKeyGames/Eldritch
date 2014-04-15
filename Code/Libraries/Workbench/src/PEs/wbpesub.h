#ifndef WBPESUB_H
#define WBPESUB_H

#include "wbpebinaryop.h"

class WBPESub : public WBPEBinaryOp
{
public:
	WBPESub();
	virtual ~WBPESub();

	DEFINE_WBPE_FACTORY( Sub );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPESUB_H
