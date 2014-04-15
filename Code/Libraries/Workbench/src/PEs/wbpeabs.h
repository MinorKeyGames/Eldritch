#ifndef WBPEABS_H
#define WBPEABS_H

#include "wbpeunaryop.h"

class WBPEAbs : public WBPEUnaryOp
{
public:
	WBPEAbs();
	virtual ~WBPEAbs();

	DEFINE_WBPE_FACTORY( Abs );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPEABS_H
