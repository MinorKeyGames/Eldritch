#include "core.h"
#include "3d.h"
#include "D3D9/d3d9renderer.h"
#include "D3D9/d3d9vertexbuffer.h"
#include "D3D9/d3d9indexbuffer.h"
#include "D3D9/d3d9vertexdeclaration.h"
#include "D3D9/d3d9texture.h"
#include "D3D9/d3d9rendertarget.h"
#include "d3d9vertexshader.h"
#include "d3d9pixelshader.h"
#include "d3d9shaderprogram.h"
#include "vector.h"
#include "vector4.h"
#include "vector2.h"
#include "matrix.h"
#include "mesh.h"
#include "shadermanager.h"
#include "texturemanager.h"
#include "vertexdeclarationmanager.h"
#include "meshfactory.h"
#include "bucket.h"
#include "view.h"
#include "font.h"
#include "configmanager.h"
#include "frustum.h"
#include "packstream.h"
#include "simplestring.h"
#include "consolemanager.h"
#include "viewportpass.h"

#include <d3d9.h>

#if !BUILD_STEAM
#include <d3dx9.h>	// Included only for saving screenshots now!
#endif

D3DMULTISAMPLE_TYPE GetD3DMultiSampleType( EMultiSampleType MultiSampleType )
{
	switch( MultiSampleType )
	{
	case EMST_None:
		return D3DMULTISAMPLE_NONE;
	case EMST_2X:
		return D3DMULTISAMPLE_2_SAMPLES;
	case EMST_4X:
		return D3DMULTISAMPLE_4_SAMPLES;
	case EMST_8X:
		return D3DMULTISAMPLE_8_SAMPLES;
	default:
		WARNDESC( "D3D multisample type not matched" );
		return D3DMULTISAMPLE_NONE;
	}
}

D3D9Renderer::D3D9Renderer( HWND hWnd, bool Fullscreen )
:	m_D3D( NULL )
,	m_D3DDevice( NULL )
,	m_DeviceLost( false )
,	m_RestoreDeviceCallback()
,	m_hWnd( hWnd )
,	m_Fullscreen( Fullscreen )
,	m_MultiSampleType( EMST_None )
{
}

void D3D9Renderer::Initialize()
{
	XTRACE_FUNCTION;

	RendererCommon::Initialize();

	STATIC_HASHED_STRING( Render );
	CATPRINTF( sRender, 1, "Creating Direct3D system...\n" );

	ASSERT( !m_D3D );
	m_D3D = Direct3DCreate9( D3D_SDK_VERSION );
	ASSERT( m_D3D );

	DEBUGPRINTF( "Initializing Direct3D...\n" );

	STATICHASH( Renderer );
	STATICHASH( TestCaps );
	const bool TestCaps = ConfigManager::GetBool( sTestCaps, false, sRenderer );
	if( TestCaps )
	{
		TestCapabilities();
	}

	D3DPRESENT_PARAMETERS D3DParams;
	GetPresentParams( D3DParams );

	CATPRINTF( sRender, 1, "Creating Direct3D device...\n" );

	HRESULT Result = 0;
#ifdef USE_NVPERFHUD
	Result = m_D3D->CreateDevice( m_D3D->GetAdapterCount() - 1, D3DDEVTYPE_REF, m_hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &D3DParams, &m_D3DDevice );
#else
	Result = m_D3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_FPU_PRESERVE | D3DCREATE_HARDWARE_VERTEXPROCESSING, &D3DParams, &m_D3DDevice );

	if( Result != D3D_OK )
	{
		CATPRINTF( sRender, 1, "CreateDevice returned 0x%08X, trying again with software processing\n", Result );
		Result = m_D3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_FPU_PRESERVE | D3DCREATE_SOFTWARE_VERTEXPROCESSING, &D3DParams, &m_D3DDevice );
	}
#endif
	CATPRINTF( sRender, 1, "CreateDevice returned 0x%08X\n", Result );
	ASSERT( Result == D3D_OK );

	CATPRINTF( sRender, 1, "Device created.\n" );
	CATPRINTF( sRender, 1, "Creating default render target...\n" );

	CreateDefaultRenderTarget();

	CATPRINTF( sRender, 1, "Default render target created.\n" );
	CATPRINTF( sRender, 1, "Initializing render state.\n" );

	// I never want to use built-in lighting. All other render state should be driven by materials.
	{
		const HRESULT Result = m_D3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
		DEBUGASSERT( Result == D3D_OK );
		Unused( Result );
	}

	CATPRINTF( sRender, 1, "Render state initialized.\n" );
	CATPRINTF( sRender, 1, "Direct3D initialized.\n" );
}

D3D9Renderer::~D3D9Renderer()
{
	DEBUGPRINTF( "Shutting down Direct3D...\n" );

	SafeRelease( m_D3DDevice );
	SafeRelease( m_D3D );
}

bool D3D9Renderer::SupportsSM3()
{
	D3DCAPS9 Caps;
	m_D3DDevice->GetDeviceCaps( &Caps );
	return(	Caps.PixelShaderVersion >= D3DPS_VERSION( 3, 0 ) &&
			Caps.VertexShaderVersion >= D3DVS_VERSION( 3, 0 ) );
}

bool D3D9Renderer::SupportsSM2()
{
	D3DCAPS9 Caps;
	m_D3DDevice->GetDeviceCaps( &Caps );
	return(	Caps.PixelShaderVersion >= D3DPS_VERSION( 2, 0 ) &&
		Caps.VertexShaderVersion >= D3DVS_VERSION( 2, 0 ) );
}

/*virtual*/ ERenderTargetFormat D3D9Renderer::GetBestSupportedRenderTargetFormat( const ERenderTargetFormat Format ) const
{
	ERenderTargetFormat ReturnFormat = Format;

#define CHECK_AND_DEMOTE( fmt, dmt ) \
	if( ReturnFormat == fmt ) \
	{ \
		const D3DFORMAT D3DFormat = D3D9RenderTarget::GetD3DFormat( ReturnFormat ); \
		const HRESULT hr = m_D3D->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFormat ); \
		if( hr != D3D_OK ) \
		{ \
			ReturnFormat = dmt; \
		} \
	}

	CHECK_AND_DEMOTE( ERTF_A32B32G32R32F, ERTF_A16B16G16R16F );
	CHECK_AND_DEMOTE( ERTF_A16B16G16R16F, ERTF_A16B16G16R16 );
	CHECK_AND_DEMOTE( ERTF_A16B16G16R16, ERTF_A8R8G8B8 );

#undef CHECK_AND_DEMOTE

	return ReturnFormat;
}

void D3D9Renderer::SetFullScreen( bool FullScreen )
{
	m_Fullscreen = FullScreen;
}

void D3D9Renderer::SetMultiSampleType( EMultiSampleType MultiSampleType )
{
	m_MultiSampleType = MultiSampleType;
}

void D3D9Renderer::GetBestSupportedMultiSampleType( EMultiSampleType& OutMultiSampleType, uint32* pOutQualityLevels /*= NULL*/ )
{
	// NOTE: This assumes the desired resolution is set before calling this function.
	D3DDISPLAYMODE D3DDisplayMode;
	m_D3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &D3DDisplayMode );

	OutMultiSampleType = EMST_None;

	for( int MultiSampleType = EMST_8X; MultiSampleType > EMST_None; --MultiSampleType )
	{
		D3DMULTISAMPLE_TYPE DesiredMultisampleType = GetD3DMultiSampleType( static_cast<EMultiSampleType>( MultiSampleType ) );
		DWORD NumQualityLevels = 0;
		HRESULT hr = m_D3D->CheckDeviceMultiSampleType(	D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DDisplayMode.Format, !m_Fullscreen, DesiredMultisampleType, &NumQualityLevels );
		if( hr == D3D_OK )
		{
			OutMultiSampleType = static_cast<EMultiSampleType>( MultiSampleType );
			if( pOutQualityLevels )
			{
				*pOutQualityLevels = NumQualityLevels;
			}
			return;
		}
	}
}

void D3D9Renderer::CreateDefaultRenderTarget()
{
	XTRACE_FUNCTION;

	// Create the default render target
	IDirect3DSurface9* ColorRenderTarget;
	IDirect3DSurface9* DepthStencilSurface;
	m_D3DDevice->GetRenderTarget( 0, &ColorRenderTarget );
	m_D3DDevice->GetDepthStencilSurface( &DepthStencilSurface );
	if( m_DefaultRenderTarget )
	{
		// Hackity hack; after reset, we don't want to make a new RenderTarget,
		// so just inject the new surfaces into the existing one.
		static_cast< D3D9RenderTarget* >( m_DefaultRenderTarget )->Reset( ColorRenderTarget, DepthStencilSurface );
	}
	else
	{
		m_DefaultRenderTarget = new D3D9RenderTarget( this, m_D3DDevice, ColorRenderTarget, DepthStencilSurface );
	}
	m_CurrentRenderTarget = m_DefaultRenderTarget;
}

// Return true if the device is working fine or if
// it has been lost but is able to be reset now
bool D3D9Renderer::CanReset()
{
	XTRACE_FUNCTION;

	HRESULT hr = m_D3DDevice->TestCooperativeLevel();
	return ( hr == D3D_OK || hr == D3DERR_DEVICENOTRESET );
}

// Recover from a lost device
bool D3D9Renderer::Reset()
{
	XTRACE_FUNCTION;

	if( !CanReset() )
	{
		return false;
	}

	PreReset();

	D3DPRESENT_PARAMETERS Params;
	GetPresentParams( Params );

	XTRACE_BEGIN( ResetDevice );
		const HRESULT hr = m_D3DDevice->Reset( &Params );
		if( hr != D3D_OK )
		{
			return false;
		}
	XTRACE_END;

	m_DeviceLost = false;
	PostReset();

	return true;
}

void D3D9Renderer::PreReset()
{
	XTRACE_FUNCTION;

	// Release all D3DPOOL_DEFAULT resources
	FOR_EACH_LIST( TargetIter, m_RenderTargets, IRenderTarget* )
	{
		( *TargetIter )->Release();
	}
	FOR_EACH_SET( VertexBufferIter, m_DynamicVertexBuffers, IVertexBuffer* )
	{
		( *VertexBufferIter )->DeviceRelease();
	}

	m_DefaultRenderTarget->Release();
	m_CurrentRenderTarget = NULL;
}

void D3D9Renderer::PostReset()
{
	XTRACE_FUNCTION;

	CreateDefaultRenderTarget();

	FOR_EACH_LIST( TargetIter, m_RenderTargets, IRenderTarget* )
	{
		( *TargetIter )->Reset();
	}
	FOR_EACH_SET( VertexBufferIter, m_DynamicVertexBuffers, IVertexBuffer* )
	{
		( *VertexBufferIter )->DeviceReset();
	}

	ResetRenderState();
}

/*virtual*/ void D3D9Renderer::SetRestoreDeviceCallback( const SRestoreDeviceCallback& Callback )
{
	m_RestoreDeviceCallback = Callback;
}

void D3D9Renderer::Tick()
{
	XTRACE_FUNCTION;

	PROFILE_FUNCTION;

	// Restore lost device
	if( !IsValid() && CanReset() )
	{
		if( m_RestoreDeviceCallback.m_Callback )
		{
			m_RestoreDeviceCallback.m_Callback( m_RestoreDeviceCallback.m_Void );
		}
		return;
	}


#if BUILD_DEBUG
	m_DEBUGRenderStats.NumMeshes = 0;
	m_DEBUGRenderStats.NumPrimitives = 0;
#endif

	m_D3DDevice->BeginScene();

	RenderBuckets();
	PostRenderBuckets();

	m_D3DDevice->EndScene();

	Present();

#if BUILD_DEBUG
	STATIC_HASHED_STRING( Render );
	DEBUGCATPRINTF( sRender, 2, "%d meshes, %d primitives\n", m_DEBUGRenderStats.NumMeshes, m_DEBUGRenderStats.NumPrimitives );
#endif
}

/*virtual*/ void D3D9Renderer::SetViewport( const ViewportPass* const pViewportPass )
{
	XTRACE_FUNCTION;

	const SRect& Bounds = pViewportPass->GetBounds();

	D3DVIEWPORT9 Viewport;
	Viewport.X		= static_cast<DWORD>( Bounds.m_Left );
	Viewport.Y		= static_cast<DWORD>( Bounds.m_Top );
	Viewport.Width	= static_cast<DWORD>( Bounds.m_Right - Bounds.m_Left );
	Viewport.Height	= static_cast<DWORD>( Bounds.m_Bottom - Bounds.m_Top );
	Viewport.MinZ	= 0.0f;
	Viewport.MaxZ	= 1.0f;

	HRESULT hr = m_D3DDevice->SetViewport( &Viewport );
	Unused( hr );
	ASSERT( hr == D3D_OK );
}

/*virtual*/ void D3D9Renderer::RenderBucket( Bucket* pBucket, const ViewportPass* const pViewportPass )
{
	XTRACE_FUNCTION;

	PROFILE_FUNCTION;

	if( pBucket->m_RenderTarget )
	{
		SetRenderTarget( pBucket->m_RenderTarget );

		if( pViewportPass )
		{
			SetViewport( pViewportPass );
		}
	}

	if( pBucket->m_ClearFlags != CLEAR_NONE )
	{
		Clear( pBucket->m_ClearFlags, pBucket->m_ClearColor, pBucket->m_ClearDepth );
	}

	if( pBucket->m_View )
	{
		View* const pView = pViewportPass ? pViewportPass->GetView( pBucket->m_View ) : pBucket->m_View;
		m_View = *pView;
		m_View.ApplyToRenderer( *this );
	}

	if( pBucket->m_Flags & MAT_ALPHA )
	{
		pBucket->Sort( m_View );
	}
	else if( m_DoMaterialSort && pBucket->m_SortByMaterial )
	{
		pBucket->SortByMaterials();
	}

	Mesh*				pMesh				= NULL;
	D3D9VertexBuffer*	VertexBuffer		= NULL;
	D3D9IndexBuffer*	IndexBuffer			= NULL;
	IVertexDeclaration*	pVertexDeclaration	= NULL;
	uint				NumMeshes			= pBucket->m_Meshes.Size();
	Frustum				ViewFrustum( GetViewMatrix() * GetProjectionMatrix() );

#if BUILD_DEBUG
	if( pBucket->m_DEBUGUseFrustum )
	{
		pBucket->m_DEBUGFrustumView.ApplyToFrustum( ViewFrustum );
	}
#endif

	for( uint MeshIndex = 0; MeshIndex < NumMeshes; ++MeshIndex )
	{
		XTRACE_NAMED( RenderBucketMesh );

		pMesh = pBucket->m_Meshes[ MeshIndex ];
		DEVASSERT( pMesh );

		DEVASSERT( pMesh->m_VertexBuffer->GetNumVertices() > 0 );

		const uint MaterialFlags = pMesh->GetMaterialFlags();

		// Frustum culling--I can't do this earlier, when a mesh is added, because
		// it might be visible in one view (e.g., shadow map depth) and not to player.
		if(	!m_DoFrustumCulling					||
			pBucket->m_Flags & MAT_HUD			||
			pBucket->m_Flags & MAT_INWORLDHUD	||
#if BUILD_DEV
			pBucket->m_Flags & MAT_DEBUG_ALWAYS	||
			MaterialFlags & MAT_DEBUG_ALWAYS	||
#endif
			pBucket->m_Flags & MAT_ALWAYS		||
			MaterialFlags & MAT_ALWAYS			||
			ViewFrustum.Intersects( pMesh->m_AABB ) )
		{
			VertexBuffer		= (D3D9VertexBuffer*)pMesh->m_VertexBuffer;
			IndexBuffer			= (D3D9IndexBuffer*)pMesh->m_IndexBuffer;
			// RENDERTODO: It might be useful to adapt this for material overrides in the future
			//Shader			= (D3D9Shader*)( pBucket->m_OverrideShader ? pBucket->m_OverrideShader : pMesh->m_OLDMaterial.m_Shader );

			if( pMesh->m_VertexDeclaration != pVertexDeclaration )
			{
				XTRACE_NAMED( SetVertexDeclaration );
				m_D3DDevice->SetVertexDeclaration( ( IDirect3DVertexDeclaration9* )pMesh->m_VertexDeclaration->GetDeclaration() );
			}
			pVertexDeclaration = pMesh->m_VertexDeclaration;

			DEVASSERT( VertexBuffer );
			DEVASSERT( pVertexDeclaration );
			DEVASSERT( IndexBuffer );

			SetWorldMatrix( pMesh->GetConcatenatedTransforms() );

			{
				XTRACE_NAMED( SetStreams );

				uint VertexSignature = pVertexDeclaration->GetSignature();
				uint Index = 0;
#define SETSTREAM( STREAM, SIGNATURE, TYPE )																			\
	if( SIGNATURE == ( VertexSignature & SIGNATURE ) )																	\
	{																													\
		IDirect3DVertexBuffer9* const pBuffer = static_cast<IDirect3DVertexBuffer9*>( VertexBuffer->Get##STREAM() );	\
		DEVASSERT( pBuffer );																							\
		m_D3DDevice->SetStreamSource( Index++, pBuffer, 0, sizeof( TYPE ) );											\
	}

				SETSTREAM( Positions, VD_POSITIONS, Vector );
				SETSTREAM( Colors, VD_COLORS, uint );
#if USE_HDR
				SETSTREAM( FloatColors1, VD_FLOATCOLORS, Vector4 );
				SETSTREAM( FloatColors2, VD_BASISCOLORS, Vector4 );
				SETSTREAM( FloatColors3, VD_BASISCOLORS, Vector4 );

				// For SM2 cards, an alternative way to do HDR colors
				SETSTREAM( FloatColors1, VD_FLOATCOLORS_SM2, Vector4 );
				SETSTREAM( FloatColors2, VD_BASISCOLORS_SM2, Vector4 );
				SETSTREAM( FloatColors3, VD_BASISCOLORS_SM2, Vector4 );
#endif
				SETSTREAM( UVs, VD_UVS, Vector2 );
				SETSTREAM( Normals, VD_NORMALS, Vector );
				SETSTREAM( Tangents, VD_TANGENTS, Vector4 );
				SETSTREAM( BoneIndices, VD_BONEINDICES, SBoneData );
				SETSTREAM( BoneWeights, VD_BONEWEIGHTS, SBoneData );

#undef SETSTREAM

				m_D3DDevice->SetIndices( (IDirect3DIndexBuffer9*)IndexBuffer->GetIndices() );
			}

			ApplyMaterial( pMesh->m_Material, pMesh, m_View );

			if( VertexBuffer->GetNumVertices() > 0 )
			{
				XTRACE_NAMED( DrawIndexedPrimitive );

				m_D3DDevice->DrawIndexedPrimitive( IndexBuffer->GetPrimitiveType(), 0, 0, VertexBuffer->GetNumVertices(), 0, IndexBuffer->GetNumPrimitives() );
			}

#if BUILD_DEBUG
			++m_DEBUGRenderStats.NumMeshes;
			m_DEBUGRenderStats.NumPrimitives += IndexBuffer->GetNumPrimitives();
#endif
		}

#if BUILD_DEV
		// WARNING: This assumes the mesh is only in one bucket, which could be bad
		if( pMesh->m_IsDebugMesh )
		{
			m_DeferredDeleteDebugMeshes.PushBackUnique( pMesh );
		}
#endif
	}
}

void D3D9Renderer::Present()
{
	PROFILE_FUNCTION;

	XTRACE_FUNCTION;

	HRESULT hr = m_D3DDevice->Present( NULL, NULL, NULL, NULL );

	m_DeviceLost = ( hr == D3DERR_DEVICELOST );
}

void D3D9Renderer::Clear( unsigned int Flags, unsigned int Color /*= 0xff000000*/, float Depth /*= 1.0f*/, unsigned int Stencil /*= 0*/ )
{
	XTRACE_FUNCTION;

	if( Flags == CLEAR_NONE )
	{
		return;
	}

	uint D3DFlags = 0;
	if( Flags & CLEAR_COLOR )
	{
		D3DFlags |= D3DCLEAR_TARGET;
	}
	if( Flags & CLEAR_DEPTH )
	{
		D3DFlags |= D3DCLEAR_ZBUFFER;
	}
	if( Flags & CLEAR_STENCIL )
	{
		D3DFlags |= D3DCLEAR_STENCIL;
	}
	m_D3DDevice->Clear( 0, NULL, D3DFlags, Color, Depth, Stencil );
}

IVertexBuffer* D3D9Renderer::CreateVertexBuffer()
{
	return new D3D9VertexBuffer( m_D3DDevice );
}

IVertexDeclaration* D3D9Renderer::CreateVertexDeclaration()
{
	return new D3D9VertexDeclaration( m_D3DDevice );
}

IIndexBuffer* D3D9Renderer::CreateIndexBuffer()
{
	return new D3D9IndexBuffer( m_D3DDevice );
}

ITexture* D3D9Renderer::CreateTexture( const char* Filename )
{
	XTRACE_FUNCTION;

	D3D9Texture* const pTexture = new D3D9Texture( m_D3DDevice );
	pTexture->Initialize( Filename );
	return pTexture;
}

/*virtual*/ IVertexShader* D3D9Renderer::CreateVertexShader( const SimpleString& Filename )
{
	D3D9VertexShader* const pVertexShader = new D3D9VertexShader;
	pVertexShader->Initialize( m_D3DDevice, PackStream( Filename.CStr() ) );
	return pVertexShader;
}

/*virtual*/ IPixelShader* D3D9Renderer::CreatePixelShader( const SimpleString& Filename )
{
	D3D9PixelShader* const pPixelShader = new D3D9PixelShader;
	pPixelShader->Initialize( m_D3DDevice, PackStream( Filename.CStr() ) );
	return pPixelShader;
}

/*virtual*/ IShaderProgram* D3D9Renderer::CreateShaderProgram( IVertexShader* const pVertexShader, IPixelShader* const pPixelShader, IVertexDeclaration* const pVertexDeclaration )
{
	D3D9ShaderProgram* const pShaderProgram = new D3D9ShaderProgram;
	pShaderProgram->Initialize( pVertexShader, pPixelShader, pVertexDeclaration );
	return pShaderProgram;
}

IRenderTarget* D3D9Renderer::CreateRenderTarget( const SRenderTargetParams& Params )
{
	D3D9RenderTarget* pTarget = new D3D9RenderTarget( this, m_D3DDevice );
	pTarget->Initialize( Params );
	m_RenderTargets.PushBack( pTarget );
	return pTarget;
}

void D3D9Renderer::SetRenderTarget( IRenderTarget* RenderTarget )
{
	XTRACE_FUNCTION;

	m_CurrentRenderTarget = RenderTarget;
	m_D3DDevice->SetRenderTarget( 0, (IDirect3DSurface9*)m_CurrentRenderTarget->GetColorRenderTargetHandle() );
	m_D3DDevice->SetDepthStencilSurface( (IDirect3DSurface9*)m_CurrentRenderTarget->GetDepthStencilRenderTargetHandle() );
}

bool D3D9Renderer::IsValid()
{
	return !m_DeviceLost;
}

void D3D9Renderer::GetPresentParams( D3DPRESENT_PARAMETERS& Params )
{
	XTRACE_FUNCTION;

	D3DDISPLAYMODE D3DDisplayMode;
	m_D3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &D3DDisplayMode );	// NOTE: This assumes the desired resolution is set before calling this for fullscreen mode

	ZeroMemory( &Params, sizeof( D3DPRESENT_PARAMETERS ) );

	if( m_Fullscreen )
	{
		Params.BackBufferWidth = D3DDisplayMode.Width;
		Params.BackBufferHeight = D3DDisplayMode.Height;
		Params.BackBufferFormat = D3DDisplayMode.Format;

		Params.Windowed = false;
		Params.FullScreen_RefreshRateInHz = D3DDisplayMode.RefreshRate;
	}
	else
	{
		STATICHASH( DisplayWidth );
		STATICHASH( DisplayHeight );
		Params.BackBufferWidth = ConfigManager::GetInt( sDisplayWidth );
		Params.BackBufferHeight = ConfigManager::GetInt( sDisplayHeight );
		Params.BackBufferFormat = D3DFMT_UNKNOWN;

		Params.Windowed = true;
		Params.FullScreen_RefreshRateInHz = 0;
	}

	Params.BackBufferCount = 1;						// Could be 1, 2 or 3... Faster? Slower? Test it...

	D3DMULTISAMPLE_TYPE DesiredMultisampleType = GetD3DMultiSampleType( m_MultiSampleType );
	DWORD NumQualityLevels = 0;
	HRESULT hr = m_D3D->CheckDeviceMultiSampleType(	D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DDisplayMode.Format, Params.Windowed, DesiredMultisampleType, &NumQualityLevels );
	if( hr == D3D_OK )
	{
		Params.MultiSampleType = DesiredMultisampleType;
		Params.MultiSampleQuality = NumQualityLevels - 1;
	}
	else
	{
		Params.MultiSampleType = D3DMULTISAMPLE_NONE;
		Params.MultiSampleQuality = 0;
	}

	Params.SwapEffect = D3DSWAPEFFECT_DISCARD;		// Is apparently fastest (and allows multisampling)

	Params.hDeviceWindow = m_hWnd;

	Params.EnableAutoDepthStencil = true;
	Params.AutoDepthStencilFormat = ( D3DDisplayMode.Format == D3DFMT_X8R8G8B8 ) ? D3DFMT_D24S8 : D3DFMT_D16;

	Params.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;	// Discard z-buffer after Present()ing, can improve performance

	STATICHASH( VSync );
	if( ConfigManager::GetBool( sVSync, true ) )
	{
		Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;		// D3DPRESENT_INTERVAL_DEFAULT is basically the same; coarser time resolution but a bit faster?
	}
	else
	{
		Params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}
}

/*virtual*/ void D3D9Renderer::EnumerateDisplayModes( Array<SDisplayMode>& DisplayModes )
{
	DisplayModes.Clear();

	EnumerateDisplayModes( DisplayModes, D3DFMT_X8R8G8B8 );
	EnumerateDisplayModes( DisplayModes, D3DFMT_A8R8G8B8 );

	ASSERT( DisplayModes.Size() );
}

void D3D9Renderer::EnumerateDisplayModes( Array<SDisplayMode>& DisplayModes, D3DFORMAT Format )
{
	uint NumModes = m_D3D->GetAdapterModeCount( D3DADAPTER_DEFAULT, Format );
	for( uint i = 0; i < NumModes; ++i )
	{
		D3DDISPLAYMODE D3DMode = {0};

		m_D3D->EnumAdapterModes( D3DADAPTER_DEFAULT, Format, i, &D3DMode );

		SDisplayMode Mode;
		Mode.Width = D3DMode.Width;
		Mode.Height = D3DMode.Height;
		DisplayModes.PushBackUnique( Mode );
	}
}

// HACKY: Just used for RadiositySolver, which freely works in D3D
void* D3D9Renderer::GetDevice()
{
	return m_D3DDevice;
}

/*virtual*/ void D3D9Renderer::SaveScreenshot( const SimpleString& Filename )
{
#if BUILD_STEAM
	Unused( Filename );
#else
	ASSERT( m_CurrentRenderTarget );

	IDirect3DSurface9* const pRenderTargetSurface = static_cast<IDirect3DSurface9*>( m_CurrentRenderTarget->GetColorRenderTargetHandle() );
	ASSERT( pRenderTargetSurface );

	D3DXSaveSurfaceToFile( Filename.CStr(), D3DXIFF_PNG, pRenderTargetSurface, NULL, NULL );
#endif
}

void D3D9Renderer::TestCapabilities()
{
	PRINTF( "Testing Direct3D device capabilities\n" );

	D3DCAPS9 Caps;
	m_D3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &Caps );

#define PRINTCAP( cap, format ) PRINTF( "\t" #cap ": " format "\n", Caps.cap )

	PRINTCAP( DeviceType, "%d" );
	PRINTCAP( AdapterOrdinal, "%d" );
	PRINTCAP( Caps, "0x%08X" );
	PRINTCAP( Caps2, "0x%08X" );
	PRINTCAP( Caps3, "0x%08X" );
	PRINTCAP( PresentationIntervals, "0x%08X" );
	PRINTCAP( CursorCaps, "0x%08X" );
	PRINTCAP( DevCaps, "0x%08X" );
	PRINTCAP( PrimitiveMiscCaps, "0x%08X" );
	PRINTCAP( RasterCaps, "0x%08X" );
	PRINTCAP( ZCmpCaps, "0x%08X" );
	PRINTCAP( SrcBlendCaps, "0x%08X" );
	PRINTCAP( DestBlendCaps, "0x%08X" );
	PRINTCAP( AlphaCmpCaps, "0x%08X" );
	PRINTCAP( ShadeCaps, "0x%08X" );
	PRINTCAP( TextureCaps, "0x%08X" );
	PRINTCAP( TextureFilterCaps, "0x%08X" );
	PRINTCAP( CubeTextureFilterCaps, "0x%08X" );
	PRINTCAP( VolumeTextureFilterCaps, "0x%08X" );
	PRINTCAP( TextureAddressCaps, "0x%08X" );
	PRINTCAP( VolumeTextureAddressCaps, "0x%08X" );
	PRINTCAP( LineCaps, "0x%08X" );
	PRINTCAP( MaxTextureWidth, "%d" );
	PRINTCAP( MaxTextureHeight, "%d" );
	PRINTCAP( MaxVolumeExtent, "%d" );
	PRINTCAP( MaxTextureRepeat, "%d" );
	PRINTCAP( MaxTextureAspectRatio, "%d" );
	PRINTCAP( MaxAnisotropy, "%d" );
	PRINTCAP( MaxVertexW, "%f" );
	PRINTCAP( GuardBandLeft, "%f" );
	PRINTCAP( GuardBandTop, "%f" );
	PRINTCAP( GuardBandRight, "%f" );
	PRINTCAP( GuardBandBottom, "%f" );
	PRINTCAP( ExtentsAdjust, "%f" );
	PRINTCAP( StencilCaps, "0x%08X" );
	PRINTCAP( FVFCaps, "0x%08X" );
	PRINTCAP( TextureOpCaps, "0x%08X" );
	PRINTCAP( MaxTextureBlendStages, "%d" );
	PRINTCAP( MaxSimultaneousTextures, "%d" );
	PRINTCAP( VertexProcessingCaps, "0x%08X" );
	PRINTCAP( MaxActiveLights, "%d" );
	PRINTCAP( MaxUserClipPlanes, "%d" );
	PRINTCAP( MaxVertexBlendMatrices, "%d" );
	PRINTCAP( MaxVertexBlendMatrixIndex, "%d" );
	PRINTCAP( MaxPointSize, "%f" );
	PRINTCAP( MaxPrimitiveCount, "%d" );
	PRINTCAP( MaxVertexIndex, "%d" );
	PRINTCAP( MaxStreams, "%d" );
	PRINTCAP( MaxStreamStride, "%d" );
	PRINTCAP( VertexShaderVersion, "0x%08X" );
	PRINTCAP( MaxVertexShaderConst, "%d" );
	PRINTCAP( PixelShaderVersion, "0x%08X" );
	PRINTCAP( PixelShader1xMaxValue, "%f" );
	PRINTCAP( DevCaps2, "0x%08X" );
	PRINTCAP( MaxNpatchTessellationLevel, "%f" );
	PRINTCAP( MasterAdapterOrdinal, "%d" );
	PRINTCAP( AdapterOrdinalInGroup, "%d" );
	PRINTCAP( NumberOfAdaptersInGroup, "%d" );
	PRINTCAP( DeclTypes, "0x%08X" );
	PRINTCAP( NumSimultaneousRTs, "%d" );
	PRINTCAP( StretchRectFilterCaps, "0x%08X" );
	PRINTCAP( VS20Caps.StaticFlowControlDepth, "%d" );
	PRINTCAP( VS20Caps.DynamicFlowControlDepth, "%d" );
	PRINTCAP( VS20Caps.NumTemps, "%d" );
	PRINTCAP( VS20Caps.StaticFlowControlDepth, "%d" );
	PRINTCAP( PS20Caps.Caps, "%d" );
	PRINTCAP( PS20Caps.DynamicFlowControlDepth, "%d" );
	PRINTCAP( PS20Caps.NumTemps, "%d" );
	PRINTCAP( PS20Caps.StaticFlowControlDepth, "%d" );
	PRINTCAP( PS20Caps.NumInstructionSlots, "%d" );
	PRINTCAP( VertexTextureFilterCaps, "0x%08X" );
	PRINTCAP( MaxVShaderInstructionsExecuted, "%d" );
	PRINTCAP( MaxPShaderInstructionsExecuted, "%d" );
	PRINTCAP( MaxVertexShader30InstructionSlots, "%d" );
	PRINTCAP( MaxPixelShader30InstructionSlots, "%d" );
#undef PRINTCAP

	HRESULT hr;
#define TESTFORMAT( adapterfmt, usage, rtype, fmt ) \
	hr = m_D3D->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterfmt, usage, rtype, fmt ); \
	PRINTF( #adapterfmt " " #usage " " #rtype " " #fmt ": 0x%08X\n", hr )

	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_DEPTHSTENCIL,	D3DRTYPE_SURFACE,		D3DFMT_D24S8 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_DEPTHSTENCIL,	D3DRTYPE_SURFACE,		D3DFMT_D16 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT1 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT2 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT3 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT4 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT5 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	0,						D3DRTYPE_TEXTURE,		D3DFMT_A8R8G8B8 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_X8R8G8B8 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_X8R8G8B8 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_A8R8G8B8 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_A8R8G8B8 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_A16B16G16R16 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_A16B16G16R16 );
	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_A16B16G16R16F );
	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_A16B16G16R16F );
	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_A32B32G32R32F );
	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_A32B32G32R32F );
	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_R32F );
	TESTFORMAT( D3DFMT_X8R8G8B8,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_R32F );

	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_DEPTHSTENCIL,	D3DRTYPE_SURFACE,		D3DFMT_D24S8 );
	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_DEPTHSTENCIL,	D3DRTYPE_SURFACE,		D3DFMT_D16 );
	TESTFORMAT( D3DFMT_R5G6B5,		0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT1 );
	TESTFORMAT( D3DFMT_R5G6B5,		0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT2 );
	TESTFORMAT( D3DFMT_R5G6B5,		0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT3 );
	TESTFORMAT( D3DFMT_R5G6B5,		0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT4 );
	TESTFORMAT( D3DFMT_R5G6B5,		0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT5 );
	TESTFORMAT( D3DFMT_R5G6B5,		0,						D3DRTYPE_TEXTURE,		D3DFMT_A8R8G8B8 );
	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_X8R8G8B8 );
	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_X8R8G8B8 );
	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_A8R8G8B8 );
	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_A8R8G8B8 );
	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_A16B16G16R16 );
	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_A16B16G16R16 );
	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_A16B16G16R16F );
	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_A16B16G16R16F );
	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_A32B32G32R32F );
	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_A32B32G32R32F );
	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_R32F );
	TESTFORMAT( D3DFMT_R5G6B5,		D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_R32F );

	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_DEPTHSTENCIL,	D3DRTYPE_SURFACE,		D3DFMT_D24S8 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_DEPTHSTENCIL,	D3DRTYPE_SURFACE,		D3DFMT_D16 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT1 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT2 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT3 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT4 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	0,						D3DRTYPE_TEXTURE,		D3DFMT_DXT5 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	0,						D3DRTYPE_TEXTURE,		D3DFMT_A8R8G8B8 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_X8R8G8B8 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_X8R8G8B8 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_A8R8G8B8 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_A8R8G8B8 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_A16B16G16R16 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_A16B16G16R16 );
	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_A16B16G16R16F );
	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_A16B16G16R16F );
	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_A32B32G32R32F );
	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_A32B32G32R32F );
	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_SURFACE,		D3DFMT_R32F );
	TESTFORMAT( D3DFMT_X1R5G5B5,	D3DUSAGE_RENDERTARGET,	D3DRTYPE_TEXTURE,		D3DFMT_R32F );

#undef TESTFORMAT
}

/*virtual*/ void D3D9Renderer::SetVertexShader( IVertexShader* const pVertexShader )
{
	DEBUGASSERT( m_D3DDevice );
	DEBUGASSERT( pVertexShader );

	if( pVertexShader == m_VertexShader )
	{
		return;
	}
	m_VertexShader = pVertexShader;

	IDirect3DVertexShader9* const pD3DVertexShader = static_cast<IDirect3DVertexShader9*>( pVertexShader->GetHandle() );
	const HRESULT Result = m_D3DDevice->SetVertexShader( pD3DVertexShader );
	DEBUGASSERT( Result == D3D_OK );
	Unused( Result );
}

/*virtual*/ void D3D9Renderer::SetPixelShader( IPixelShader* const pPixelShader )
{
	DEBUGASSERT( m_D3DDevice );
	DEBUGASSERT( pPixelShader );

	if( pPixelShader == m_PixelShader )
	{
		return;
	}
	m_PixelShader = pPixelShader;

	IDirect3DPixelShader9* const pD3DPixelShader = static_cast<IDirect3DPixelShader9*>( pPixelShader->GetHandle() );
	const HRESULT Result = m_D3DDevice->SetPixelShader( pD3DPixelShader );
	DEBUGASSERT( Result == D3D_OK );
	Unused( Result );
}

/*virtual*/ void D3D9Renderer::SetShaderProgram( IShaderProgram* const pShaderProgram )
{
	DEBUGASSERT( pShaderProgram );

	if( pShaderProgram == m_ShaderProgram )
	{
		return;
	}
	m_ShaderProgram = pShaderProgram;

	SetVertexShader( pShaderProgram->GetVertexShader() );
	SetPixelShader( pShaderProgram->GetPixelShader() );
}

/*virtual*/ SimpleString D3D9Renderer::GetShaderType() const
{
	return "HLSL";
}

static DWORD D3DCullMode[] =
{
	0,							//	ECM_Unknown
	D3DCULL_NONE,
	D3DCULL_CW,
	D3DCULL_CCW,
};

/*virtual*/ void D3D9Renderer::SetCullMode( const ECullMode CullMode )
{
	DEBUGASSERT( m_D3DDevice );
	DEBUGASSERT( CullMode > ECM_Unknown );

	if( CullMode == m_RenderState.m_CullMode )
	{
		return;
	}
	m_RenderState.m_CullMode = CullMode;

	const HRESULT Result = m_D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCullMode[ CullMode ] );
	DEBUGASSERT( Result == D3D_OK );
	Unused( Result );
}

/*virtual*/ void D3D9Renderer::SetZEnable( const EZEnable ZEnable )
{
	DEBUGASSERT( m_D3DDevice );
	DEBUGASSERT( ZEnable > EZE_Unknown );

	if( ZEnable == m_RenderState.m_ZEnable )
	{
		return;
	}
	m_RenderState.m_ZEnable = ZEnable;

	const HRESULT Result = m_D3DDevice->SetRenderState( D3DRS_ZENABLE, ( ZEnable == EZE_True ) ? TRUE : FALSE );
	DEBUGASSERT( Result == D3D_OK );
	Unused( Result );
}

/*virtual*/ void D3D9Renderer::SetZWriteEnable( const EZWriteEnable ZWriteEnable )
{
	DEBUGASSERT( m_D3DDevice );
	DEBUGASSERT( ZWriteEnable > EZWE_Unknown );

	if( ZWriteEnable == m_RenderState.m_ZWriteEnable )
	{
		return;
	}
	m_RenderState.m_ZWriteEnable = ZWriteEnable;

	const HRESULT Result = m_D3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, ( ZWriteEnable == EZWE_True ) ? TRUE : FALSE );
	DEBUGASSERT( Result == D3D_OK );
	Unused( Result );
}

/*virtual*/ void D3D9Renderer::SetAlphaBlendEnable( const EAlphaBlendEnable AlphaBlendEnable )
{
	DEBUGASSERT( m_D3DDevice );
	DEBUGASSERT( AlphaBlendEnable > EABE_Unknown );

	if( AlphaBlendEnable == m_RenderState.m_AlphaBlendEnable )
	{
		return;
	}
	m_RenderState.m_AlphaBlendEnable = AlphaBlendEnable;

	const HRESULT Result = m_D3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, ( AlphaBlendEnable == EABE_True ) ? TRUE : FALSE );
	DEBUGASSERT( Result == D3D_OK );
	Unused( Result );
}

static DWORD D3DBlend[] =
{
	0,							//	EB_Unknown
	D3DBLEND_ZERO,
	D3DBLEND_ONE,
	D3DBLEND_SRCCOLOR,
	D3DBLEND_INVSRCCOLOR,
	D3DBLEND_SRCALPHA,
	D3DBLEND_INVSRCALPHA,
	D3DBLEND_DESTALPHA,
	D3DBLEND_INVDESTALPHA,
	D3DBLEND_DESTCOLOR,
	D3DBLEND_INVDESTCOLOR,
	D3DBLEND_SRCALPHASAT,
};

/*virtual*/ void D3D9Renderer::SetBlend( const EBlend SrcBlend, const EBlend DestBlend )
{
	DEBUGASSERT( m_D3DDevice );
	DEBUGASSERT( SrcBlend > EB_Unknown );
	DEBUGASSERT( DestBlend > EB_Unknown );

	if( SrcBlend == m_RenderState.m_SrcBlend &&
		DestBlend == m_RenderState.m_DestBlend )
	{
		return;
	}
	m_RenderState.m_SrcBlend = SrcBlend;
	m_RenderState.m_DestBlend = DestBlend;

	{
		const HRESULT Result = m_D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBlend[ SrcBlend ] );
		DEBUGASSERT( Result == D3D_OK );
		Unused( Result );
	}

	{
		const HRESULT Result = m_D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBlend[ DestBlend ] );
		DEBUGASSERT( Result == D3D_OK );
		Unused( Result );
	}
}

/*virtual*/ void D3D9Renderer::SetTexture( const uint SamplerStage, ITexture* const pTexture )
{
	DEBUGASSERT( m_D3DDevice );
	DEBUGASSERT( SamplerStage < MAX_TEXTURE_STAGES );
	DEBUGASSERT( pTexture );

	SSamplerState& SamplerState = m_SamplerStates[ SamplerStage ];

	if( pTexture == SamplerState.m_Texture )
	{
		return;
	}
	SamplerState.m_Texture = pTexture;

	IDirect3DTexture9* const pD3DTexture = static_cast<IDirect3DTexture9*>( pTexture->GetHandle() );
	const HRESULT Result = m_D3DDevice->SetTexture( SamplerStage, pD3DTexture );
	DEBUGASSERT( Result == D3D_OK );
	Unused( Result );
}

static DWORD D3DTextureAddress[] =
{
	0,					//	ETA_Unknown
	D3DTADDRESS_WRAP,
	D3DTADDRESS_MIRROR,
	D3DTADDRESS_CLAMP,
};

/*virtual*/ void D3D9Renderer::SetAddressing( const uint SamplerStage, const ETextureAddress AddressU, const ETextureAddress AddressV )
{
	DEBUGASSERT( m_D3DDevice );
	DEBUGASSERT( SamplerStage < MAX_TEXTURE_STAGES );
	DEBUGASSERT( AddressU > ETA_Unknown );
	DEBUGASSERT( AddressV > ETA_Unknown );

	SSamplerState& SamplerState = m_SamplerStates[ SamplerStage ];

	if( AddressU == SamplerState.m_AddressU &&
		AddressV == SamplerState.m_AddressV )
	{
		return;
	}
	SamplerState.m_AddressU = AddressU;
	SamplerState.m_AddressV = AddressV;

	{
		const HRESULT Result = m_D3DDevice->SetSamplerState( SamplerStage, D3DSAMP_ADDRESSU, D3DTextureAddress[ AddressU ] );
		DEBUGASSERT( Result == D3D_OK );
		Unused( Result );
	}

	{
		const HRESULT Result = m_D3DDevice->SetSamplerState( SamplerStage, D3DSAMP_ADDRESSV, D3DTextureAddress[ AddressV ] );
		DEBUGASSERT( Result == D3D_OK );
		Unused( Result );
	}
}

static DWORD D3DTextureFilter[] =
{
	0,					// ETF_Unknown
	D3DTEXF_NONE,
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_ANISOTROPIC,
};

/*virtual*/ void D3D9Renderer::SetMinMipFilters( const uint SamplerStage, const ETextureFilter MinFilter, const ETextureFilter MipFilter )
{
	DEBUGASSERT( m_D3DDevice );
	DEBUGASSERT( SamplerStage < MAX_TEXTURE_STAGES );
	DEBUGASSERT( MinFilter > ETF_None );
	DEBUGASSERT( MipFilter > ETF_Unknown );

	SSamplerState& SamplerState = m_SamplerStates[ SamplerStage ];

	if( MinFilter == SamplerState.m_MinFilter &&
		MipFilter == SamplerState.m_MipFilter )
	{
		return;
	}
	SamplerState.m_MinFilter = MinFilter;
	SamplerState.m_MipFilter = MipFilter;

	{
		const HRESULT Result = m_D3DDevice->SetSamplerState( SamplerStage, D3DSAMP_MINFILTER, D3DTextureFilter[ MinFilter ] );
		DEBUGASSERT( Result == D3D_OK );
		Unused( Result );
	}

	{
		const HRESULT Result = m_D3DDevice->SetSamplerState( SamplerStage, D3DSAMP_MIPFILTER, D3DTextureFilter[ MipFilter ] );
		DEBUGASSERT( Result == D3D_OK );
		Unused( Result );
	}
}

/*virtual*/ void D3D9Renderer::SetMagFilter( const uint SamplerStage, const ETextureFilter MagFilter )
{
	DEBUGASSERT( m_D3DDevice );
	DEBUGASSERT( SamplerStage < MAX_TEXTURE_STAGES );
	DEBUGASSERT( MagFilter > ETF_None );

	SSamplerState& SamplerState = m_SamplerStates[ SamplerStage ];

	if( MagFilter == SamplerState.m_MagFilter )
	{
		return;
	}
	SamplerState.m_MagFilter = MagFilter;

	const HRESULT Result = m_D3DDevice->SetSamplerState( SamplerStage, D3DSAMP_MAGFILTER, D3DTextureFilter[ MagFilter ] );
	DEBUGASSERT( Result == D3D_OK );
	Unused( Result );
}

/*virtual*/ void D3D9Renderer::SetVertexShaderFloat4( const HashedString& Parameter, const float* const pFloats, const uint NumFloat4s )
{
	ASSERT( m_VertexShader );

	uint Register;
	if( !m_VertexShader->GetRegister( Parameter, Register ) )
	{
		return;
	}

	const HRESULT Result = m_D3DDevice->SetVertexShaderConstantF( Register, pFloats, NumFloat4s );
	DEBUGASSERT( Result == D3D_OK );
	Unused( Result );
}

/*virtual*/ void D3D9Renderer::SetVertexShaderMatrix( const HashedString& Parameter, const float* const pFloats, const uint NumMatrices )
{
	ASSERT( m_VertexShader );

	uint Register;
	if( !m_VertexShader->GetRegister( Parameter, Register ) )
	{
		return;
	}

	const HRESULT Result = m_D3DDevice->SetVertexShaderConstantF( Register, pFloats, NumMatrices * 4 );
	DEBUGASSERT( Result == D3D_OK );
	Unused( Result );
}

/*virtual*/ void D3D9Renderer::SetPixelShaderFloat4( const HashedString& Parameter, const float* const pFloats, const uint NumFloat4s )
{
	ASSERT( m_PixelShader );

	uint Register;
	if( !m_PixelShader->GetRegister( Parameter, Register ) )
	{
		return;
	}

	const HRESULT Result = m_D3DDevice->SetPixelShaderConstantF( Register, pFloats, NumFloat4s );
	DEBUGASSERT( Result == D3D_OK );
	Unused( Result );
}

/*virtual*/ void D3D9Renderer::SetPixelShaderMatrix( const HashedString& Parameter, const float* const pFloats, const uint NumMatrices )
{
	ASSERT( m_PixelShader );

	uint Register;
	if( !m_PixelShader->GetRegister( Parameter, Register ) )
	{
		return;
	}

	const HRESULT Result = m_D3DDevice->SetPixelShaderConstantF( Register, pFloats, NumMatrices * 4 );
	DEBUGASSERT( Result == D3D_OK );
	Unused( Result );
}