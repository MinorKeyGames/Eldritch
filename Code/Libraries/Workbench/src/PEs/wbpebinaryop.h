#ifndef WBPEBINARYOP_H
#define WBPEBINARYOP_H

#include "wbpe.h"

class WBPEBinaryOp : public WBPE
{
public:
	WBPEBinaryOp();
	virtual ~WBPEBinaryOp();

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

protected:
	WBPE*	m_InputA;
	WBPE*	m_InputB;
};

#endif // WBPEBINARYOP_H
