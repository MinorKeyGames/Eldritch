#ifndef WBPARAMEVALUATORFACTORY_H
#define WBPARAMEVALUATORFACTORY_H

#include "wbpe.h"

class HashedString;
class SimpleString;

namespace WBParamEvaluatorFactory
{
	void	RegisterFactory( const HashedString& TypeName, WBPEFactoryFunc Factory );
	void	InitializeBaseFactories();
	void	ShutDown();

	// Build PE chain from node definition.
	WBPE*	Create( const SimpleString& DefinitionName );
};

#endif // WBPARAMEVALUATORFACTORY_H