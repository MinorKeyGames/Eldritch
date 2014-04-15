#ifndef WBPEDIV_H
#define WBPEDIV_H

#include "wbpebinaryop.h"

class WBPEDiv : public WBPEBinaryOp
{
public:
	WBPEDiv();
	virtual ~WBPEDiv();

	DEFINE_WBPE_FACTORY( Div );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPEDIV_H
