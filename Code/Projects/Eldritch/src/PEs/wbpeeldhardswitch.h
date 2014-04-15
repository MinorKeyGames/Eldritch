#ifndef WBPEELDHARDSWITCH_H
#define WBPEELDHARDSWITCH_H

#include "wbpe.h"

class WBPEEldHardSwitch : public WBPE
{
public:
	WBPEEldHardSwitch();
	virtual ~WBPEEldHardSwitch();

	DEFINE_WBPE_FACTORY( EldHardSwitch );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	WBPE*	m_NormalInput;
	WBPE*	m_HardInput;
};

#endif // WBPEELDHARDSWITCH_H
