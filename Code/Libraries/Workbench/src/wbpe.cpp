#include "core.h"
#include "wbpe.h"
#include "simplestring.h"

WBPE::WBPE()
{
}

WBPE::~WBPE()
{
}

void WBPE::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Unused( DefinitionName );
}

void WBPE::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );
	Unused( EvaluatedParam );
}