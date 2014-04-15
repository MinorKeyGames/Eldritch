#ifndef WBPEADD_H
#define WBPEADD_H

#include "wbpebinaryop.h"

class WBPEAdd : public WBPEBinaryOp
{
public:
	WBPEAdd();
	virtual ~WBPEAdd();

	DEFINE_WBPE_FACTORY( Add );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPEADD_H
