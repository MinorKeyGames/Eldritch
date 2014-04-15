#ifndef WBPECONSTANTSTRING_H
#define WBPECONSTANTSTRING_H

#include "wbpe.h"

class WBPEConstantString : public WBPE
{
public:
	WBPEConstantString();
	virtual ~WBPEConstantString();

	DEFINE_WBPE_FACTORY( ConstantString );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	SimpleString	m_Value;
};

#endif // WBPECONSTANTSTRING_H
