#ifndef WBPERANDOMINT_H
#define WBPERANDOMINT_H

#include "wbpe.h"

class WBPERandomInt : public WBPE
{
public:
	WBPERandomInt();
	virtual ~WBPERandomInt();

	DEFINE_WBPE_FACTORY( RandomInt );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	int	m_ValueA;
	int	m_ValueB;
};

#endif // WBPERANDOMINT_H
