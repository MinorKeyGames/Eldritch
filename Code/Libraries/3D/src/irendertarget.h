#ifndef IRENDERTARGET_H
#define IRENDERTARGET_H

enum ERenderTargetFormat
{
	ERTF_Unknown,
	ERTF_None,
	ERTF_UseDefault,
	ERTF_X8R8G8B8,
	ERTF_A8R8G8B8,
	ERTF_A16B16G16R16,
	ERTF_A16B16G16R16F,
	ERTF_A32B32G32R32F,
	ERTF_R32F,
	ERTF_D24S8,
};

struct SRenderTargetParams
{
	SRenderTargetParams(
		int Width = 0,
		int Height = 0,
		ERenderTargetFormat ColorFormat = ERTF_Unknown,
		ERenderTargetFormat DepthStencilFormat = ERTF_Unknown,
		bool AutoGenMipMaps = false )
		:	Width( Width )
		,	Height( Height )
		,	ColorFormat( ColorFormat )
		,	DepthStencilFormat( DepthStencilFormat )
		,	AutoGenMipMaps( AutoGenMipMaps ) {}
	int				Width;
	int				Height;
	ERenderTargetFormat	ColorFormat;
	ERenderTargetFormat	DepthStencilFormat;
	bool			AutoGenMipMaps;
};

class ITexture;

class IRenderTarget
{
public:
	virtual ~IRenderTarget() {}

	virtual void		Initialize( const SRenderTargetParams& Params ) = 0;

	virtual void		Release() = 0;
	virtual void		Reset() = 0;

	virtual uint		GetWidth() = 0;
	virtual uint		GetHeight() = 0;

	virtual void*		GetHandle() = 0;
	virtual void*		GetColorRenderTargetHandle() = 0;
	virtual void*		GetDepthStencilRenderTargetHandle() = 0;
	virtual ITexture*	GetColorTextureHandle() = 0;
	virtual ITexture*	GetDepthStencilTextureHandle() = 0;
};

#endif // IRENDERTARGET_H