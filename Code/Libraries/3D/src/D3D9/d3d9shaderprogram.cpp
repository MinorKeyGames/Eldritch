#include "core.h"
#include "d3d9shaderprogram.h"
#include "ivertexshader.h"
#include "ipixelshader.h"

D3D9ShaderProgram::D3D9ShaderProgram()
:	m_VertexShader( NULL )
,	m_PixelShader( NULL )
{
}

D3D9ShaderProgram::~D3D9ShaderProgram()
{
}

/*virtual*/ void D3D9ShaderProgram::Initialize( IVertexShader* const pVertexShader, IPixelShader* const pPixelShader, IVertexDeclaration* const pVertexDeclaration )
{
	Unused( pVertexDeclaration );
	m_VertexShader	= pVertexShader;
	m_PixelShader	= pPixelShader;
}

/*virtual*/ bool D3D9ShaderProgram::GetVertexShaderRegister( const HashedString& Parameter, uint& Register ) const
{
	DEBUGASSERT( m_VertexShader );
	return m_VertexShader->GetRegister( Parameter, Register );
}

/*virtual*/ bool D3D9ShaderProgram::GetPixelShaderRegister( const HashedString& Parameter, uint& Register ) const
{
	DEBUGASSERT( m_PixelShader );
	return m_PixelShader->GetRegister( Parameter, Register );
}