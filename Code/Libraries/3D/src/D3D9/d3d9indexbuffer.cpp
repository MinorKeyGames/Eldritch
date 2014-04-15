#include "core.h"
#include "3d.h"
#include "D3D9/d3d9indexbuffer.h"

#include <memory.h>

#define MEMPOOL D3DPOOL_MANAGED	// Could use D3DPOOL_DEFAULT

// Maps my EPrimitiveType to D3D's D3DPRIMITIVETYPE
D3DPRIMITIVETYPE PrimitiveTypeTable[] =
{
	D3DPT_POINTLIST,
	D3DPT_LINELIST,
	D3DPT_LINESTRIP,
	D3DPT_TRIANGLELIST,
	D3DPT_TRIANGLESTRIP,
	D3DPT_TRIANGLEFAN,
};

D3D9IndexBuffer::D3D9IndexBuffer( IDirect3DDevice9* D3DDevice )
:	m_RefCount( 0 ),
	m_NumIndices( 0 ),
	m_Indices( NULL ),
	m_D3DDevice( D3DDevice ),
	m_PrimitiveType( D3DPT_TRIANGLELIST ) {}

D3D9IndexBuffer::~D3D9IndexBuffer()
{
	SafeRelease( m_Indices );
}

int D3D9IndexBuffer::AddReference()
{
	++m_RefCount;
	return m_RefCount;
}

int D3D9IndexBuffer::Release()
{
	DEVASSERT( m_RefCount > 0 );
	--m_RefCount;
	if( m_RefCount <= 0 )
	{
		delete this;
		return 0;
	}
	return m_RefCount;
}

void D3D9IndexBuffer::Init( uint NumIndices, index_t* Indices )
{
	XTRACE_FUNCTION;

	m_NumIndices = NumIndices;

	if( Indices )
	{
		void* TempIndices;
		m_D3DDevice->CreateIndexBuffer( sizeof( index_t ) * NumIndices, D3DUSAGE_WRITEONLY, D3DINDEXFORMAT, MEMPOOL, &m_Indices, NULL );
		m_Indices->Lock( 0, 0, (void**)&TempIndices, 0 );
		memcpy( TempIndices, Indices, sizeof( index_t ) * NumIndices );
		m_Indices->Unlock();
	}
}

void* D3D9IndexBuffer::GetIndices()
{
	return m_Indices;
}

uint D3D9IndexBuffer::GetNumIndices()
{
	return m_NumIndices;
}

void D3D9IndexBuffer::SetNumIndices( uint NumIndices )
{
	m_NumIndices = NumIndices;
}

uint D3D9IndexBuffer::GetNumPrimitives()
{
	switch( m_PrimitiveType )
	{
	case D3DPT_POINTLIST:
		return m_NumIndices;
	case D3DPT_LINELIST:
		return m_NumIndices >> 1;
	case D3DPT_LINESTRIP:
		return m_NumIndices - 1;
	case D3DPT_TRIANGLELIST:
		return m_NumIndices / 3;
	case D3DPT_TRIANGLESTRIP:
		return m_NumIndices - 2;
	case D3DPT_TRIANGLEFAN:
		return m_NumIndices - 2;
	default:
		return 0;
	}
}

void D3D9IndexBuffer::SetPrimitiveType( EPrimitiveType PrimitiveType )
{
	m_PrimitiveType = PrimitiveTypeTable[ PrimitiveType ];
}