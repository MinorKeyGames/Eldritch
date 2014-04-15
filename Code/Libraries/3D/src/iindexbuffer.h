#ifndef IINDEXBUFFER_H
#define IINDEXBUFFER_H

#include "3d.h"

enum EPrimitiveType
{
	EPT_POINTLIST,
	EPT_LINELIST,
	EPT_LINESTRIP,
	EPT_TRIANGLELIST,
	EPT_TRIANGLESTRIP,
	EPT_TRIANGLEFAN,
};

class IIndexBuffer
{
protected:
	virtual ~IIndexBuffer() {}

public:
	virtual void	Init( uint NumIndices, index_t* Indices ) = 0;

	virtual void*	GetIndices() = 0;

	virtual uint	GetNumIndices() = 0;
	virtual void	SetNumIndices( uint NumIndices ) = 0;
	virtual uint	GetNumPrimitives() = 0;

	virtual void	SetPrimitiveType( EPrimitiveType PrimitiveType ) = 0;

	virtual int		AddReference() = 0;
	virtual int		Release() = 0;
};

#endif // IINDEXBUFFER_H