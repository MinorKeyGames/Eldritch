#ifndef WBPE_H
#define WBPE_H

#include "wbparamevaluator.h"
class SimpleString;

#define DEFINE_WBPE_FACTORY( type ) static class WBPE* Factory() { return new WBPE##type; }
typedef class WBPE* ( *WBPEFactoryFunc )( void );

class WBPE
{
public:
	WBPE();
	virtual ~WBPE();

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;
};

#endif // WBPE_H