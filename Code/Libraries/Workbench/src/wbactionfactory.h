#ifndef WBACTIONFACTORY_H
#define WBACTIONFACTORY_H

#include "wbaction.h"
#include "array.h"

class SimpleString;

namespace WBActionFactory
{
	void		RegisterFactory( const HashedString& TypeName, WBActionFactoryFunc Factory );
	void		InitializeBaseFactories();
	void		ShutDown();

	WBAction*	Create( const SimpleString& DefinitionName );

	void		InitializeActionArray( const SimpleString& DefinitionName, Array<WBAction*>& OutActionArray );
};

#endif // WBACTIONFACTORY_H