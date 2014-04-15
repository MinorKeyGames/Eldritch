#ifndef GL2RENDERTARGET_H
#define GL2RENDERTARGET_H

#include "irendertarget.h"
#include "gl2.h"

class GL2RenderTarget : public IRenderTarget
{
public:
	GL2RenderTarget();
	virtual ~GL2RenderTarget();

	virtual void		Initialize( const SRenderTargetParams& Params );

	virtual void		Release();
	virtual void		Reset();

	virtual uint		GetWidth() { return m_Width; }
	virtual uint		GetHeight() { return m_Height; }

	virtual void*		GetHandle();
	virtual void*		GetColorRenderTargetHandle();
	virtual void*		GetDepthStencilRenderTargetHandle();
	virtual ITexture*	GetColorTextureHandle();
	virtual ITexture*	GetDepthStencilTextureHandle();

private:
	GLuint		m_FrameBufferObject;
	GLuint		m_ColorTextureObject;
	ITexture*	m_ColorTexture;
	GLuint		m_DepthStencilRenderBufferObject;

	uint		m_Width;
	uint		m_Height;
};

#endif // GL2RENDERTARGET_H