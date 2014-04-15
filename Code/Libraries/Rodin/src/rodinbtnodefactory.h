#ifndef RODINBTNODEFACTORY_H
#define RODINBTNODEFACTORY_H

#include "rodinbtnode.h"

class SimpleString;
class WBCompRodinBehaviorTree;

namespace RodinBTNodeFactory
{
	void	RegisterFactory( const HashedString& TypeName, RodinBTNodeFactoryFunc Factory );
	void	InitializeBaseFactories();
	void	ShutDown();

	// Recursively construct tree from given node definition.
	RodinBTNode*	Create( const SimpleString& DefinitionName, WBCompRodinBehaviorTree* const pBehaviorTree );
};

#endif // RODINBTNODEFACTORY_H