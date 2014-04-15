#ifndef RENDERSTATES_H
#define RENDERSTATES_H

class HashedString;
class ITexture;

enum ECullMode
{
	ECM_Unknown,
	ECM_None,
	ECM_CW,
	ECM_CCW,
};

enum EZEnable
{
	EZE_Unknown,
	EZE_False,
	EZE_True,
};

enum EZWriteEnable
{
	EZWE_Unknown,
	EZWE_False,
	EZWE_True,
};

enum EAlphaBlendEnable
{
	EABE_Unknown,
	EABE_False,
	EABE_True,
};

enum EBlend
{
	EB_Unknown,
	EB_Zero,
	EB_One,
	EB_SrcColor,
	EB_InvSrcColor,
	EB_SrcAlpha,
	EB_InvSrcAlpha,
	EB_DestAlpha,
	EB_InvDestAlpha,
	EB_DestColor,
	EB_InvDestColor,
	EB_SrcAlphaSat,
};

enum ETextureAddress
{
	ETA_Unknown,
	ETA_Wrap,
	ETA_Mirror,
	ETA_Clamp,
};

enum ETextureFilter
{
	ETF_Unknown,
	ETF_None,		// For mipmapping
	ETF_Point,
	ETF_Linear,
	ETF_Anisotropic,
	ETF_SIZE
};

struct SRenderState
{
	SRenderState()
	:	m_CullMode( ECM_Unknown )
	,	m_ZEnable( EZE_Unknown )
	,	m_ZWriteEnable( EZWE_Unknown )
	,	m_AlphaBlendEnable( EABE_Unknown )
	,	m_SrcBlend( EB_Unknown )
	,	m_DestBlend( EB_Unknown )
	{
	}

	ECullMode			m_CullMode;
	EZEnable			m_ZEnable;
	EZWriteEnable		m_ZWriteEnable;
	EAlphaBlendEnable	m_AlphaBlendEnable;
	EBlend				m_SrcBlend;
	EBlend				m_DestBlend;
};

struct SSamplerState
{
	SSamplerState()
	:	m_Texture( NULL )
	,	m_AddressU( ETA_Unknown )
	,	m_AddressV( ETA_Unknown )
	,	m_MinFilter( ETF_Unknown )
	,	m_MagFilter( ETF_Unknown )
	,	m_MipFilter( ETF_Unknown )
	{
	}

	ITexture*		m_Texture;
	ETextureAddress	m_AddressU;
	ETextureAddress	m_AddressV;
	ETextureFilter	m_MinFilter;
	ETextureFilter	m_MagFilter;
	ETextureFilter	m_MipFilter;
};

namespace RenderStates
{
	ECullMode		GetCullMode( const HashedString& CullMode );
	EBlend			GetBlend( const HashedString& Blend );
	ETextureAddress	GetTextureAddress( const HashedString& TextureAddress );
	ETextureFilter	GetTextureFilter( const HashedString& TextureFilter );
}

#endif // RENDERSTATES_H