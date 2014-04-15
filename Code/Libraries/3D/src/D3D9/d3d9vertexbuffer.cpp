#include "core.h"
#include "3d.h"
#include "D3D9/d3d9vertexbuffer.h"
#include "vector.h"
#include "vector4.h"
#include "vector2.h"

#include <d3d9.h>
#include <memory.h>

#define MEMPOOL D3DPOOL_MANAGED	// Could use D3DPOOL_DEFAULT

D3D9VertexBuffer::D3D9VertexBuffer( IDirect3DDevice9* D3DDevice )
:	m_RefCount( 0 )
,	m_NumVertices( 0 )
,	m_Positions( NULL )
,	m_Colors( NULL )
#if USE_HDR
,	m_FloatColors1( NULL )
,	m_FloatColors2( NULL )
,	m_FloatColors3( NULL )
#endif
,	m_UVs( NULL )
,	m_Normals( NULL )
,	m_Tangents( NULL )
,	m_BoneIndices( NULL )
,	m_BoneWeights( NULL )
,	m_D3DDevice( D3DDevice )
,	m_Dynamic( false )
{
}

D3D9VertexBuffer::~D3D9VertexBuffer()
{
	DeviceRelease();
}

int D3D9VertexBuffer::AddReference()
{
	++m_RefCount;
	return m_RefCount;
}

int D3D9VertexBuffer::Release()
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

void D3D9VertexBuffer::DeviceRelease()
{
	SafeRelease( m_Positions );
	SafeRelease( m_Colors );
#if USE_HDR
	SafeRelease( m_FloatColors1 );
	SafeRelease( m_FloatColors2 );
	SafeRelease( m_FloatColors3 );
#endif
	SafeRelease( m_UVs );
	SafeRelease( m_Normals );
	SafeRelease( m_Tangents );
	SafeRelease( m_BoneIndices );
	SafeRelease( m_BoneWeights );
}

void D3D9VertexBuffer::DeviceReset()
{
	FOR_EACH_LIST( CallbackIter, m_DeviceResetCallbacks, SDeviceResetCallback )
	{
		SDeviceResetCallback Callback = *CallbackIter;
		Callback.m_Callback( Callback.m_Void, this );
	}
}

void D3D9VertexBuffer::RegisterDeviceResetCallback( const SDeviceResetCallback& Callback )
{
	m_DeviceResetCallbacks.PushBack( Callback );
}

void D3D9VertexBuffer::Init( const SInit& InitStruct )
{
	XTRACE_FUNCTION;

	m_NumVertices = InitStruct.NumVertices;

	m_Dynamic = InitStruct.Dynamic;

	DWORD Usage;
	D3DPOOL MemoryPool;
	if( m_Dynamic )
	{
		Usage		= D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
		MemoryPool	= D3DPOOL_DEFAULT;
	}
	else
	{
		Usage		= D3DUSAGE_WRITEONLY;
		MemoryPool	= MEMPOOL;
	}

#define CREATEBUFFER( USE, TYPE )																							\
	if( InitStruct.USE )																									\
	{																														\
		void* Temp##USE;																									\
		m_D3DDevice->CreateVertexBuffer( sizeof( TYPE ) * InitStruct.NumVertices, Usage, 0, MemoryPool, &m_##USE, NULL );	\
		m_##USE->Lock( 0, 0, &Temp##USE, 0 );																				\
		memcpy( Temp##USE, InitStruct.USE, sizeof( TYPE ) * InitStruct.NumVertices );										\
		m_##USE->Unlock();																									\
	}

	CREATEBUFFER( Positions, Vector );
	CREATEBUFFER( Colors, uint );
#if USE_HDR
	CREATEBUFFER( FloatColors1, Vector4 );
	CREATEBUFFER( FloatColors2, Vector4 );
	CREATEBUFFER( FloatColors3, Vector4 );
#endif
	CREATEBUFFER( UVs, Vector2 );
	CREATEBUFFER( Normals, Vector );
	CREATEBUFFER( Tangents, Vector4 );
	CREATEBUFFER( BoneIndices, SBoneData );
	CREATEBUFFER( BoneWeights, SBoneData );
#undef CREATEBUFFER
}

void* D3D9VertexBuffer::GetPositions()
{
	return m_Positions;
}

void* D3D9VertexBuffer::GetColors()
{
	return m_Colors;
}

#if USE_HDR
void* D3D9VertexBuffer::GetFloatColors1()
{
	return m_FloatColors1;
}

void* D3D9VertexBuffer::GetFloatColors2()
{
	return m_FloatColors2;
}

void* D3D9VertexBuffer::GetFloatColors3()
{
	return m_FloatColors3;
}
#endif

void* D3D9VertexBuffer::GetUVs()
{
	return m_UVs;
}

void* D3D9VertexBuffer::GetNormals()
{
	return m_Normals;
}

void* D3D9VertexBuffer::GetTangents()
{
	return m_Tangents;
}

void* D3D9VertexBuffer::GetBoneIndices()
{
	return m_BoneIndices;
}

void* D3D9VertexBuffer::GetBoneWeights()
{
	return m_BoneWeights;
}

uint D3D9VertexBuffer::GetNumVertices()
{
	return m_NumVertices;
}

void D3D9VertexBuffer::SetNumVertices( uint NumVertices )
{
	m_NumVertices = NumVertices;
}

void* D3D9VertexBuffer::Lock( IVertexBuffer::EVertexElements VertexType )
{
	ASSERT( m_Dynamic );

	IDirect3DVertexBuffer9* const	pVertexBuffer	= InternalGetBuffer( VertexType );
	void*							pRetVal			= NULL;

	ASSERT( pVertexBuffer );

	HRESULT hr = pVertexBuffer->Lock( 0, 0, &pRetVal, D3DLOCK_DISCARD );
	ASSERT( hr == D3D_OK );
	Unused( hr );

	return pRetVal;
}

void D3D9VertexBuffer::Unlock( EVertexElements VertexType )
{
	ASSERT( m_Dynamic );

	IDirect3DVertexBuffer9* const	pVertexBuffer	= InternalGetBuffer( VertexType );

	ASSERT( pVertexBuffer );

	HRESULT hr = pVertexBuffer->Unlock();
	ASSERT( hr == D3D_OK );
	Unused( hr );
}

IDirect3DVertexBuffer9* D3D9VertexBuffer::InternalGetBuffer( EVertexElements VertexType )
{
	switch( VertexType )
	{
	case EVE_Positions:
		return m_Positions;
	case EVE_Colors:
		return m_Colors;
#if USE_HDR
	case EVE_FloatColors1:
		return m_FloatColors1;
	case EVE_FloatColors2:
		return m_FloatColors2;
	case EVE_FloatColors3:
		return m_FloatColors3;
#endif
	case EVE_UVs:
		return m_UVs;
	case EVE_Normals:
		return m_Normals;
	case EVE_Tangents:
		return m_Tangents;
	case EVE_BoneIndices:
		return m_BoneIndices;
	case EVE_BoneWeights:
		return m_BoneWeights;
	default:
		WARN;
		return NULL;
	}
}