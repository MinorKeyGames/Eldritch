#ifndef WBPELOOKUP_H
#define WBPELOOKUP_H

#include "wbpe.h"

class WBPELookup : public WBPE
{
public:
	WBPELookup();
	virtual ~WBPELookup();

	DEFINE_WBPE_FACTORY( Lookup );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	HashedString	m_Key;

#if BUILD_DEV
	bool			m_WarnFailure;
#endif
};

#endif // WBPELOOKUP_H
