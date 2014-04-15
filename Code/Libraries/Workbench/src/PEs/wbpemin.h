#ifndef WBPEMIN_H
#define WBPEMIN_H

#include "wbpebinaryop.h"

class WBPEMin : public WBPEBinaryOp
{
public:
	WBPEMin();
	virtual ~WBPEMin();

	DEFINE_WBPE_FACTORY( Min );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPEMIN_H
