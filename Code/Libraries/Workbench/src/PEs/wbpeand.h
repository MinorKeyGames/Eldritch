#ifndef WBPEAND_H
#define WBPEAND_H

#include "wbpebinaryop.h"

class WBPEAND : public WBPEBinaryOp
{
public:
	WBPEAND();
	virtual ~WBPEAND();

	DEFINE_WBPE_FACTORY( AND );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPEAND_H
