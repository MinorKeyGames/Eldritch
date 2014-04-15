#ifndef WBPESELF_H
#define WBPESELF_H

#include "wbpe.h"

class WBPESelf : public WBPE
{
public:
	WBPESelf();
	virtual ~WBPESelf();

	DEFINE_WBPE_FACTORY( Self );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPESELF_H
