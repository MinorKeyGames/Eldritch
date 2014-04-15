#ifndef SDPFACTORY_H
#define SDPFACTORY_H

#include "shaderdataprovider.h"

class SimpleString;

namespace SDPFactory
{
	void	RegisterSDPFactory( const HashedString& TypeName, SDPFactoryFunc Factory );

	void	InitializeBaseFactories();
	void	ShutDown();

	// NOTE: SDPs work different than other factories; we want a singular instance, not a new one
	ShaderDataProvider*	GetSDPInstance( const HashedString& TypeName );
};

#endif // SDPFACTORY_H