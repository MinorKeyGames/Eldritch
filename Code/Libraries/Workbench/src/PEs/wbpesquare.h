#ifndef WBPESQUARE_H
#define WBPESQUARE_H

#include "wbpeunaryop.h"

class WBPESquare : public WBPEUnaryOp
{
public:
	WBPESquare();
	virtual ~WBPESquare();

	DEFINE_WBPE_FACTORY( Square );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPESQUARE_H
