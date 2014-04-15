#include "core.h"
#include "gl2renderer.h"
#include "gl2vertexbuffer.h"
#include "gl2indexbuffer.h"
#include "gl2texture.h"
#include "gl2vertexdeclaration.h"
#include "gl2rendertarget.h"
#include "gl2vertexshader.h"
#include "gl2pixelshader.h"
#include "gl2shaderprogram.h"
#include "simplestring.h"
#include "vector4.h"
#include "bucket.h"
#include "mesh.h"
#include "viewportpass.h"
#include "frustum.h"
#include "configmanager.h"
#include "packstream.h"
#include "windowwrapper.h"

// I'm making this easy to compile out because GL, with its global state,
// might be causing problems if I create new objects after setting render state.
#define IGNORE_REDUNDANT_STATE			1

// This is supported separately, with state shadowed on each texture.
#define IGNORE_REDUNDANT_SAMPLER_STATE	1

GL2Renderer::GL2Renderer( Window* const pWindow )
:	m_Window( pWindow )
,	m_RenderContext( NULL )
,	m_MaxVertexAttribs( 0 )
{
}

GL2Renderer::~GL2Renderer()
{
	GLERRORCHECK;

	{
#if BUILD_WINDOWS_NO_SDL
		const BOOL Success	= wglMakeCurrent( NULL, NULL );
		Unused( Success );
		ASSERT( Success == TRUE );
#endif
#if BUILD_SDL
		const int Error = SDL_GL_MakeCurrent( NULL, NULL );
		Unused( Error );
		ASSERT( Error == 0 );
#endif
	}

	if( m_RenderContext != NULL )
	{
#if BUILD_WINDOWS_NO_SDL
		const BOOL Success = wglDeleteContext( m_RenderContext );
		Unused( Success );
		ASSERT( Success == TRUE );
#endif
#if BUILD_SDL
		SDL_GL_DeleteContext( m_RenderContext );
#endif
	}
}

void GL2Renderer::Initialize()
{
	RendererCommon::Initialize();

#if BUILD_WINDOWS_NO_SDL
	{
		PIXELFORMATDESCRIPTOR PixelFormatDescriptor = { 0 };

		// TODO PORT: Choose flags to match D3D settings. (Also, maybe move this to a "GetPresentParams" kind of function.)
		// TODO SDL: Do I need to do anything like this? It seemed to work just fine without it.
		PixelFormatDescriptor.nSize			= sizeof( PIXELFORMATDESCRIPTOR );
		PixelFormatDescriptor.nVersion		= 1;
		PixelFormatDescriptor.dwFlags		= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		PixelFormatDescriptor.iPixelType	= PFD_TYPE_RGBA;
		PixelFormatDescriptor.cColorBits	= 24;
		PixelFormatDescriptor.cAlphaBits	= 8;
		PixelFormatDescriptor.cDepthBits	= 32;
		PixelFormatDescriptor.iLayerType	= PFD_MAIN_PLANE;

		const int	PixelFormat	= ChoosePixelFormat( m_Window->GetHDC(), &PixelFormatDescriptor );
		ASSERT( PixelFormat > 0 );

		const BOOL	Success		= SetPixelFormat( m_Window->GetHDC(), PixelFormat, &PixelFormatDescriptor );
		Unused( Success );
		ASSERT( Success == TRUE );
	}
#endif // BUILD_WINDOWS_NO_SDL

#if BUILD_WINDOWS_NO_SDL
	m_RenderContext = wglCreateContext( m_Window->GetHDC() );
	ASSERT( m_RenderContext != NULL );
#endif
#if BUILD_SDL
	m_RenderContext = SDL_GL_CreateContext( m_Window->GetSDLWindow() );
	ASSERT( m_RenderContext != NULL );
#endif

	{
#if BUILD_WINDOWS_NO_SDL
		const BOOL Success	= wglMakeCurrent( m_Window->GetHDC(), m_RenderContext );
		Unused( Success );
		ASSERT( Success == TRUE );
#endif
#if BUILD_SDL
		const int Error = SDL_GL_MakeCurrent( m_Window->GetSDLWindow(), m_RenderContext );
		Unused( Error );
		ASSERT( Error == 0 );
#endif
	}

	{
		const GLenum Error		= glewInit();
		Unused( Error );
		ASSERT( Error == GLEW_OK );
	}

	// TODO: Move to a "test capabilities" kind of function
	{
		ASSERT( GLEW_VERSION_2_1 );
		ASSERT( GLEW_EXT_framebuffer_object || GLEW_ARB_framebuffer_object );
		ASSERT( GLEW_EXT_texture_compression_s3tc );
#if BUILD_WINDOWS_NO_SDL
		ASSERT( WGLEW_EXT_swap_control );
#endif
	}

	glFrontFace( GL_CCW );
	glGetIntegerv( GL_MAX_VERTEX_ATTRIBS, &m_MaxVertexAttribs );

	STATICHASH( VSync );
	const int SwapInterval = ConfigManager::GetBool( sVSync, true ) ? 1 : 0;
#if BUILD_WINDOWS_NO_SDL
	wglSwapIntervalEXT( SwapInterval );
#endif
#if BUILD_SDL
	SDL_GL_SetSwapInterval( SwapInterval );
#endif

	m_DefaultRenderTarget = new GL2RenderTarget;
}

bool GL2Renderer::SupportsSM3()
{
	// SM2/3 doesn't have any meaning in OpenGL.
	return true;
}

bool GL2Renderer::SupportsSM2()
{
	// SM2/3 doesn't have any meaning in OpenGL.
	return true;
}

/*virtual*/ void* GL2Renderer::GetDevice()
{
	DEVWARN;	// There's no reason I should need to call this.
	return m_RenderContext;
}

/*virtual*/ ERenderTargetFormat GL2Renderer::GetBestSupportedRenderTargetFormat( const ERenderTargetFormat Format ) const
{
	// TODO PORT LATER GetBestSupportedRenderTargetFormat
	// There's a good chance I don't need this, because the OpenGL spec allows the implementation to
	// automatically demote the requested format to a supported format when needed.
	Unused( Format );
	return ERTF_Unknown;
}

void GL2Renderer::SetFullScreen( bool FullScreen )
{
	// GL doesn't need to do anything special for fullscreen.
	// It just runs as a borderless fullscreen window.
	Unused( FullScreen );
}

void GL2Renderer::SetMultiSampleType( EMultiSampleType MultiSampleType )
{
	// TODO PORT LATER SetMultiSampleType
	Unused( MultiSampleType );
}

void GL2Renderer::GetBestSupportedMultiSampleType( EMultiSampleType& OutMultiSampleType, uint32* pOutQualityLevels /*= NULL*/ )
{
	// TODO PORT LATER GetBestSupportedMultiSampleType
	Unused( OutMultiSampleType );
	Unused( pOutQualityLevels );
}

bool GL2Renderer::CanReset()
{
	return true;
}

bool GL2Renderer::Reset()
{
	return true;
}

/*virtual*/ void GL2Renderer::SetRestoreDeviceCallback( const SRestoreDeviceCallback& Callback )
{
	Unused( Callback );
}

/*virtual*/ void GL2Renderer::Refresh()
{
#if BUILD_SDL
	// On Linux, at least, SDL needs to be set as the current context *after* the
	// GL window is shown, so this allows be to do it there.
	if( m_Window && m_Window->GetSDLWindow() && m_RenderContext )
	{
		const int Error = SDL_GL_MakeCurrent( m_Window->GetSDLWindow(), m_RenderContext );
		Unused( Error );
		ASSERT( Error == 0 );
	}
#endif
}

void GL2Renderer::Tick()
{
	XTRACE_FUNCTION;

	PROFILE_FUNCTION;

#if BUILD_DEBUG
	m_DEBUGRenderStats.NumMeshes = 0;
	m_DEBUGRenderStats.NumPrimitives = 0;
#endif

	RenderBuckets();
	PostRenderBuckets();

	Present();

#if BUILD_DEBUG
	STATIC_HASHED_STRING( Render );
	DEBUGCATPRINTF( sRender, 2, "%d meshes, %d primitives\n", m_DEBUGRenderStats.NumMeshes, m_DEBUGRenderStats.NumPrimitives );
#endif
}

/*virtual*/ void GL2Renderer::SetViewport( const ViewportPass* const pViewportPass )
{
	// TODO PORT LATER SetViewport
	Unused( pViewportPass );
}

/*virtual*/ void GL2Renderer::RenderBucket( Bucket* pBucket, const ViewportPass* const pViewportPass )
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
	GL2VertexBuffer*	VertexBuffer		= NULL;
	GL2IndexBuffer*		IndexBuffer			= NULL;
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
			VertexBuffer		= static_cast<GL2VertexBuffer*>( pMesh->m_VertexBuffer );
			IndexBuffer			= static_cast<GL2IndexBuffer*>( pMesh->m_IndexBuffer );

			pVertexDeclaration = pMesh->m_VertexDeclaration;

			DEVASSERT( VertexBuffer );
			DEVASSERT( pVertexDeclaration );
			DEVASSERT( IndexBuffer );

			SetWorldMatrix( pMesh->GetConcatenatedTransforms() );

			{
				XTRACE_NAMED( SetStreams );

				const uint	VertexSignature	= pVertexDeclaration->GetSignature();
				uint		Index			= 0;

#define SETSTREAM( STREAM, SIGNATURE, NUMCOMPONENTS, COMPONENTTYPE, NORMALIZE )				\
	if( SIGNATURE == ( VertexSignature & SIGNATURE ) )										\
	{																						\
		const GLuint VBO = *static_cast<GLuint*>( VertexBuffer->Get##STREAM() );			\
		ASSERT( VBO != 0 );																	\
		glBindBuffer( GL_ARRAY_BUFFER, VBO );												\
		glEnableVertexAttribArray( Index );													\
		glVertexAttribPointer( Index, NUMCOMPONENTS, COMPONENTTYPE, NORMALIZE, 0, NULL );	\
		++Index;																			\
	}

				SETSTREAM( Positions,		VD_POSITIONS,		3, GL_FLOAT,			GL_FALSE );
				SETSTREAM( Colors,			VD_COLORS,			4, GL_UNSIGNED_BYTE,	GL_TRUE );
#if USE_HDR
				SETSTREAM( FloatColors1,	VD_FLOATCOLORS,		4, GL_FLOAT,			GL_FALSE );
				SETSTREAM( FloatColors2,	VD_BASISCOLORS,		4, GL_FLOAT,			GL_FALSE );
				SETSTREAM( FloatColors3,	VD_BASISCOLORS,		4, GL_FLOAT,			GL_FALSE );

				// Not actually different on GL, but needed for compatibility.
				SETSTREAM( FloatColors1,	VD_FLOATCOLORS_SM2,	4, GL_FLOAT,			GL_FALSE );
				SETSTREAM( FloatColors2,	VD_BASISCOLORS_SM2,	4, GL_FLOAT,			GL_FALSE );
				SETSTREAM( FloatColors3,	VD_BASISCOLORS_SM2,	4, GL_FLOAT,			GL_FALSE );
#endif
				SETSTREAM( UVs,				VD_UVS,				2, GL_FLOAT,			GL_FALSE );
				SETSTREAM( Normals,			VD_NORMALS,			3, GL_FLOAT,			GL_FALSE );
				SETSTREAM( Tangents,		VD_TANGENTS,		4, GL_FLOAT,			GL_FALSE );
				SETSTREAM( BoneIndices,		VD_BONEINDICES,		4, GL_UNSIGNED_BYTE,	GL_FALSE );
				SETSTREAM( BoneWeights,		VD_BONEWEIGHTS,		4, GL_UNSIGNED_BYTE,	GL_TRUE );

#undef SETSTREAM

				// Disable all the attributes we're not using.
				for( int DisableAttribIndex = Index; DisableAttribIndex < m_MaxVertexAttribs; ++DisableAttribIndex )
				{
					glDisableVertexAttribArray( DisableAttribIndex );
				}

				const GLuint IBO = *static_cast<GLuint*>( IndexBuffer->GetIndices() );
				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IBO );
			}

			ApplyMaterial( pMesh->m_Material, pMesh, m_View );

			if( VertexBuffer->GetNumVertices() > 0 )
			{
				XTRACE_NAMED( glDrawElements );

				DEBUGASSERT( IndexBuffer->GetNumIndices() > 0 );
				glDrawElements( IndexBuffer->GetPrimitiveType(), IndexBuffer->GetNumIndices(), GLINDEXFORMAT, NULL );
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

void GL2Renderer::Present()
{
	PROFILE_FUNCTION;

	XTRACE_FUNCTION;

#if BUILD_WINDOWS_NO_SDL
	const BOOL Success = SwapBuffers( m_Window->GetHDC() );
	Unused( Success );
	ASSERT( Success == TRUE );
#endif
#if BUILD_SDL
	SDL_GL_SwapWindow( m_Window->GetSDLWindow() );
#endif
}

void GL2Renderer::Clear( unsigned int Flags, unsigned int Color /*= 0xff000000*/, float Depth /*= 1.0f*/, unsigned int Stencil /*= 0*/ )
{
	XTRACE_FUNCTION;

	if( Flags == CLEAR_NONE )
	{
		return;
	}

	GLbitfield GLFlags = 0;
	if( Flags & CLEAR_COLOR )
	{
		GLFlags |= GL_COLOR_BUFFER_BIT;

		Vector4 ClearColor = Color;
		glClearColor( ClearColor.r, ClearColor.g, ClearColor.b, ClearColor.a );
	}

	// HACKHACK: GL global state strikes again! If the last material was disabling
	// depth writes, glClear won't write to depth buffer either. So fix that.
	// (NOTE: I could also maybe use glPush/PopAttrib( GL_DEPTH_BUFFER_BIT), but
	// the performance and support for that is unclear, and this works.)
	bool FixDepthMask = false;

	if( Flags & CLEAR_DEPTH )
	{
		GLFlags |= GL_DEPTH_BUFFER_BIT;
		glClearDepth( Depth );

		if( m_RenderState.m_ZWriteEnable == EZWE_False )
		{
			FixDepthMask = true;
		}
	}

	if( Flags & CLEAR_STENCIL )
	{
		GLFlags |= GL_STENCIL_BUFFER_BIT;
		glClearStencil( Stencil );
	}

	if( FixDepthMask )
	{
		glDepthMask( GL_TRUE );
	}

	glClear( GLFlags );

	if( FixDepthMask )
	{
		glDepthMask( GL_FALSE );
	}
}

IVertexBuffer* GL2Renderer::CreateVertexBuffer()
{
	return new GL2VertexBuffer;
}

IVertexDeclaration* GL2Renderer::CreateVertexDeclaration()
{
	return new GL2VertexDeclaration;
}

IIndexBuffer* GL2Renderer::CreateIndexBuffer()
{
	return new GL2IndexBuffer;
}

ITexture* GL2Renderer::CreateTexture( const char* Filename )
{
	XTRACE_FUNCTION;

	GL2Texture* const pTexture = new GL2Texture;
	pTexture->Initialize( Filename );
	return pTexture;
}

/*virtual*/ IVertexShader* GL2Renderer::CreateVertexShader( const SimpleString& Filename )
{
	GL2VertexShader* const pVertexShader = new GL2VertexShader;
	pVertexShader->Initialize( PackStream( Filename.CStr() ) );
	return pVertexShader;
}

/*virtual*/ IPixelShader* GL2Renderer::CreatePixelShader( const SimpleString& Filename )
{
	GL2PixelShader* const pPixelShader = new GL2PixelShader;
	pPixelShader->Initialize( PackStream( Filename.CStr() ) );
	return pPixelShader;
}

/*virtual*/ IShaderProgram* GL2Renderer::CreateShaderProgram( IVertexShader* const pVertexShader, IPixelShader* const pPixelShader, IVertexDeclaration* const pVertexDeclaration )
{
	GL2ShaderProgram* const pShaderProgram = new GL2ShaderProgram;
	pShaderProgram->Initialize( pVertexShader, pPixelShader, pVertexDeclaration );
	return pShaderProgram;
}

IRenderTarget* GL2Renderer::CreateRenderTarget( const SRenderTargetParams& Params )
{
	GL2RenderTarget* pTarget = new GL2RenderTarget;
	pTarget->Initialize( Params );
	m_RenderTargets.PushBack( pTarget );
	return pTarget;
}

void GL2Renderer::SetRenderTarget( IRenderTarget* RenderTarget )
{
	XTRACE_FUNCTION;

	m_CurrentRenderTarget = RenderTarget;

	const GLuint FrameBufferObject = *static_cast<GLuint*>( m_CurrentRenderTarget->GetHandle() );

	ASSERT( GLEW_ARB_framebuffer_object || GLEW_EXT_framebuffer_object );
	if( GLEW_ARB_framebuffer_object )
	{
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, FrameBufferObject );
	}
	else if( GLEW_EXT_framebuffer_object )
	{
		glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, FrameBufferObject );
	}

	// GL also requires manually setting the viewport for RTs.
	{
		ASSERT( m_Display );
		const uint ViewportWidth	= ( FrameBufferObject == 0 ) ? m_Display->m_Width : m_CurrentRenderTarget->GetWidth();
		const uint ViewportHeight	= ( FrameBufferObject == 0 ) ? m_Display->m_Height : m_CurrentRenderTarget->GetHeight();
		glViewport( 0, 0, ViewportWidth, ViewportHeight );
	}
}

bool GL2Renderer::IsValid()
{
	// GL has no concept of being invalid the way D3D does.
	return true;
}

void GL2Renderer::EnumerateDisplayModes( Array<SDisplayMode>& DisplayModes )
{
	Display::EnumerateDisplayModes( DisplayModes );
}

/*virtual*/ void GL2Renderer::SaveScreenshot( const SimpleString& Filename )
{
	// TODO PORT LATER SaveScreenshot
	Unused( Filename );
}

/*virtual*/ void GL2Renderer::SetVertexShader( IVertexShader* const pVertexShader )
{
	// Not supported in GL. Use SetShaderProgram.
	Unused( pVertexShader );
}

/*virtual*/ void GL2Renderer::SetPixelShader( IPixelShader* const pPixelShader )
{
	// Not supported in GL. Use SetShaderProgram.
	Unused( pPixelShader );
}

/*virtual*/ void GL2Renderer::SetShaderProgram( IShaderProgram* const pShaderProgram )
{
	DEBUGASSERT( pShaderProgram );

#if IGNORE_REDUNDANT_STATE
	if( pShaderProgram == m_ShaderProgram )
	{
		return;
	}
#endif
	m_ShaderProgram = pShaderProgram;

	const GLuint ShaderProgram = *static_cast<GLuint*>( pShaderProgram->GetHandle() );
	glUseProgram( ShaderProgram );
}

/*virtual*/ SimpleString GL2Renderer::GetShaderType() const
{
	return "GLSL";
}

void GL2Renderer::glSetEnabled( const GLenum Cap, const bool Enabled )
{
	if( Enabled )
	{
		glEnable( Cap );
	}
	else
	{
		glDisable( Cap );
	}
}

static GLenum GLCullMode[] =
{
	0,			//	ECM_Unknown
	0,
	GL_BACK,
	GL_FRONT,
};

/*virtual*/ void GL2Renderer::SetCullMode( const ECullMode CullMode )
{
	DEBUGASSERT( CullMode > ECM_Unknown );

#if IGNORE_REDUNDANT_STATE
	if( CullMode == m_RenderState.m_CullMode )
	{
		return;
	}
#endif
	m_RenderState.m_CullMode = CullMode;

	if( CullMode == ECM_None )
	{
		glDisable( GL_CULL_FACE );
	}
	else
	{
		glEnable( GL_CULL_FACE );
		glCullFace( GLCullMode[ CullMode ] );
	}
}

/*virtual*/ void GL2Renderer::SetZEnable( const EZEnable ZEnable )
{
	DEBUGASSERT( ZEnable > EZE_Unknown );

#if IGNORE_REDUNDANT_STATE
	if( ZEnable == m_RenderState.m_ZEnable )
	{
		return;
	}
#endif
	m_RenderState.m_ZEnable = ZEnable;

	glSetEnabled( GL_DEPTH_TEST, ( ZEnable == EZE_True ) );
}

/*virtual*/ void GL2Renderer::SetZWriteEnable( const EZWriteEnable ZWriteEnable )
{
	DEBUGASSERT( ZWriteEnable > EZWE_Unknown );

#if IGNORE_REDUNDANT_STATE
	if( ZWriteEnable == m_RenderState.m_ZWriteEnable )
	{
		return;
	}
#endif
	m_RenderState.m_ZWriteEnable = ZWriteEnable;

	glDepthMask( ( ZWriteEnable == EZWE_True ) ? GL_TRUE : GL_FALSE );
}

/*virtual*/ void GL2Renderer::SetAlphaBlendEnable( const EAlphaBlendEnable AlphaBlendEnable )
{
	DEBUGASSERT( AlphaBlendEnable > EABE_Unknown );

#if IGNORE_REDUNDANT_STATE
	if( AlphaBlendEnable == m_RenderState.m_AlphaBlendEnable )
	{
		return;
	}
#endif
	m_RenderState.m_AlphaBlendEnable = AlphaBlendEnable;

	glSetEnabled( GL_BLEND, ( AlphaBlendEnable == EABE_True ) );
}

static GLenum GLBlend[] =
{
	0,						//	EB_Unknown
	GL_ZERO,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA_SATURATE,
};

/*virtual*/ void GL2Renderer::SetBlend( const EBlend SrcBlend, const EBlend DestBlend )
{
	DEBUGASSERT( SrcBlend > EB_Unknown );
	DEBUGASSERT( DestBlend > EB_Unknown );

#if IGNORE_REDUNDANT_STATE
	if( SrcBlend == m_RenderState.m_SrcBlend &&
		DestBlend == m_RenderState.m_DestBlend )
	{
		return;
	}
#endif
	m_RenderState.m_SrcBlend = SrcBlend;
	m_RenderState.m_DestBlend = DestBlend;

	glBlendFunc( GLBlend[ SrcBlend ], GLBlend[ DestBlend ] );
}

/*virtual*/ void GL2Renderer::SetTexture( const uint SamplerStage, ITexture* const pTexture )
{
	DEBUGASSERT( SamplerStage < MAX_TEXTURE_STAGES );
	DEBUGASSERT( pTexture );

	// This part is necessary even if the state is redundant, because it
	// controls which sampler state is affected by glTexParameter calls.
	glActiveTexture( GL_TEXTURE0 + SamplerStage );

	SSamplerState& SamplerState = m_SamplerStates[ SamplerStage ];

#if IGNORE_REDUNDANT_STATE
	if( pTexture == SamplerState.m_Texture )
	{
		return;
	}
#endif
	SamplerState.m_Texture = pTexture;

	const GLuint Texture = *static_cast<GLuint*>( pTexture->GetHandle() );
	glBindTexture( GL_TEXTURE_2D, Texture );
}

static GLenum GLTextureAddress[] =
{
	0,					//	ETA_Unknown
	GL_REPEAT,
	GL_MIRRORED_REPEAT,
	GL_CLAMP_TO_EDGE,
};

/*virtual*/ void GL2Renderer::SetAddressing( const uint SamplerStage, const ETextureAddress AddressU, const ETextureAddress AddressV )
{
	DEBUGASSERT( SamplerStage < MAX_TEXTURE_STAGES );
	DEBUGASSERT( AddressU > ETA_Unknown );
	DEBUGASSERT( AddressV > ETA_Unknown );

	SSamplerState& OuterSamplerState = m_SamplerStates[ SamplerStage ];

	DEBUGASSERT( OuterSamplerState.m_Texture );
	GL2Texture* const	pTexture		= static_cast<GL2Texture*>( OuterSamplerState.m_Texture );
	SSamplerState&		SamplerState	= pTexture->GetSamplerState();

#if IGNORE_REDUNDANT_SAMPLER_STATE
	if( AddressU == SamplerState.m_AddressU &&
		AddressV == SamplerState.m_AddressV )
	{
		return;
	}
#endif
	SamplerState.m_AddressU = AddressU;
	SamplerState.m_AddressV = AddressV;

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GLTextureAddress[ AddressU ] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GLTextureAddress[ AddressV ] );
}

static GLenum GLMinMipFilters[][ ETF_SIZE ] =
{	//	ETF_Unknown,	ETF_None,	ETF_Point,					ETF_Linear,					ETF_Anisotropic (mip)
	{	0,				0,			0,							0,							0, },			// ETF_Unknown (min)
	{	0,				0,			0,							0,							0, },			// ETF_None (min)
	{	0,				GL_NEAREST,	GL_NEAREST_MIPMAP_NEAREST,	GL_NEAREST_MIPMAP_LINEAR,	0, },			// ETF_Point (min)
	{	0,				GL_LINEAR,	GL_LINEAR_MIPMAP_NEAREST,	GL_LINEAR_MIPMAP_LINEAR,	0, },			// ETF_Linear (min)
	{	0,				0,			0,							0,							0, },			// ETF_Anisotropic (min) (not supported on GL)
};

/*virtual*/ void GL2Renderer::SetMinMipFilters( const uint SamplerStage, const ETextureFilter MinFilter, const ETextureFilter MipFilter )
{
	DEBUGASSERT( SamplerStage < MAX_TEXTURE_STAGES );
	DEBUGASSERT( MinFilter > ETF_None );
	DEBUGASSERT( MipFilter > ETF_Unknown );

	SSamplerState& OuterSamplerState = m_SamplerStates[ SamplerStage ];

	DEBUGASSERT( OuterSamplerState.m_Texture );
	GL2Texture* const	pTexture		= static_cast<GL2Texture*>( OuterSamplerState.m_Texture );
	SSamplerState&		SamplerState	= pTexture->GetSamplerState();

#if IGNORE_REDUNDANT_SAMPLER_STATE
	if( MinFilter == SamplerState.m_MinFilter &&
		MipFilter == SamplerState.m_MipFilter )
	{
		return;
	}
#endif
	SamplerState.m_MinFilter = MinFilter;
	SamplerState.m_MipFilter = MipFilter;

	const GLenum GLMinMipFilter = GLMinMipFilters[ MinFilter ][ MipFilter ];
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GLMinMipFilter );
}

static GLenum GLMagFilters[] =
{
	0,			// ETF_Unknown
	0,			// ETF_None
	GL_NEAREST,
	GL_LINEAR,
	0,			// ETF_Anisotropic (not supported on GL)
};

/*virtual*/ void GL2Renderer::SetMagFilter( const uint SamplerStage, const ETextureFilter MagFilter )
{
	DEBUGASSERT( SamplerStage < MAX_TEXTURE_STAGES );
	DEBUGASSERT( MagFilter > ETF_None );

	SSamplerState& OuterSamplerState = m_SamplerStates[ SamplerStage ];

	DEBUGASSERT( OuterSamplerState.m_Texture );
	GL2Texture* const	pTexture		= static_cast<GL2Texture*>( OuterSamplerState.m_Texture );
	SSamplerState&		SamplerState	= pTexture->GetSamplerState();

#if IGNORE_REDUNDANT_SAMPLER_STATE
	if( MagFilter == SamplerState.m_MagFilter )
	{
		return;
	}
#endif
	SamplerState.m_MagFilter = MagFilter;

	const GLenum GLMagFilter = GLMagFilters[ MagFilter ];
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GLMagFilter );
}

/*virtual*/ void GL2Renderer::SetVertexShaderFloat4( const HashedString& Parameter, const float* const pFloats, const uint NumFloat4s )
{
	SetPixelShaderFloat4( Parameter, pFloats, NumFloat4s );
}

/*virtual*/ void GL2Renderer::SetVertexShaderMatrix( const HashedString& Parameter, const float* const pFloats, const uint NumMatrices )
{
	SetPixelShaderMatrix( Parameter, pFloats, NumMatrices );
}

/*virtual*/ void GL2Renderer::SetPixelShaderFloat4( const HashedString& Parameter, const float* const pFloats, const uint NumFloat4s )
{
	ASSERT( m_ShaderProgram );

	uint Register;
	if( !m_ShaderProgram->GetPixelShaderRegister( Parameter, Register ) )
	{
		return;
	}

	glUniform4fv( Register, NumFloat4s, pFloats );
	GLERRORCHECK;
}

/*virtual*/ void GL2Renderer::SetPixelShaderMatrix( const HashedString& Parameter, const float* const pFloats, const uint NumMatrices )
{
	ASSERT( m_ShaderProgram );

	uint Register;
	if( !m_ShaderProgram->GetPixelShaderRegister( Parameter, Register ) )
	{
		return;
	}

	// HACKHACK: If we're passing in a multiple of 4 float4s, assume it's a matrix.
	// (GL doesn't accept multiples of float4s unless the GLSL variable was an array.)
	// Also, transpose matrices while we're at it.
	const GLboolean Transpose = GL_TRUE;
	glUniformMatrix4fv( Register, NumMatrices, Transpose, pFloats );
	GLERRORCHECK;
}