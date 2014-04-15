#include "core.h"
#include "D3D9/d3d9rendertarget.h"
#include "D3D9/d3d9texture.h"
#include "irenderer.h"

/*static*/ D3DFORMAT D3D9RenderTarget::GetD3DFormat( const ERenderTargetFormat Format )
{
	switch( Format )
	{
	case ERTF_Unknown:
		return D3DFMT_UNKNOWN;
	case ERTF_X8R8G8B8:
		return D3DFMT_X8R8G8B8;
	case ERTF_A8R8G8B8:
		return D3DFMT_A8R8G8B8;
	case ERTF_A16B16G16R16:
		return D3DFMT_A16B16G16R16;
	case ERTF_A16B16G16R16F:
		return D3DFMT_A16B16G16R16F;
	case ERTF_A32B32G32R32F:
		return D3DFMT_A32B32G32R32F;
	case ERTF_R32F:
		return D3DFMT_R32F;
	case ERTF_D24S8:
		return D3DFMT_D24S8;
	default:
		WARNDESC( "D3D texture format not matched" );
		return D3DFMT_UNKNOWN;
	}
}

D3D9RenderTarget::D3D9RenderTarget( IRenderer* pRenderer, IDirect3DDevice9* D3DDevice )
:	m_Params()
,	m_Renderer( pRenderer )
,	m_D3DDevice( D3DDevice )
,	m_ColorTexture( NULL )
,	m_DepthStencilTexture( NULL )
,	m_ColorSurface( NULL )
,	m_DepthStencilSurface( NULL )
{
}

D3D9RenderTarget::D3D9RenderTarget( IRenderer* pRenderer, IDirect3DDevice9* D3DDevice, IDirect3DSurface9* ColorSurface, IDirect3DSurface9* DepthStencilSurface )
:	m_Params()
,	m_Renderer( pRenderer )
,	m_D3DDevice( D3DDevice )
,	m_ColorTexture( NULL )
,	m_DepthStencilTexture( NULL )
,	m_ColorSurface( ColorSurface )
,	m_DepthStencilSurface( DepthStencilSurface ) {}

D3D9RenderTarget::~D3D9RenderTarget()
{
	Release();
}

/*virtual*/ void D3D9RenderTarget::Initialize( const SRenderTargetParams& Params )
{
	m_Params = Params;

	BuildRenderTarget();
}

void D3D9RenderTarget::Release()
{
	SafeRelease( m_ColorSurface );
	SafeRelease( m_DepthStencilSurface );
	SafeDelete( m_ColorTexture );
	SafeDelete( m_DepthStencilTexture );
}

void D3D9RenderTarget::Reset()
{
	BuildRenderTarget();
}

void D3D9RenderTarget::Reset( IDirect3DSurface9* ColorSurface, IDirect3DSurface9* DepthStencilSurface )
{
	// No need to AddRef these; when called from D3D9Renderer::CreateDefaultRenderTarget, they have
	// already been implicitly AddRef-ed by the GetRenderTarget/GetDepthStencilSurface calls.

	m_ColorSurface = ColorSurface;
	m_DepthStencilSurface = DepthStencilSurface;
}

void D3D9RenderTarget::BuildRenderTarget()
{
	XTRACE_FUNCTION;

	ASSERT( m_Params.Width );
	ASSERT( m_Params.Height );

	STATIC_HASHED_STRING( Render );
	CATPRINTF( sRender, 1, "Building render target...\n" );

	uint ColorUsage = D3DUSAGE_RENDERTARGET;
	if( m_Params.AutoGenMipMaps )
	{
		ColorUsage |= D3DUSAGE_AUTOGENMIPMAP;
	}

	uint DepthStencilUsage = D3DUSAGE_DEPTHSTENCIL;

	if( m_Params.ColorFormat != ERTF_None )
	{
		CATPRINTF( sRender, 1, "Trying to create color texture with format %d\n", m_Params.ColorFormat );

		const ERenderTargetFormat SupportedFormat = m_Renderer->GetBestSupportedRenderTargetFormat( m_Params.ColorFormat );
		CATPRINTF( sRender, 1, "Creating color texture with format %d...\n", SupportedFormat );

		IDirect3DTexture9* ColorTexture;
		HRESULT hr = m_D3DDevice->CreateTexture(
			m_Params.Width,
			m_Params.Height,
			1,
			ColorUsage,
			GetD3DFormat( SupportedFormat ),
			D3DPOOL_DEFAULT,
			&ColorTexture,
			NULL );
		ASSERT( hr == D3D_OK );
		m_ColorTexture = new D3D9Texture( m_D3DDevice, ColorTexture );
		ColorTexture->GetSurfaceLevel( 0, &m_ColorSurface );
	}

	if( m_Params.DepthStencilFormat != ERTF_None )
	{
		if( m_Params.DepthStencilFormat != ERTF_UseDefault )
		{
			CATPRINTF( sRender, 1, "Trying to create depth/stencil texture with format %d\n", m_Params.DepthStencilFormat );

			const ERenderTargetFormat SupportedFormat = m_Renderer->GetBestSupportedRenderTargetFormat( m_Params.DepthStencilFormat );
			CATPRINTF( sRender, 1, "Creating depth/stencil texture with format %d...\n", SupportedFormat );

			IDirect3DTexture9* DepthStencilTexture;
			HRESULT hr = m_D3DDevice->CreateTexture(
				m_Params.Width,
				m_Params.Height,
				1,
				DepthStencilUsage,
				GetD3DFormat( SupportedFormat ),
				D3DPOOL_DEFAULT,
				&DepthStencilTexture, NULL );
			ASSERT( hr == D3D_OK );
			m_DepthStencilTexture = new D3D9Texture( m_D3DDevice, DepthStencilTexture );
			DepthStencilTexture->GetSurfaceLevel( 0, &m_DepthStencilSurface );
		}
		else
		{
			m_DepthStencilSurface = ( IDirect3DSurface9* )( m_Renderer->GetDefaultRenderTarget()->GetDepthStencilRenderTargetHandle() );
			m_DepthStencilSurface->AddRef();
		}
	}

	CATPRINTF( sRender, 1, "Render target built.\n" );
}

void* D3D9RenderTarget::GetHandle()
{
	// Not used by D3D
	return NULL;
}

void* D3D9RenderTarget::GetColorRenderTargetHandle()
{
	return m_ColorSurface;
}

void* D3D9RenderTarget::GetDepthStencilRenderTargetHandle()
{
	return m_DepthStencilSurface;
}

ITexture* D3D9RenderTarget::GetColorTextureHandle()
{
	return m_ColorTexture;
}

ITexture* D3D9RenderTarget::GetDepthStencilTextureHandle()
{
	return m_DepthStencilTexture;
}