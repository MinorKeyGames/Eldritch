#ifndef WBPEUNARYOP_H
#define WBPEUNARYOP_H

#include "wbpe.h"

class WBPEUnaryOp : public WBPE
{
public:
	WBPEUnaryOp();
	virtual ~WBPEUnaryOp();

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

protected:
	WBPE*	m_Input;
};

#endif // WBPEUNARYOP_H
