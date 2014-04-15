#ifndef WBPEELDDISTANCE_H
#define WBPEELDDISTANCE_H

#include "PEs/wbpebinaryop.h"

class WBPEEldDistance : public WBPEBinaryOp
{
public:
	WBPEEldDistance();
	virtual ~WBPEEldDistance();

	DEFINE_WBPE_FACTORY( EldDistance );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPEELDDISTANCE_H
