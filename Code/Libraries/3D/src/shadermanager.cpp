#include "core.h"
#include "3d.h"
#include "shadermanager.h"
#include "irenderer.h"
#include "ivertexshader.h"
#include "ipixelshader.h"
#include "ishaderprogram.h"

ShaderManager::ShaderManager( IRenderer* Renderer )
:	m_ShaderPrograms()
,	m_VertexShaders()
,	m_PixelShaders()
,	m_Renderer( Renderer )
{
	ASSERT( m_Renderer );
}

ShaderManager::~ShaderManager()
{
	FreeShaders();
}

void ShaderManager::FreeShaders()
{
	FOR_EACH_MAP( ProgramIter, m_ShaderPrograms, SShaderProgramKey, IShaderProgram* )
	{
		SafeDelete( ProgramIter.GetValue() );
	}

	FOR_EACH_MAP( VSIter, m_VertexShaders, HashedString, IVertexShader* )
	{
		SafeDelete( VSIter.GetValue() );
	}

	FOR_EACH_MAP( PSIter, m_PixelShaders, HashedString, IPixelShader* )
	{
		SafeDelete( PSIter.GetValue() );
	}
}

IShaderProgram* ShaderManager::GetShaderProgram( const SimpleString& VertexShaderFilename, const SimpleString& PixelShaderFilename, IVertexDeclaration* const pVertexDeclaration )
{
	SShaderProgramKey Key;
	Key.m_VertexShaderHash = VertexShaderFilename;
	Key.m_PixelShaderHash = PixelShaderFilename;

	const Map<SShaderProgramKey, IShaderProgram*>::Iterator ProgramIter = m_ShaderPrograms.Search( Key );
	if( ProgramIter.IsValid() )
	{
		return ProgramIter.GetValue();
	}
	else
	{
		IVertexShader* const	pVertexShader = GetVertexShader( VertexShaderFilename );
		IPixelShader* const		pPixelShader = GetPixelShader( PixelShaderFilename );
		IShaderProgram* const	pShaderProgram = m_Renderer->CreateShaderProgram( pVertexShader, pPixelShader, pVertexDeclaration );
		ASSERT( pShaderProgram );

		m_ShaderPrograms.Insert( Key, pShaderProgram );

		return pShaderProgram;
	}
}

IVertexShader* ShaderManager::GetVertexShader( const SimpleString& Filename )
{
	const HashedString HashedFilename = Filename;
	const Map<HashedString, IVertexShader*>::Iterator VSIter = m_VertexShaders.Search( HashedFilename );
	if( VSIter.IsValid() )
	{
		return VSIter.GetValue();
	}
	else
	{
		IVertexShader* const pVertexShader = m_Renderer->CreateVertexShader( Filename );
		ASSERT( pVertexShader );

		m_VertexShaders.Insert( HashedFilename, pVertexShader );

		return pVertexShader;
	}
}

IPixelShader* ShaderManager::GetPixelShader( const SimpleString& Filename )
{
	const HashedString HashedFilename = Filename;
	const Map<HashedString, IPixelShader*>::Iterator PSIter = m_PixelShaders.Search( HashedFilename );
	if( PSIter.IsValid() )
	{
		return PSIter.GetValue();
	}
	else
	{
		IPixelShader* const pPixelShader = m_Renderer->CreatePixelShader( Filename );
		ASSERT( pPixelShader );

		m_PixelShaders.Insert( HashedFilename, pPixelShader );

		return pPixelShader;
	}
}