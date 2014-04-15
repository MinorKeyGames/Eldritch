#include "core.h"
#include "d3d9vertexshader.h"
#include "idatastream.h"
#include "memorystream.h"
#include "d3d9constanttable.h"

#include <d3d9.h>

D3D9VertexShader::D3D9VertexShader()
:	m_VertexShader( NULL )
,	m_ConstantTable()
{
}

D3D9VertexShader::~D3D9VertexShader()
{
	SafeRelease( m_VertexShader );
}

void D3D9VertexShader::Initialize( IDirect3DDevice9* const pD3DDevice, const IDataStream& Stream )
{
	ASSERT( pD3DDevice );

	const int Length	= Stream.Size();
	byte* pBuffer		= new byte[ Length ];
	Stream.Read( Length, pBuffer );

	// We're passing this along as a DWORD pointer, it better be aligned. If not, round up and memset it to 0 first.
	DEBUGASSERT( Length % sizeof( DWORD ) == 0 );

	const HRESULT hr = pD3DDevice->CreateVertexShader( reinterpret_cast<DWORD*>( pBuffer ), &m_VertexShader );
	ASSERT( m_VertexShader );
	ASSERT( hr == D3D_OK );
	Unused( hr );

	D3D9ConstantTable::ParseConstantTable( MemoryStream( pBuffer, Length ), m_ConstantTable ); 

	SafeDeleteArray( pBuffer );
}

/*virtual*/ bool D3D9VertexShader::GetRegister( const HashedString& Parameter, uint& Register ) const
{
	Map<HashedString, uint>::Iterator ConstantIter = m_ConstantTable.Search( Parameter );
	if( ConstantIter.IsValid() )
	{
		Register = ConstantIter.GetValue();
		return true;
	}
	else
	{
		return false;
	}
}