#ifndef RODINBTNODECOMPOSITE_H
#define RODINBTNODECOMPOSITE_H

#include "rodinbtnode.h"
#include "array.h"

class RodinBTNodeComposite : public RodinBTNode
{
public:
	RodinBTNodeComposite();
	virtual ~RodinBTNodeComposite();

	virtual void InitializeFromDefinition( const SimpleString& DefinitionName );

protected:
	Array<RodinBTNode*>	m_Children;
};

#endif // RODINBTNODECOMPOSITE_H