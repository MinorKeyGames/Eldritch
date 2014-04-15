#include "core.h"
#include "gl2rendertarget.h"
#include "gl2texture.h"

GLenum GetGLFormat( const ERenderTargetFormat Format )
{
	switch( Format )
	{
	case ERTF_Unknown:
		return 0;
	case ERTF_X8R8G8B8:
		return GL_RGBA8;	// Meh?
	case ERTF_A8R8G8B8:
		return GL_RGBA8;
	case ERTF_A16B16G16R16:
		return GL_RGBA16;
	case ERTF_A16B16G16R16F:
		ASSERT( GLEW_ARB_texture_float );
		return GL_RGBA16F_ARB;
	case ERTF_A32B32G32R32F:
		ASSERT( GLEW_ARB_texture_float );
		return GL_RGBA32F_ARB;
	case ERTF_R32F:
		ASSERT( GLEW_ARB_texture_float );
		return GL_INTENSITY32F_ARB;
	case ERTF_D24S8:
		return GL_DEPTH24_STENCIL8;
	case ERTF_UseDefault:
		// HACKHACK: GL doesn't support binding to the default depth buffer,
		// so promote ERTF_UseDefault to a reasonable default format.
		// NOTE: This won't work if the pass(es) associated with this target
		// needed to use data already in the buffer. It works for Eldritch
		// because the buffer was always cleared with each target, and reusing
		// the default was merely an optimization.
		return GL_DEPTH24_STENCIL8;
	default:
		WARNDESC( "GL texture format not matched" );
		return 0;
	}
}

GL2RenderTarget::GL2RenderTarget()
:	m_FrameBufferObject( 0 )
,	m_ColorTextureObject( 0 )
,	m_ColorTexture( NULL )
,	m_DepthStencilRenderBufferObject( 0 )
,	m_Width( 0 )
,	m_Height( 0 )
{
}

GL2RenderTarget::~GL2RenderTarget()
{
	// Don't clean up m_ColorTextureObject; deleting the texture does that.
	SafeDelete( m_ColorTexture );

	if( m_DepthStencilRenderBufferObject != 0 )
	{
		if( GLEW_ARB_framebuffer_object )
		{
			glDeleteRenderbuffers( 1, &m_DepthStencilRenderBufferObject );
		}
		else if( GLEW_EXT_framebuffer_object )
		{
			glDeleteRenderbuffersEXT( 1, &m_DepthStencilRenderBufferObject );
		}
	}

	if( m_FrameBufferObject != 0 )
	{
		if( GLEW_ARB_framebuffer_object )
		{
			glDeleteFramebuffers( 1, &m_FrameBufferObject );
		}
		else if( GLEW_EXT_framebuffer_object )
		{
			glDeleteFramebuffersEXT( 1, &m_FrameBufferObject );
		}
	}
}

/*virtual*/ void GL2RenderTarget::Initialize( const SRenderTargetParams& Params )
{
	XTRACE_FUNCTION;

	m_Width		= Params.Width;
	m_Height	= Params.Height;

	// FBOs were supported in GL 2.1 only by extension, but some newer drivers
	// don't still support that extension. Use whichever is available.
	ASSERT( GLEW_EXT_framebuffer_object || GLEW_ARB_framebuffer_object );

	if( GLEW_ARB_framebuffer_object )
	{
		glGenFramebuffers( 1, &m_FrameBufferObject );
		ASSERT( m_FrameBufferObject != 0 );
		glBindFramebuffer( GL_FRAMEBUFFER, m_FrameBufferObject );
	}
	else if( GLEW_EXT_framebuffer_object )
	{
		glGenFramebuffersEXT( 1, &m_FrameBufferObject );
		ASSERT( m_FrameBufferObject != 0 );
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_FrameBufferObject );
	}

	if( Params.ColorFormat != ERTF_None )
	{
		glGenTextures( 1, &m_ColorTextureObject );
		ASSERT( m_ColorTextureObject != 0 );
		glBindTexture( GL_TEXTURE_2D, m_ColorTextureObject );
		const GLenum ColorFormat = GetGLFormat( Params.ColorFormat );
		// The image format parameters don't necessarily match the color format,
		// but it doesn't matter because we're not providing image data here.
		glTexImage2D( GL_TEXTURE_2D, 0, ColorFormat, Params.Width, Params.Height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL );
		m_ColorTexture = new GL2Texture( m_ColorTextureObject );
	}

	if( Params.DepthStencilFormat != ERTF_None )
	{
		if( GLEW_ARB_framebuffer_object )
		{
			glGenRenderbuffers( 1, &m_DepthStencilRenderBufferObject );
			ASSERT( m_DepthStencilRenderBufferObject != 0 );
			glBindRenderbuffer( GL_RENDERBUFFER, m_DepthStencilRenderBufferObject );
			const GLenum DepthStencilFormat = GetGLFormat( Params.DepthStencilFormat );
			glRenderbufferStorage( GL_RENDERBUFFER, DepthStencilFormat, Params.Width, Params.Height );
		}
		else if( GLEW_EXT_framebuffer_object )
		{
			glGenRenderbuffersEXT( 1, &m_DepthStencilRenderBufferObject );
			ASSERT( m_DepthStencilRenderBufferObject != 0 );
			glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, m_DepthStencilRenderBufferObject );
			const GLenum DepthStencilFormat = GetGLFormat( Params.DepthStencilFormat );
			glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, DepthStencilFormat, Params.Width, Params.Height );
		}
	}

	if( GLEW_ARB_framebuffer_object )
	{
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTextureObject, 0 );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthStencilRenderBufferObject );

		const GLenum FrameBufferStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
		ASSERT( FrameBufferStatus == GL_FRAMEBUFFER_COMPLETE );
		Unused( FrameBufferStatus );
	}
	else if( GLEW_EXT_framebuffer_object )
	{
        glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_ColorTextureObject, 0 );
		glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_DepthStencilRenderBufferObject );
		glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_DepthStencilRenderBufferObject );

		const GLenum FrameBufferStatus = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
		ASSERT( FrameBufferStatus == GL_FRAMEBUFFER_COMPLETE_EXT );
		Unused( FrameBufferStatus );
	}
}

void GL2RenderTarget::Release()
{
}

void GL2RenderTarget::Reset()
{
}

/*virtual*/ void* GL2RenderTarget::GetHandle()
{
	return &m_FrameBufferObject;
}

/*virtual*/ void* GL2RenderTarget::GetColorRenderTargetHandle()
{
	return &m_ColorTextureObject;
}

/*virtual*/ void* GL2RenderTarget::GetDepthStencilRenderTargetHandle()
{
	return &m_DepthStencilRenderBufferObject;
}

/*virtual*/ ITexture* GL2RenderTarget::GetColorTextureHandle()
{
	return m_ColorTexture;
}

/*virtual*/ ITexture* GL2RenderTarget::GetDepthStencilTextureHandle()
{
	// Unsupported on GL. Cannot sample from depth stencil.
	return NULL;
}