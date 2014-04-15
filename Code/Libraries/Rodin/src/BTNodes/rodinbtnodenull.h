#ifndef RODINBTNODENULL_H
#define RODINBTNODENULL_H

#include "rodinbtnode.h"

class RodinBTNodeNull : public RodinBTNode
{
public:
	RodinBTNodeNull();
	virtual ~RodinBTNodeNull();

	DEFINE_RODINBTNODE_FACTORY( Null );

	virtual ETickStatus Tick( float DeltaTime );
};

#endif // RODINBTNODENULL_H