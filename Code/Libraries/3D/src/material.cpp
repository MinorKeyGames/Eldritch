#include "core.h"
#include "material.h"
#include "configmanager.h"
#include "irenderer.h"
#include "shadermanager.h"
#include "sdpfactory.h"

Material::Material()
:	m_ShaderProgram( NULL )
,	m_SDP( NULL )
,	m_Flags( 0 )
,	m_RenderState()
,	m_SamplerStates()
,	m_NumSamplers( 0 )
{
}

Material::~Material()
{
}

void Material::SetDefinition( const SimpleString& DefinitionName, IRenderer* const pRenderer, IVertexDeclaration* const pVertexDeclaration )
{
	DEBUGASSERT( pRenderer );

	MAKEHASH( DefinitionName );

	STATICHASH( CullMode );
	STATIC_HASHED_STRING( CW );
	const HashedString CullMode = ConfigManager::GetHash( sCullMode, sCW, sDefinitionName );
	m_RenderState.m_CullMode = RenderStates::GetCullMode( CullMode );

	STATICHASH( ZEnable );
	const bool ZEnable = ConfigManager::GetBool( sZEnable, true, sDefinitionName );
	m_RenderState.m_ZEnable = ZEnable ? EZE_True : EZE_False;

	STATICHASH( ZWriteEnable );
	const bool ZWriteEnable = ConfigManager::GetBool( sZWriteEnable, true, sDefinitionName );
	m_RenderState.m_ZWriteEnable = ZWriteEnable ? EZWE_True : EZWE_False;

	STATICHASH( AlphaBlendEnable );
	const bool AlphaBlendEnable = ConfigManager::GetBool( sAlphaBlendEnable, false, sDefinitionName );
	m_RenderState.m_AlphaBlendEnable = AlphaBlendEnable ? EABE_True : EABE_False;

	STATICHASH( SrcBlend );
	STATIC_HASHED_STRING( One );
	const HashedString SrcBlend = ConfigManager::GetHash( sSrcBlend, sOne, sDefinitionName );
	m_RenderState.m_SrcBlend = RenderStates::GetBlend( SrcBlend );

	STATICHASH( DestBlend );
	STATIC_HASHED_STRING( Zero );
	const HashedString DestBlend = ConfigManager::GetHash( sDestBlend, sZero, sDefinitionName );
	m_RenderState.m_DestBlend = RenderStates::GetBlend( DestBlend );

	STATICHASH( SDP );
	STATIC_HASHED_STRING( Base );
	const HashedString SDP = ConfigManager::GetHash( sSDP, sBase, sDefinitionName );
	m_SDP = SDPFactory::GetSDPInstance( SDP );
	DEVASSERT( m_SDP );

	ShaderManager* const pShaderManager = pRenderer->GetShaderManager();
	DEBUGASSERT( pShaderManager );

	STATICHASH( VertexShader );
	const SimpleString VertexShader = ConfigManager::GetString( sVertexShader, "", sDefinitionName );

	STATICHASH( PixelShader );
	const SimpleString PixelShader = ConfigManager::GetString( sPixelShader, "", sDefinitionName );

	const SimpleString ShaderType = pRenderer->GetShaderType();
	MAKEHASH( ShaderType );

	MAKEHASHFROM( VertexShaderDef, VertexShader );
	const SimpleString VertexShaderVersion = ConfigManager::GetString( sShaderType, "", sVertexShaderDef );

	MAKEHASHFROM( PixelShaderDef, PixelShader );
	const SimpleString PixelShaderVersion = ConfigManager::GetString( sShaderType, "", sPixelShaderDef );

	m_ShaderProgram = pShaderManager->GetShaderProgram( VertexShaderVersion, PixelShaderVersion, pVertexDeclaration );
	DEVASSERT( m_ShaderProgram );

	STATICHASH( NumSamplers );
	m_NumSamplers = ConfigManager::GetInt( sNumSamplers, 0, sDefinitionName );
	ASSERT( m_NumSamplers <= MAX_TEXTURE_STAGES );
	for( uint SamplerIndex = 0; SamplerIndex < m_NumSamplers; ++SamplerIndex )
	{
		const SimpleString SamplerDefinitionName = ConfigManager::GetSequenceString( "Sampler%d", SamplerIndex, "", sDefinitionName );
		SetSamplerDefinition( SamplerIndex, SamplerDefinitionName );
	}
}

void Material::SetSamplerDefinition( const uint SamplerStage, const SimpleString& SamplerDefinitionName )
{
	DEVASSERT( SamplerStage < MAX_TEXTURE_STAGES );

	MAKEHASH( SamplerDefinitionName );

	SSamplerState& SamplerState = m_SamplerStates[ SamplerStage ];

	STATICHASH( AddressU );
	STATIC_HASHED_STRING( Wrap );
	const HashedString AddressU = ConfigManager::GetHash( sAddressU, sWrap, sSamplerDefinitionName );
	SamplerState.m_AddressU = RenderStates::GetTextureAddress( AddressU );

	STATICHASH( AddressV );
	const HashedString AddressV = ConfigManager::GetHash( sAddressV, sWrap, sSamplerDefinitionName );
	SamplerState.m_AddressV = RenderStates::GetTextureAddress( AddressV );

	STATICHASH( MinFilter );
	STATIC_HASHED_STRING( Point );
	const HashedString MinFilter = ConfigManager::GetHash( sMinFilter, sPoint, sSamplerDefinitionName );
	SamplerState.m_MinFilter = RenderStates::GetTextureFilter( MinFilter );

	STATICHASH( MagFilter );
	const HashedString MagFilter = ConfigManager::GetHash( sMagFilter, sPoint, sSamplerDefinitionName );
	SamplerState.m_MagFilter = RenderStates::GetTextureFilter( MagFilter );

	STATICHASH( MipFilter );
	STATIC_HASHED_STRING( None );
	const HashedString MipFilter = ConfigManager::GetHash( sMipFilter, sNone, sSamplerDefinitionName );
	SamplerState.m_MipFilter = RenderStates::GetTextureFilter( MipFilter );
}

void Material::SetTexture( const uint SamplerStage, ITexture* const pTexture )
{
	DEBUGASSERT( SamplerStage < MAX_TEXTURE_STAGES );

	SSamplerState& SamplerState = m_SamplerStates[ SamplerStage ];
	SamplerState.m_Texture = pTexture;
}

ITexture* Material::GetTexture( const uint SamplerStage ) const
{
	DEBUGASSERT( SamplerStage < m_NumSamplers );
	DEBUGASSERT( SamplerStage < MAX_TEXTURE_STAGES );

	const SSamplerState&	SamplerState	= m_SamplerStates[ SamplerStage ];
	ITexture* const			pTexture		= SamplerState.m_Texture;
	return pTexture;
}

const SSamplerState& Material::GetSamplerState( const uint SamplerStage ) const
{
	DEBUGASSERT( SamplerStage < m_NumSamplers );
	DEBUGASSERT( SamplerStage < MAX_TEXTURE_STAGES );

	return m_SamplerStates[ SamplerStage ];
}

void Material::SetFlags( const uint Flags, const uint Mask /*= MAT_ALL*/ )
{
	m_Flags &= ~Mask;
	m_Flags |= Flags;
}

void Material::SetFlag( const uint Flag, const bool Set )
{
	if( Set )
	{
		m_Flags |= Flag;
	}
	else
	{
		m_Flags &= ~Flag;
	}
}