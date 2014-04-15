#include "core.h"
#include "3d.h"
#include "targetmanager.h"
#include "irendertarget.h"
#include "configmanager.h"
#include "irenderer.h"

TargetManager::TargetManager( IRenderer* pRenderer, uint Width, uint Height )
:	m_Renderer( pRenderer )
,	m_OriginalRenderTarget( NULL )
,	m_DepthRenderTarget( NULL )
,	m_DepthAlphaRenderTarget( NULL )
,	m_PrimaryRenderTarget( NULL )
,	m_BloomSmallTarget1( NULL )
,	m_BloomSmallTarget2( NULL )
,	m_BloomWidth( 0 )
,	m_BloomHeight( 0 )
{
	CreateTargets( Width, Height );
}

TargetManager::~TargetManager()
{
	ReleaseTargets();
}

void TargetManager::CreateTargets( uint Width, uint Height )
{
	STATIC_HASHED_STRING( Render );
	CATPRINTF( sRender, 1, "Creating render targets.\n" );

	m_OriginalRenderTarget = m_Renderer->GetDefaultRenderTarget();
	m_DepthRenderTarget = m_Renderer->CreateRenderTarget( SRenderTargetParams( Width, Height, ERTF_R32F, ERTF_UseDefault ) );
	m_DepthAlphaRenderTarget = m_Renderer->CreateRenderTarget( SRenderTargetParams( Width, Height, ERTF_A16B16G16R16F, ERTF_UseDefault ) );
#if USE_HDR
	STATICHASH( UseHDR );
	STATICHASH( UseBloom );
	STATICHASH( BloomTargetSizeShift );
	if( ConfigManager::GetBool( sUseHDR ) )
	{
		m_PrimaryRenderTarget = m_Renderer->CreateRenderTarget( SRenderTargetParams( Width, Height, ERTF_A16B16G16R16F, ERTF_UseDefault, true ) );
		if( ConfigManager::GetBool( sUseBloom ) )
		{
			int BloomTargetSizeShift = ConfigManager::GetInt( sBloomTargetSizeShift );
			ASSERT( BloomTargetSizeShift >= 0 );
			m_BloomWidth = Width >> BloomTargetSizeShift;
			m_BloomHeight = Height >> BloomTargetSizeShift;
			ASSERT( m_BloomWidth > 0 );
			ASSERT( m_BloomHeight > 0 );
			m_BloomSmallTarget1 = m_Renderer->CreateRenderTarget( SRenderTargetParams( m_BloomWidth, m_BloomHeight, ERTF_A16B16G16R16F, ERTF_None ) );
			m_BloomSmallTarget2 = m_Renderer->CreateRenderTarget( SRenderTargetParams( m_BloomWidth, m_BloomHeight, ERTF_A16B16G16R16F, ERTF_None ) );
		}
	}
	else
#endif
	{
		m_PrimaryRenderTarget = m_Renderer->CreateRenderTarget( SRenderTargetParams( Width, Height, ERTF_A8R8G8B8, ERTF_UseDefault ) );
	}

	CATPRINTF( sRender, 1, "Render targets created.\n" );
}

void TargetManager::ReleaseTargets()
{
	m_Renderer->FreeRenderTargets();
	SafeDelete( m_DepthRenderTarget );
	SafeDelete( m_DepthAlphaRenderTarget );
	SafeDelete( m_PrimaryRenderTarget );
	SafeDelete( m_BloomSmallTarget1 );
	SafeDelete( m_BloomSmallTarget2 );
}

void TargetManager::GetBloomDimensions( uint& Width, uint& Height )
{
	Width = m_BloomWidth;
	Height = m_BloomHeight;
}