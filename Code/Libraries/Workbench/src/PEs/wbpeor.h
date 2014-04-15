#ifndef WBPEOR_H
#define WBPEOR_H

#include "wbpebinaryop.h"

class WBPEOR : public WBPEBinaryOp
{
public:
	WBPEOR();
	virtual ~WBPEOR();

	DEFINE_WBPE_FACTORY( OR );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPEOR_H
