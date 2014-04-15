#ifndef IRENDERER_H
#define IRENDERER_H

#include "3d.h"
#include "array.h"
#include "display.h"
#include "irendertarget.h"
#include "renderstates.h"

class Display;
class Matrix;
class View;
class IVertexBuffer;
class IVertexDeclaration;
class IIndexBuffer;
class Mesh;
class ITexture;
class IVertexShader;
class IPixelShader;
class IShaderProgram;
class IRenderTarget;
class Vector;
class ShaderManager;
class TextureManager;
class FontManager;
class VertexDeclarationManager;
class MeshFactory;
class Bucket;
class Font;
class SimpleString;
class Clock;
class ViewportPass;
class Angles;
class Vector4;

struct SRect;
struct SRenderTargetParams;

enum EMultiSampleType
{
	EMST_None,
	//EMST_NONMASKABLE,
	EMST_2X,
	EMST_4X,
	EMST_8X,
};

class IRenderer
{
public:
	virtual ~IRenderer() {}

	virtual void	Initialize() = 0;
	virtual void	Tick() = 0;
	virtual void	Clear( unsigned int Flags = CLEAR_COLOR | CLEAR_DEPTH | CLEAR_STENCIL, unsigned int Color = 0xff000000, float Depth = 1.0f, unsigned int Stencil = 0 ) = 0;
	virtual void	Present() = 0;
	virtual bool	CanReset() = 0;
	virtual bool	Reset() = 0;
	virtual void	Refresh() = 0;

	virtual void	SetVertexShader( IVertexShader* const pVertexShader ) = 0;
	virtual void	SetPixelShader( IPixelShader* const pPixelShader ) = 0;
	virtual void	SetShaderProgram( IShaderProgram* const pShaderProgram ) = 0;

	virtual SimpleString	GetShaderType() const = 0;

	virtual void	SetCullMode( const ECullMode CullMode ) = 0;
	virtual void	SetZEnable( const EZEnable ZEnable ) = 0;
	virtual void	SetZWriteEnable( const EZWriteEnable ZWriteEnable ) = 0;
	virtual void	SetAlphaBlendEnable( const EAlphaBlendEnable AlphaBlendEnable ) = 0;
	virtual void	SetBlend( const EBlend SrcBlend, const EBlend DestBlend ) = 0;

	virtual void	SetTexture( const uint SamplerStage, ITexture* const pTexture ) = 0;
	virtual void	SetAddressing( const uint SamplerStage, const ETextureAddress AddressU, const ETextureAddress AddressV ) = 0;
	virtual void	SetMinMipFilters( const uint SamplerStage, const ETextureFilter MinFilter, const ETextureFilter MipFilter ) = 0;
	virtual void	SetMagFilter( const uint SamplerStage, const ETextureFilter MagFilter ) = 0;

	virtual void	SetVertexShaderFloat4( const HashedString& Parameter, const float* const pFloats, const uint NumFloat4s ) = 0;
	virtual void	SetVertexShaderMatrix( const HashedString& Parameter, const float* const pFloats, const uint NumMatrices ) = 0;
	virtual void	SetPixelShaderFloat4( const HashedString& Parameter, const float* const pFloats, const uint NumFloat4s ) = 0;
	virtual void	SetPixelShaderMatrix( const HashedString& Parameter, const float* const pFloats, const uint NumMatrices ) = 0;

	virtual float	GetPixelGridOffset() const = 0;

	typedef void ( *RestoreDeviceCallback )( void* );
	struct SRestoreDeviceCallback
	{
		SRestoreDeviceCallback()
		:	m_Callback( NULL )
		,	m_Void( NULL )
		{
		}

		RestoreDeviceCallback	m_Callback;
		void*					m_Void;
	};
	virtual void	SetRestoreDeviceCallback( const SRestoreDeviceCallback& Callback ) = 0;

	virtual void	SetFullScreen( bool FullScreen ) = 0;

	virtual void	SetMultiSampleType( EMultiSampleType MultiSampleType ) = 0;
	virtual void	GetBestSupportedMultiSampleType( EMultiSampleType& OutMultiSampleType, uint32* pOutQualityLevels = NULL ) = 0;

	virtual ERenderTargetFormat	GetBestSupportedRenderTargetFormat( const ERenderTargetFormat Format ) const = 0;

	virtual void	SetWorldMatrix( const Matrix& WorldMatrix ) = 0;
	virtual void	SetViewMatrix( const Matrix& ViewMatrix ) = 0;
	virtual void	SetProjectionMatrix( const Matrix& ProjectionMatrix ) = 0;
	virtual Matrix	GetWorldMatrix() = 0;
	virtual Matrix	GetViewMatrix() = 0;
	virtual Matrix	GetProjectionMatrix() = 0;

	virtual void	AddMesh( Mesh* pMesh ) = 0;
	virtual void	AddBucket( const HashedString& Name, Bucket* pBucket ) = 0;	// Add (ordered) buckets for doing different render passes. Renderer owns memory.
	virtual Bucket*	GetBucket( const HashedString& Name ) = 0;
	virtual Bucket*	GetBucket( uint Index ) = 0;		// This is pretty ugly, may want to refactor later (I'm just using it for debug stuff at the moment)
	virtual void	FreeBuckets() = 0;
	virtual void	FlushBuckets() = 0;

	// Viewport pass defines a viewport rectangle and a map of view overrides.
	virtual void			AddViewportPass( ViewportPass* const pViewportPass ) = 0;
	virtual void			FreeViewportPasses() = 0;

	virtual IVertexBuffer*		CreateVertexBuffer() = 0;
	virtual void				AddDynamicVertexBuffer( IVertexBuffer* pBuffer ) = 0;
	virtual void				RemoveDynamicVertexBuffer( IVertexBuffer* pBuffer ) = 0;
	virtual void				ClearDynamicVertexBuffers() = 0;
	virtual IVertexDeclaration*	GetVertexDeclaration( uint VertexSignature ) = 0;
	virtual IVertexDeclaration*	CreateVertexDeclaration() = 0;
	virtual IIndexBuffer*		CreateIndexBuffer() = 0;
	virtual ITexture*			CreateTexture( const char* Filename ) = 0;
	virtual IVertexShader*		CreateVertexShader( const SimpleString& Filename ) = 0;
	virtual IPixelShader*		CreatePixelShader( const SimpleString& Filename ) = 0;
	virtual IShaderProgram*		CreateShaderProgram( IVertexShader* const pVertexShader, IPixelShader* const pPixelShader, IVertexDeclaration* const pVertexDeclaration ) = 0;
	virtual IRenderTarget*		CreateRenderTarget( const SRenderTargetParams& Params ) = 0;
	virtual void				FreeRenderTargets() = 0;

	virtual void			SetRenderTarget( IRenderTarget* RenderTarget ) = 0;
	virtual IRenderTarget*	GetCurrentRenderTarget() = 0;
	virtual IRenderTarget*	GetDefaultRenderTarget() = 0;

	virtual void			SaveScreenshot( const SimpleString& Filename ) = 0;

#if BUILD_DEV
	virtual void	DEBUGDrawLine( const Vector& Start, const Vector& End, unsigned int Color ) = 0;
	virtual void	DEBUGDrawTriangle( const Vector& V1, const Vector& V2, const Vector& V3, unsigned int Color ) = 0;
	virtual void	DEBUGDrawBox( const Vector& Min, const Vector& Max, unsigned int Color ) = 0;
	virtual void	DEBUGDrawFrustum( const View& rView, unsigned int Color ) = 0;
	virtual void	DEBUGDrawSphere( const Vector& Center, float Radius, unsigned int Color ) = 0;
	virtual void	DEBUGDrawEllipsoid( const Vector& Center, const Vector& Extents, unsigned int Color ) = 0;
	virtual void	DEBUGDrawCross( const Vector& Center, const float Length, unsigned int Color ) = 0;
	virtual void	DEBUGDrawArrow( const Vector& Root, const Angles& Direction, const float Length, unsigned int Color ) = 0;

	// 2D versions draw to the HUD instead of the world.
	virtual void	DEBUGDrawLine2D( const Vector& Start, const Vector& End, unsigned int Color ) = 0;
	virtual void	DEBUGDrawBox2D( const Vector& Min, const Vector& Max, unsigned int Color ) = 0;

	virtual void	DEBUGPrint( const SimpleString& UTF8String, const Font* const pFont, const SRect& Bounds, const Vector4& Color ) = 0;
#endif // BUILD_DEV

	// Flags are defined in font.h
	virtual Mesh*	Print( const SimpleString& UTF8String, const Font* const pFont, const SRect& Bounds, unsigned int Flags ) = 0;

	virtual ShaderManager*				GetShaderManager() = 0;
	virtual TextureManager*				GetTextureManager() = 0;
	virtual FontManager*				GetFontManager() = 0;
	virtual VertexDeclarationManager*	GetVertexDeclarationManager() = 0;
	virtual MeshFactory*				GetMeshFactory() = 0;

	virtual bool	IsValid() = 0;

	virtual void	EnumerateDisplayModes( Array<SDisplayMode>& DisplayModes ) = 0;

	virtual bool	SupportsSM3() = 0;
	virtual bool	SupportsSM2() = 0;

	virtual void*	GetDevice() = 0;

	virtual void	SetDisplay( Display* const pDisplay ) = 0;

	virtual void	SetClock( Clock* pClock ) = 0;
	virtual Clock*	GetClock() = 0;

#if BUILD_DEBUG
	struct SDEBUGRenderStats
	{
		SDEBUGRenderStats() : NumMeshes(0), NumPrimitives(0) {}
		uint NumMeshes;
		uint NumPrimitives;
	};

	virtual SDEBUGRenderStats&	DEBUGGetStats() = 0;
#endif
};

#endif // IRENDERER_H