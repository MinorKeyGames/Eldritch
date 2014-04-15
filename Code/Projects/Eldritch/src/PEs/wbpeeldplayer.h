#ifndef WBPEELDPLAYER_H
#define WBPEELDPLAYER_H

#include "wbpe.h"

class WBPEEldPlayer : public WBPE
{
public:
	WBPEEldPlayer();
	virtual ~WBPEEldPlayer();

	DEFINE_WBPE_FACTORY( EldPlayer );

	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPEELDGETITEM_H
