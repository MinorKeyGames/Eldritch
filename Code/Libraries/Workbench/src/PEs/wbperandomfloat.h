#ifndef WBPERANDOMFLOAT_H
#define WBPERANDOMFLOAT_H

#include "wbpe.h"

class WBPERandomFloat : public WBPE
{
public:
	WBPERandomFloat();
	virtual ~WBPERandomFloat();

	DEFINE_WBPE_FACTORY( RandomFloat );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	float	m_ValueA;
	float	m_ValueB;
};

#endif // WBPECONSTANTFLOAT_H
