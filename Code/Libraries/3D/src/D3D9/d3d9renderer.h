#ifndef D3D9RENDERER_H
#define D3D9RENDERER_H

// This class encapsulates the D3D interface and one device.
// In the future, I may want to separate those concepts...

#include "renderercommon.h"

#include <Windows.h>
#include <d3d9.h>

class ShaderManager;
class TextureManager;
class FontManager;
class MeshFactory;
class D3D9Shader;

class D3D9Renderer : public RendererCommon
{
public:
	virtual ~D3D9Renderer();
	D3D9Renderer( HWND hWnd, bool Fullscreen );

	virtual void	Initialize();
	virtual void	Tick();
	virtual void	Clear( unsigned int Flags = CLEAR_COLOR | CLEAR_DEPTH | CLEAR_STENCIL, unsigned int Color = 0xff000000, float Depth = 1.0f, unsigned int Stencil = 0 );
	virtual void	Present();
	virtual bool	CanReset();
	virtual bool	Reset();
	virtual void	Refresh() { /* Unused */ }

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

	virtual float	GetPixelGridOffset() const { return 0.5f; }

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

	void					TestCapabilities();

	virtual bool	IsValid();

	virtual void	EnumerateDisplayModes( Array<SDisplayMode>& DisplayModes );

	virtual bool	SupportsSM3();
	virtual bool	SupportsSM2();

	virtual void*	GetDevice();

protected:
	D3D9Renderer() {}

	virtual void	RenderBucket( Bucket* pBucket, const ViewportPass* const pViewportPass );

	void	EnumerateDisplayModes( Array<SDisplayMode>& DisplayModes, D3DFORMAT Format );

	void	CreateDefaultRenderTarget();
	void	PreReset();
	void	PostReset();
	void	GetPresentParams( D3DPRESENT_PARAMETERS& Params );

	virtual void	SetViewport( const ViewportPass* const pViewportPass );

	IDirect3D9*			m_D3D;
	IDirect3DDevice9*	m_D3DDevice;

	bool					m_DeviceLost;
	SRestoreDeviceCallback	m_RestoreDeviceCallback;

	HWND				m_hWnd;
	bool				m_Fullscreen;
	EMultiSampleType	m_MultiSampleType;
};

#endif // D3D9RENDERER_H