#ifndef WBPEXOR_H
#define WBPEXOR_H

#include "wbpebinaryop.h"

class WBPEXOR : public WBPEBinaryOp
{
public:
	WBPEXOR();
	virtual ~WBPEXOR();

	DEFINE_WBPE_FACTORY( XOR );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPEXOR_H
