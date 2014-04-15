#ifndef D3D9INDEXBUFFER_H
#define D3D9INDEXBUFFER_H

#include "iindexbuffer.h"
#include <d3d9.h>

#if USE_LONG_INDICES
#define D3DINDEXFORMAT D3DFMT_INDEX32
#else
#define D3DINDEXFORMAT D3DFMT_INDEX16
#endif

class D3D9IndexBuffer : public IIndexBuffer
{
protected:
	virtual ~D3D9IndexBuffer();

public:
	D3D9IndexBuffer( IDirect3DDevice9* D3DDevice );

	virtual void	Init( uint NumIndices, index_t* Indices );

	virtual void*	GetIndices();

	virtual uint	GetNumIndices();
	virtual void	SetNumIndices( uint NumIndices );
	virtual uint	GetNumPrimitives();

	virtual void	SetPrimitiveType( EPrimitiveType PrimitiveType );

	virtual int		AddReference();
	virtual int		Release();

	D3DPRIMITIVETYPE				GetPrimitiveType() { return m_PrimitiveType; }

private:
	int								m_RefCount;
	uint							m_NumIndices;
	IDirect3DIndexBuffer9*			m_Indices;
	IDirect3DDevice9*				m_D3DDevice;
	D3DPRIMITIVETYPE				m_PrimitiveType;
};

#endif // D3D9INDEXBUFFER_H