#ifndef GL2RENDERER_H
#define GL2RENDERER_H

#include "renderercommon.h"
#include "gl2.h"

class Window;

#if BUILD_SDL
#include "SDL2/SDL.h"
#endif

class GL2Renderer : public RendererCommon
{
public:
	GL2Renderer( Window* const pWindow );
	virtual ~GL2Renderer();

	virtual void	Initialize();
	virtual void	Tick();
	virtual void	Clear( unsigned int Flags = CLEAR_COLOR | CLEAR_DEPTH | CLEAR_STENCIL, unsigned int Color = 0xff000000, float Depth = 1.0f, unsigned int Stencil = 0 );
	virtual void	Present();
	virtual bool	CanReset();
	virtual bool	Reset();
	virtual void	Refresh();

	virtual void	SetVertexShader( IVertexShader* const pVertexShader );
	virtual void	SetPixelShader( IPixelShader* const pPixelShader );
	virtual void	SetShaderProgram( IShaderProgram* const pShaderProgram );

	virtual SimpleString	GetShaderType() const;

	virtual void	SetCullMode( const ECullMode CullMode );
	virtual void	SetZEnable( const EZEnable ZEnable );
	virtual void	SetZWriteEnable( const EZWriteEnable ZWriteEnable );
	virtual void	SetAlphaBlendEnable( const EAlphaBlendEnable AlphaBlendEnable );
	virtual void	SetBlend( const EBlend SrcBlend, const EBlend DestBlend );

	virtual void	SetTexture( const uint SamplerStage, ITexture* const pTexture );
	virtual void	SetAddressing( const uint SamplerStage, const ETextureAddress AddressU, const ETextureAddress AddressV );
	virtual void	SetMinMipFilters( const uint SamplerStage, const ETextureFilter MinFilter, const ETextureFilter MipFilter );
	virtual void	SetMagFilter( const uint SamplerStage, const ETextureFilter MagFilter );

	virtual void	SetVertexShaderFloat4( const HashedString& Parameter, const float* const pFloats, const uint NumFloat4s );
	virtual void	SetVertexShaderMatrix( const HashedString& Parameter, const float* const pFloats, const uint NumMatrices );
	virtual void	SetPixelShaderFloat4( const HashedString& Parameter, const float* const pFloats, const uint NumFloat4s );
	virtual void	SetPixelShaderMatrix( const HashedString& Parameter, const float* const pFloats, const uint NumMatrices );

	virtual float	GetPixelGridOffset() const { return 0.0f; }

	virtual void	SetRestoreDeviceCallback( const SRestoreDeviceCallback& Callback );

	virtual void	SetFullScreen( bool FullScreen );

	virtual void	SetMultiSampleType( EMultiSampleType MultiSampleType );
	virtual void	GetBestSupportedMultiSampleType( EMultiSampleType& OutMultiSampleType, uint32* pOutQualityLevels = NULL );

	virtual ERenderTargetFormat	GetBestSupportedRenderTargetFormat( const ERenderTargetFormat Format ) const;

	virtual IVertexBuffer*		CreateVertexBuffer();
	virtual IVertexDeclaration*	CreateVertexDeclaration();
	virtual IIndexBuffer*		CreateIndexBuffer();
	virtual ITexture*			CreateTexture( const char* Filename );
	virtual IVertexShader*		CreateVertexShader( const SimpleString& Filename );
	virtual IPixelShader*		CreatePixelShader( const SimpleString& Filename );
	virtual IShaderProgram*		CreateShaderProgram( IVertexShader* const pVertexShader, IPixelShader* const pPixelShader, IVertexDeclaration* const pVertexDeclaration );
	virtual IRenderTarget*		CreateRenderTarget( const SRenderTargetParams& Params );

	virtual void			SetRenderTarget( IRenderTarget* RenderTarget );

	virtual void			SaveScreenshot( const SimpleString& Filename );

	virtual bool	IsValid();

	virtual void	EnumerateDisplayModes( Array<SDisplayMode>& DisplayModes );

	virtual bool	SupportsSM3();
	virtual bool	SupportsSM2();

	virtual void*	GetDevice();

protected:
	void			glSetEnabled( const GLenum Cap, const bool Enabled );
	virtual void	RenderBucket( Bucket* pBucket, const ViewportPass* const pViewportPass );

	virtual void	SetViewport( const ViewportPass* const pViewportPass );

	Window*			m_Window;

#if BUILD_WINDOWS_NO_SDL
	HGLRC			m_RenderContext;
#endif
#if BUILD_SDL
	SDL_GLContext	m_RenderContext;
#endif

	GLint			m_MaxVertexAttribs;
};

#endif // GL2RENDERER_H
