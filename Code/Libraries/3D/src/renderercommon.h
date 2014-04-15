#ifndef RENDERERCOMMON_H
#define RENDERERCOMMON_H

#include "irenderer.h"
#include "matrix.h"
#include "array.h"
#include "view.h"
#include "map.h"
#include "list.h"
#include "set.h"

class Material;

class RendererCommon : public IRenderer
{
public:
	RendererCommon();
	~RendererCommon();

	virtual void	Initialize();

	virtual ShaderManager*				GetShaderManager();
	virtual TextureManager*				GetTextureManager();
	virtual FontManager*				GetFontManager();
	virtual VertexDeclarationManager*	GetVertexDeclarationManager();
	virtual MeshFactory*				GetMeshFactory();

	virtual void	SetDisplay( Display* const pDisplay );

	virtual void	SetClock( Clock* pClock );
	virtual Clock*	GetClock();

	virtual void	SetWorldMatrix( const Matrix& WorldMatrix );
	virtual void	SetViewMatrix( const Matrix& ViewMatrix );
	virtual void	SetProjectionMatrix( const Matrix& ProjectionMatrix );
	virtual Matrix	GetWorldMatrix();
	virtual Matrix	GetViewMatrix();
	virtual Matrix	GetProjectionMatrix();

	virtual void	AddMesh( Mesh* pMesh );
	virtual void	AddBucket( const HashedString& Name, Bucket* pBucket );
	virtual Bucket*	GetBucket( const HashedString& Name );
	virtual Bucket*	GetBucket( uint Index );
	virtual void	FreeBuckets();
	virtual void	FlushBuckets();

	// Viewport pass defines a viewport rectangle and a map of view overrides.
	virtual void	AddViewportPass( ViewportPass* const pViewportPass );
	virtual void	FreeViewportPasses();

	virtual void			FreeRenderTargets();
	virtual IRenderTarget*	GetCurrentRenderTarget();
	virtual IRenderTarget*	GetDefaultRenderTarget();

	virtual void	AddDynamicVertexBuffer( IVertexBuffer* pBuffer );
	virtual void	RemoveDynamicVertexBuffer( IVertexBuffer* pBuffer );
	virtual void	ClearDynamicVertexBuffers();

	virtual IVertexDeclaration*	GetVertexDeclaration( uint VertexSignature );

#if BUILD_DEV
	virtual void	DEBUGDrawLine( const Vector& Start, const Vector& End, unsigned int Color );
	virtual void	DEBUGDrawTriangle( const Vector& V1, const Vector& V2, const Vector& V3, unsigned int Color );
	virtual void	DEBUGDrawBox( const Vector& Min, const Vector& Max, unsigned int Color );
	virtual void	DEBUGDrawFrustum( const View& rView, unsigned int Color );
	virtual void	DEBUGDrawSphere( const Vector& Center, float Radius, unsigned int Color );
	virtual void	DEBUGDrawEllipsoid( const Vector& Center, const Vector& Extents, unsigned int Color );
	virtual void	DEBUGDrawCross( const Vector& Center, const float Length, unsigned int Color );
	virtual void	DEBUGDrawArrow( const Vector& Root, const Angles& Direction, const float Length, unsigned int Color );

	virtual void	DEBUGDrawLine2D( const Vector& Start, const Vector& End, unsigned int Color );
	virtual void	DEBUGDrawBox2D( const Vector& Min, const Vector& Max, unsigned int Color );

	virtual void	DEBUGPrint( const SimpleString& UTF8String, const Font* const pFont, const SRect& Bounds, const Vector4& Color );
#endif // BUILD_DEV

	virtual Mesh*	Print( const SimpleString& UTF8String, const Font* const pFont, const SRect& Bounds, unsigned int Flags );

#if BUILD_DEBUG
	virtual SDEBUGRenderStats&	DEBUGGetStats();
#endif

protected:
	virtual void	RenderBucket( Bucket* pBucket, const ViewportPass* const pViewportPass ) = 0;
	void			RenderBuckets();
	void			PostRenderBuckets();

	virtual void	SetViewport( const ViewportPass* const pViewportPass ) = 0;

	void			ApplyMaterial( const Material& Material, Mesh* const pMesh, const View& View );
	void			ApplyRenderState( const SRenderState& RenderState );
	void			ApplySamplerState( const uint SamplerStage, const SSamplerState& SamplerState );

	void			ResetRenderState();

	Array< Bucket* >				m_OrderedBuckets;
	Map< HashedString, Bucket* >	m_NamedBuckets;

	Array<ViewportPass*>			m_ViewportPasses;

#if BUILD_DEV
	Array<Mesh*>					m_DeferredDeleteDebugMeshes;
#endif

	Matrix				m_WorldMatrix;
	Matrix				m_ViewMatrix;
	Matrix				m_ProjectionMatrix;

	View				m_View;

	IRenderTarget*			m_CurrentRenderTarget;
	IRenderTarget*			m_DefaultRenderTarget;
	List< IRenderTarget* >	m_RenderTargets;	// TODO: Replace this with functionality in TargetManager
	Set< IVertexBuffer* >	m_DynamicVertexBuffers;	// Things wot get released and rebuilt when device is reset

	ShaderManager*				m_ShaderManager;
	TextureManager*				m_TextureManager;
	FontManager*				m_FontManager;
	VertexDeclarationManager*	m_VertexDeclarationManager;
	MeshFactory*				m_MeshFactory;

	bool					m_DoFrustumCulling;
	bool					m_DoMaterialSort;

	//HWND				m_hWnd;
	//bool				m_Fullscreen;
	//EMultiSampleType	m_MultiSampleType;

	Clock*				m_Clock;

	// Shadow device state to avoid querying hardware
	IShaderProgram*		m_ShaderProgram;
	IVertexShader*		m_VertexShader;
	IPixelShader*		m_PixelShader;
	SRenderState		m_RenderState;
	SSamplerState		m_SamplerStates[ MAX_TEXTURE_STAGES ];

	Display*			m_Display;

#if BUILD_DEBUG
	SDEBUGRenderStats	m_DEBUGRenderStats;
#endif
};

#endif // RENDERERCOMMON_H