#ifndef WBPEDOT_H
#define WBPEDOT_H

#include "wbpebinaryop.h"

class WBPEDot : public WBPEBinaryOp
{
public:
	WBPEDot();
	virtual ~WBPEDot();

	DEFINE_WBPE_FACTORY( Dot );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPEDOT_H
