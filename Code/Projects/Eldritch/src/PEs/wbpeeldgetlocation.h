#ifndef WBPEELDGETLOCATION_H
#define WBPEELDGETLOCATION_H

#include "wbpe.h"

class WBPEEldGetLocation : public WBPE
{
public:
	WBPEEldGetLocation();
	virtual ~WBPEEldGetLocation();

	DEFINE_WBPE_FACTORY( EldGetLocation );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	WBPE*	m_EntityPE;
};

#endif // WBPEELDGETLOCATION_H
