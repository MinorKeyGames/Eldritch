#ifndef WBPENOT_H
#define WBPENOT_H

#include "wbpeunaryop.h"

class WBPENOT : public WBPEUnaryOp
{
public:
	WBPENOT();
	virtual ~WBPENOT();

	DEFINE_WBPE_FACTORY( NOT );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPENOT_H
