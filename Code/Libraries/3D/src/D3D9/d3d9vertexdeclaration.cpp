#include "core.h"
#include "3d.h"
#include "D3D9/d3d9vertexdeclaration.h"
#include "map.h"
#include "mathcore.h"

#include <d3d9.h>

D3D9VertexDeclaration::D3D9VertexDeclaration( IDirect3DDevice9* pD3DDevice )
:	m_D3DDevice( pD3DDevice )
,	m_VertexDeclaration( NULL )
,	m_VertexSignature( 0 ) {}

D3D9VertexDeclaration::~D3D9VertexDeclaration()
{
	SafeRelease( m_VertexDeclaration );
}

void D3D9VertexDeclaration::Initialize( uint VertexSignature )
{
	m_VertexSignature = VertexSignature;

	uint NumElements = CountBits( VertexSignature );

	D3DVERTEXELEMENT9* pVertexElements = new D3DVERTEXELEMENT9[ NumElements + 1 ];

	uint Index = 0;
	// "if( SIG == ... )" is done so that 0 is acceptable for SIG
#define CREATESTREAM( NUM, SIG, TYPE, USAGE, INDEX )					\
	if( SIG == ( VertexSignature & SIG ) )								\
	{																	\
		pVertexElements[ Index ].Stream		= (WORD)NUM;				\
		pVertexElements[ Index ].Offset		= 0;						\
		pVertexElements[ Index ].Type		= TYPE;						\
		pVertexElements[ Index ].Method		= D3DDECLMETHOD_DEFAULT;	\
		pVertexElements[ Index ].Usage		= USAGE;					\
		pVertexElements[ Index ].UsageIndex	= INDEX;					\
		Index++;														\
	}

	CREATESTREAM( Index, VD_POSITIONS, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION, 0 );
	CREATESTREAM( Index, VD_COLORS, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR, 0 );
#if USE_HDR
	CREATESTREAM( Index, VD_FLOATCOLORS, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_COLOR, 0 );
	CREATESTREAM( Index, VD_BASISCOLORS, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_COLOR, 1 );
	CREATESTREAM( Index, VD_BASISCOLORS, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_COLOR, 2 );

	// For SM2 cards, an alternative way to do HDR colors
	CREATESTREAM( Index, VD_FLOATCOLORS_SM2, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 1 );
	CREATESTREAM( Index, VD_BASISCOLORS_SM2, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2 );
	CREATESTREAM( Index, VD_BASISCOLORS_SM2, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 3 );
#endif
	CREATESTREAM( Index, VD_UVS, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0 );
	CREATESTREAM( Index, VD_NORMALS, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL, 0 );
	CREATESTREAM( Index, VD_TANGENTS, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TANGENT, 0 );
	CREATESTREAM( Index, VD_BONEINDICES, D3DDECLTYPE_UBYTE4, D3DDECLUSAGE_BLENDINDICES, 0 );
	CREATESTREAM( Index, VD_BONEWEIGHTS, D3DDECLTYPE_UBYTE4N, D3DDECLUSAGE_BLENDWEIGHT, 0 );
	CREATESTREAM( 0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0 );
#undef CREATESTREAM

	m_D3DDevice->CreateVertexDeclaration( pVertexElements, &m_VertexDeclaration );

	SafeDelete( pVertexElements );
}

void* D3D9VertexDeclaration::GetDeclaration()
{
	return m_VertexDeclaration;
}

uint D3D9VertexDeclaration::GetSignature()
{
	return m_VertexSignature;
}