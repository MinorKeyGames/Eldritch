#ifndef WBPENULL_H
#define WBPENULL_H

#include "wbpe.h"

class WBPENull : public WBPE
{
public:
	WBPENull();
	virtual ~WBPENull();

	DEFINE_WBPE_FACTORY( Null );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPENULL_H
