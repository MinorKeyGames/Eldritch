#ifndef WBPEMAX_H
#define WBPEMAX_H

#include "wbpebinaryop.h"

class WBPEMax : public WBPEBinaryOp
{
public:
	WBPEMax();
	virtual ~WBPEMax();

	DEFINE_WBPE_FACTORY( Max );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPEMAX_H
