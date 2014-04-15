#ifndef D3D9RENDERTARGET_H
#define D3D9RENDERTARGET_H

#include "irendertarget.h"

// Included for D3DFORMAT, which can't be forward declared because C++.
#include <d3d9.h>

struct IDirect3DDevice9;
struct IDirect3DSurface9;
class IRenderer;

class D3D9RenderTarget : public IRenderTarget
{
public:
	D3D9RenderTarget( IRenderer* pRenderer, IDirect3DDevice9* D3DDevice );
	D3D9RenderTarget( IRenderer* pRenderer, IDirect3DDevice9* D3DDevice, IDirect3DSurface9* ColorSurface, IDirect3DSurface9* DepthStencilSurface );
	virtual ~D3D9RenderTarget();

	virtual void		Initialize( const SRenderTargetParams& Params );

	virtual void		Release();
	virtual void		Reset();
	void				Reset( IDirect3DSurface9* ColorSurface, IDirect3DSurface9* DepthStencilSurface );

	virtual uint		GetWidth() { return m_Params.Width; }
	virtual uint		GetHeight() { return m_Params.Height; }

	virtual void*		GetHandle();
	virtual void*		GetColorRenderTargetHandle();
	virtual void*		GetDepthStencilRenderTargetHandle();
	virtual ITexture*	GetColorTextureHandle();
	virtual ITexture*	GetDepthStencilTextureHandle();

	static D3DFORMAT	GetD3DFormat( const ERenderTargetFormat Format );

protected:
	void				BuildRenderTarget();

	SRenderTargetParams	m_Params;
	IRenderer*			m_Renderer;
	IDirect3DDevice9*	m_D3DDevice;
	ITexture*			m_ColorTexture;
	ITexture*			m_DepthStencilTexture;
	IDirect3DSurface9*	m_ColorSurface;
	IDirect3DSurface9*	m_DepthStencilSurface;
};

#endif // D3D9RENDERTARGET_H