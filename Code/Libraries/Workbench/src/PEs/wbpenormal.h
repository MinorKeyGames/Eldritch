#ifndef WBPENORMAL_H
#define WBPENORMAL_H

#include "wbpeunaryop.h"

class WBPENormal : public WBPEUnaryOp
{
public:
	WBPENormal();
	virtual ~WBPENormal();

	DEFINE_WBPE_FACTORY( Normal );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPENORMAL_H
