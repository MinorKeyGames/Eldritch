#include "core.h"
#include "renderercommon.h"
#include "bucket.h"
#include "mesh.h"
#include "meshfactory.h"
#include "shadermanager.h"
#include "texturemanager.h"
#include "font.h"
#include "fontmanager.h"
#include "vertexdeclarationmanager.h"
#include "ivertexbuffer.h"
#include "viewportpass.h"
#include "configmanager.h"
#include "shaderdataprovider.h"

RendererCommon::RendererCommon()
:	m_OrderedBuckets()
,	m_NamedBuckets()
,	m_ViewportPasses()
#if BUILD_DEV
,	m_DeferredDeleteDebugMeshes()
#endif
,	m_WorldMatrix()
,	m_ViewMatrix()
,	m_ProjectionMatrix()
,	m_View()
,	m_CurrentRenderTarget( NULL )
,	m_DefaultRenderTarget( NULL )
,	m_RenderTargets()
,	m_DynamicVertexBuffers()
,	m_ShaderManager( NULL )
,	m_TextureManager( NULL )
,	m_FontManager( NULL )
,	m_VertexDeclarationManager( NULL )
,	m_MeshFactory( NULL )
,	m_DoFrustumCulling( false )
,	m_DoMaterialSort( false )
,	m_Clock( NULL )
,	m_ShaderProgram( NULL )
,	m_VertexShader( NULL )
,	m_PixelShader( NULL )
,	m_RenderState()
,	m_SamplerStates()
,	m_Display( NULL )
#if BUILD_DEBUG
,	m_DEBUGRenderStats()
#endif
{
#if BUILD_DEV
	m_DeferredDeleteDebugMeshes.SetDeflate( false );
#endif
}

RendererCommon::~RendererCommon()
{
	SafeDelete( m_DefaultRenderTarget );

	SafeDelete( m_ShaderManager );
	SafeDelete( m_TextureManager );
	SafeDelete( m_FontManager );
	SafeDelete( m_VertexDeclarationManager );
	SafeDelete( m_MeshFactory );

	FreeBuckets();
	FreeViewportPasses();
}

void RendererCommon::Initialize()
{
	XTRACE_FUNCTION;

	STATICHASH( Renderer );

	STATICHASH( FrustumCulling );
	m_DoFrustumCulling =
		ConfigManager::GetBool( sFrustumCulling, false, sRenderer ) ||
		ConfigManager::GetBool( sFrustumCulling );	// Support for old projects

	STATICHASH( DoMaterialSort );
	m_DoMaterialSort = ConfigManager::GetBool( sDoMaterialSort, false, sRenderer );
}

void RendererCommon::SetWorldMatrix( const Matrix& WorldMatrix )
{
	m_WorldMatrix = WorldMatrix;
}

void RendererCommon::SetViewMatrix( const Matrix& ViewMatrix )
{
	m_ViewMatrix = ViewMatrix;
}

void RendererCommon::SetProjectionMatrix( const Matrix& ProjectionMatrix )
{
	m_ProjectionMatrix = ProjectionMatrix;
}

Matrix RendererCommon::GetWorldMatrix()
{
	return m_WorldMatrix;
}

Matrix RendererCommon::GetViewMatrix()
{
	return m_ViewMatrix;
}

Matrix RendererCommon::GetProjectionMatrix()
{
	return m_ProjectionMatrix;
}

void RendererCommon::AddMesh( Mesh* pMesh )
{
	XTRACE_FUNCTION;

	DEVASSERT( pMesh );

	if( pMesh->m_VertexBuffer->GetNumVertices() == 0 )
	{
		return;
	}

#if BUILD_DEV
	uint NumBucketsAdded = 0;
#endif

	const uint MaterialFlags = pMesh->GetMaterialFlags();

	Bucket* pBucket;
	uint NumBuckets = m_OrderedBuckets.Size();
	for( uint i = 0; i < NumBuckets; ++i )
	{
		pBucket = m_OrderedBuckets[i];
		// This conditional means that the mesh must have all the
		// flags of the bucket and none of the filtered flags
		if( ( MaterialFlags & pBucket->m_Flags ) == pBucket->m_Flags &&
			( MaterialFlags & pBucket->m_FilterFlags ) == 0 )
		{
#if BUILD_DEV
			++NumBucketsAdded;
#endif
			pBucket->m_Meshes.PushBack( pMesh );
			if( pBucket->m_ExclusiveMeshes )
			{
				return;
			}
		}
	}

	DEVASSERTDESC( NumBucketsAdded > 0, "Mesh was added but fell into no buckets." );
}

void RendererCommon::AddBucket( const HashedString& Name, Bucket* pBucket )
{
	m_OrderedBuckets.PushBack( pBucket );
	m_NamedBuckets[ Name ] = pBucket;
}

Bucket*	RendererCommon::GetBucket( const HashedString& Name )
{
	Map< HashedString, Bucket* >::Iterator BucketIter = m_NamedBuckets.Search( Name );
	if( BucketIter.IsNull() )
	{
		return NULL;
	}
	else
	{
		return ( *BucketIter );
	}
}

Bucket*	RendererCommon::GetBucket( uint Index )
{
	if( Index < m_OrderedBuckets.Size() )
	{
		return m_OrderedBuckets[ Index ];
	}
	return NULL;
}

void RendererCommon::FreeBuckets()
{
	uint NumBuckets = m_OrderedBuckets.Size();
	for( uint i = 0; i < NumBuckets; ++i )
	{
		SafeDelete( m_OrderedBuckets[i] );
	}
	m_OrderedBuckets.Clear();
	m_NamedBuckets.Clear();
}

/*virtual*/ void RendererCommon::FlushBuckets()
{
	for( uint BucketIndex = 0; BucketIndex < m_OrderedBuckets.Size(); ++BucketIndex )
	{
		Bucket* const pBucket = m_OrderedBuckets[ BucketIndex ];
		if( pBucket )
		{
			pBucket->m_Meshes.Clear();
		}
	}
}

/*virtual*/ void RendererCommon::AddViewportPass( ViewportPass* const pViewportPass )
{
	m_ViewportPasses.PushBack( pViewportPass );
}

/*virtual*/ void RendererCommon::FreeViewportPasses()
{
	const uint NumViewportPasses = m_ViewportPasses.Size();
	for( uint ViewportPassIndex = 0; ViewportPassIndex < NumViewportPasses; ++ViewportPassIndex )
	{
		SafeDelete( m_ViewportPasses[ ViewportPassIndex ] );
	}
	m_ViewportPasses.Clear();
}

IVertexDeclaration* RendererCommon::GetVertexDeclaration( uint VertexSignature )
{
	return GetVertexDeclarationManager()->GetVertexDeclaration( VertexSignature );
}

void RendererCommon::FreeRenderTargets()
{
	m_RenderTargets.Clear();
}

IRenderTarget* RendererCommon::GetCurrentRenderTarget()
{
	return m_CurrentRenderTarget;
}

IRenderTarget* RendererCommon::GetDefaultRenderTarget()
{
	return m_DefaultRenderTarget;
}

void RendererCommon::AddDynamicVertexBuffer( IVertexBuffer* pBuffer )
{
	m_DynamicVertexBuffers.Insert( pBuffer );
}

void RendererCommon::RemoveDynamicVertexBuffer( IVertexBuffer* pBuffer )
{
	m_DynamicVertexBuffers.Remove( pBuffer );
}

void RendererCommon::ClearDynamicVertexBuffers()
{
	m_DynamicVertexBuffers.Clear();
}

/*virtual*/ void RendererCommon::SetDisplay( Display* const pDisplay )
{
	m_Display = pDisplay;
}

void RendererCommon::SetClock( Clock* pClock )
{
	m_Clock = pClock;
}

Clock* RendererCommon::GetClock()
{
	return m_Clock;
}

ShaderManager* RendererCommon::GetShaderManager()
{
	if( !m_ShaderManager )
	{
		m_ShaderManager = new ShaderManager( this );
	}
	return m_ShaderManager;
}

TextureManager* RendererCommon::GetTextureManager()
{
	if( !m_TextureManager )
	{
		m_TextureManager = new TextureManager( this );
	}
	return m_TextureManager;
}

FontManager* RendererCommon::GetFontManager()
{
	if( !m_FontManager )
	{
		m_FontManager = new FontManager( this );
	}
	return m_FontManager;
}

VertexDeclarationManager* RendererCommon::GetVertexDeclarationManager()
{
	if( !m_VertexDeclarationManager )
	{
		m_VertexDeclarationManager = new VertexDeclarationManager( this );
	}
	return m_VertexDeclarationManager;
}

MeshFactory* RendererCommon::GetMeshFactory()
{
	if( !m_MeshFactory )
	{
		m_MeshFactory = new MeshFactory( this );
	}
	return m_MeshFactory;
}

void RendererCommon::RenderBuckets()
{
	XTRACE_FUNCTION;

	if( m_ViewportPasses.Empty() )
	{
		// Just render buckets like normal
		for( uint i = 0; i < m_OrderedBuckets.Size(); ++i )
		{
			RenderBucket( m_OrderedBuckets[i], NULL );
		}
	}
	else
	{
		// Render each bucket for each viewport pass
		const uint NumViewportPasses = m_ViewportPasses.Size();
		for( uint ViewportPassIndex = 0; ViewportPassIndex < NumViewportPasses; ++ViewportPassIndex )
		{
			const ViewportPass* const pViewportPass = m_ViewportPasses[ ViewportPassIndex ];

			SetViewport( pViewportPass );

			for( uint i = 0; i < m_OrderedBuckets.Size(); ++i )
			{
				RenderBucket( m_OrderedBuckets[i], pViewportPass );
			}
		}
	}
}

void RendererCommon::PostRenderBuckets()
{
	XTRACE_FUNCTION;

	for( uint i = 0; i < m_OrderedBuckets.Size(); ++i )
	{
		Bucket* pBucket = m_OrderedBuckets[i];
		pBucket->m_Meshes.Clear();	// TODO: Later, only clear transient meshes, keep long-term ones around (as defined by a heuristic)
	}

#if BUILD_DEV
	PROFILE_BEGIN( RendererCommon_DeferredDeleteDebugMeshes );
	const uint NumDeferredDeleteDebugMeshes = m_DeferredDeleteDebugMeshes.Size();
	for( uint DebugMeshIndex = 0; DebugMeshIndex < NumDeferredDeleteDebugMeshes; ++DebugMeshIndex )
	{
		Mesh* pDebugMesh = m_DeferredDeleteDebugMeshes[ DebugMeshIndex ];
		ASSERT( pDebugMesh );
		SafeDelete( pDebugMesh );
	}
	m_DeferredDeleteDebugMeshes.Clear();
	PROFILE_END;
#endif
}

#if BUILD_DEV
void RendererCommon::DEBUGDrawLine( const Vector& Start, const Vector& End, unsigned int Color )
{
	Mesh* LineMesh = GetMeshFactory()->CreateDebugLine( Start, End, Color );
	LineMesh->m_IsDebugMesh = true;
	LineMesh->SetMaterialDefinition( DEFAULT_NEWMATERIAL, this );
	LineMesh->SetTexture( 0, GetTextureManager()->GetTexture( DEFAULT_TEXTURE, TextureManager::ETL_Permanent ) );
	LineMesh->SetMaterialFlags( MAT_DEBUG_WORLD );
	AddMesh( LineMesh );
}

void RendererCommon::DEBUGDrawTriangle( const Vector& V1, const Vector& V2, const Vector& V3, unsigned int Color )
{
	Mesh* TriMesh = GetMeshFactory()->CreateDebugTriangle( V1, V2, V3, Color );
	TriMesh->m_IsDebugMesh = true;
	TriMesh->SetMaterialDefinition( DEFAULT_NEWMATERIAL, this );
	TriMesh->SetTexture( 0, GetTextureManager()->GetTexture( DEFAULT_TEXTURE, TextureManager::ETL_Permanent ) );
	TriMesh->SetMaterialFlags( MAT_DEBUG_WORLD );
	AddMesh( TriMesh );
}

void RendererCommon::DEBUGDrawBox( const Vector& Min, const Vector& Max, unsigned int Color )
{
	Mesh* BoxMesh = GetMeshFactory()->CreateDebugBox( Min, Max, Color );
	BoxMesh->m_IsDebugMesh = true;
	BoxMesh->SetMaterialDefinition( DEFAULT_NEWMATERIAL, this );
	BoxMesh->SetTexture( 0, GetTextureManager()->GetTexture( DEFAULT_TEXTURE, TextureManager::ETL_Permanent ) );
	BoxMesh->SetMaterialFlags( MAT_DEBUG_WORLD );
	AddMesh( BoxMesh );
}

void RendererCommon::DEBUGDrawFrustum( const View& rView, unsigned int Color )
{
	Mesh* FrustumMesh = GetMeshFactory()->CreateDebugFrustum( rView, Color );
	FrustumMesh->m_IsDebugMesh = true;
	FrustumMesh->SetMaterialDefinition( DEFAULT_NEWMATERIAL, this );
	FrustumMesh->SetTexture( 0, GetTextureManager()->GetTexture( DEFAULT_TEXTURE, TextureManager::ETL_Permanent ) );
	FrustumMesh->SetMaterialFlags( MAT_DEBUG_WORLD );
	AddMesh( FrustumMesh );
}

void RendererCommon::DEBUGDrawSphere( const Vector& Center, float Radius, unsigned int Color )
{
	Mesh* SphereMesh = GetMeshFactory()->CreateDebugSphere( Center, Radius, Color );
	SphereMesh->m_IsDebugMesh = true;
	SphereMesh->SetMaterialDefinition( DEFAULT_NEWMATERIAL, this );
	SphereMesh->SetTexture( 0, GetTextureManager()->GetTexture( DEFAULT_TEXTURE, TextureManager::ETL_Permanent ) );
	SphereMesh->SetMaterialFlags( MAT_DEBUG_WORLD );
	AddMesh( SphereMesh );
}

void RendererCommon::DEBUGDrawEllipsoid( const Vector& Center, const Vector& Extents, unsigned int Color )
{
	Mesh* EllipsoidMesh = GetMeshFactory()->CreateDebugEllipsoid( Center, Extents, Color );
	EllipsoidMesh->m_IsDebugMesh = true;
	EllipsoidMesh->SetMaterialDefinition( DEFAULT_NEWMATERIAL, this );
	EllipsoidMesh->SetTexture( 0, GetTextureManager()->GetTexture( DEFAULT_TEXTURE, TextureManager::ETL_Permanent ) );
	EllipsoidMesh->SetMaterialFlags( MAT_DEBUG_WORLD );
	AddMesh( EllipsoidMesh );
}

void RendererCommon::DEBUGDrawCross( const Vector& Center, const float Length, unsigned int Color )
{
	Mesh* CrossMesh = GetMeshFactory()->CreateDebugCross( Center, Length, Color );
	CrossMesh->m_IsDebugMesh = true;
	CrossMesh->SetMaterialDefinition( DEFAULT_NEWMATERIAL, this );
	CrossMesh->SetTexture( 0, GetTextureManager()->GetTexture( DEFAULT_TEXTURE, TextureManager::ETL_Permanent ) );
	CrossMesh->SetMaterialFlags( MAT_DEBUG_WORLD );
	AddMesh( CrossMesh );
}

void RendererCommon::DEBUGDrawArrow( const Vector& Root, const Angles& Direction, const float Length, unsigned int Color )
{
	Mesh* ArrowMesh = GetMeshFactory()->CreateDebugArrow( Root, Direction, Length, Color );
	ArrowMesh->m_IsDebugMesh = true;
	ArrowMesh->SetMaterialDefinition( DEFAULT_NEWMATERIAL, this );
	ArrowMesh->SetTexture( 0, GetTextureManager()->GetTexture( DEFAULT_TEXTURE, TextureManager::ETL_Permanent ) );
	ArrowMesh->SetMaterialFlags( MAT_DEBUG_WORLD );
	AddMesh( ArrowMesh );
}

void RendererCommon::DEBUGDrawLine2D( const Vector& Start, const Vector& End, unsigned int Color )
{
	Mesh* LineMesh = GetMeshFactory()->CreateDebugLine( Start, End, Color );
	LineMesh->m_IsDebugMesh = true;
	LineMesh->SetMaterialDefinition( DEFAULT_NEWMATERIAL, this );
	LineMesh->SetTexture( 0, GetTextureManager()->GetTexture( DEFAULT_TEXTURE, TextureManager::ETL_Permanent ) );
	LineMesh->SetMaterialFlags( MAT_DEBUG_HUD );
	AddMesh( LineMesh );
}

void RendererCommon::DEBUGDrawBox2D( const Vector& Min, const Vector& Max, unsigned int Color )
{
	Mesh* BoxMesh = GetMeshFactory()->CreateDebugBox( Min, Max, Color );
	BoxMesh->m_IsDebugMesh = true;
	BoxMesh->SetMaterialDefinition( DEFAULT_NEWMATERIAL, this );
	BoxMesh->SetTexture( 0, GetTextureManager()->GetTexture( DEFAULT_TEXTURE, TextureManager::ETL_Permanent ) );
	BoxMesh->SetMaterialFlags( MAT_DEBUG_HUD );
	AddMesh( BoxMesh );
}

void RendererCommon::DEBUGPrint( const SimpleString& UTF8String, const Font* const pFont, const SRect& Bounds, const Vector4& Color )
{
	Mesh* const	pPrintMesh		= Print( UTF8String, pFont, Bounds, 0 );
	pPrintMesh->m_IsDebugMesh	= true;
	pPrintMesh->m_ConstantColor	= Color;
	pPrintMesh->SetMaterialFlags( MAT_DEBUG_HUD );
	AddMesh( pPrintMesh );
}
#endif // BUILD_DEV

Mesh* RendererCommon::Print( const SimpleString& UTF8String, const Font* const pFont, const SRect& Bounds, unsigned int Flags )
{
	DEVASSERT( pFont );
	Mesh* StringMesh = pFont->Print( UTF8String, Bounds, Flags );
	StringMesh->SetMaterialFlags( MAT_HUD );
	StringMesh->SetMaterialDefinition( "Material_HUD", this );
	return StringMesh;
}

#if BUILD_DEBUG
IRenderer::SDEBUGRenderStats& RendererCommon::DEBUGGetStats()
{
	return m_DEBUGRenderStats;
}
#endif

void RendererCommon::ApplyMaterial( const Material& Material, Mesh* const pMesh, const View& View )
{
	SetShaderProgram( Material.GetShaderProgram() );

	ApplyRenderState( Material.GetRenderState() );

	const uint NumSamplers = Material.GetNumSamplers();
	for( uint SamplerIndex = 0; SamplerIndex < NumSamplers; ++SamplerIndex )
	{
		ApplySamplerState( SamplerIndex, Material.GetSamplerState( SamplerIndex ) );
	}

	DEBUGASSERT( Material.GetSDP() );
	Material.GetSDP()->SetShaderParameters( this, pMesh, View );
}

void RendererCommon::ApplyRenderState( const SRenderState& RenderState )
{
	SetCullMode(			RenderState.m_CullMode );
	SetZEnable(				RenderState.m_ZEnable );
	SetZWriteEnable(		RenderState.m_ZWriteEnable );
	SetAlphaBlendEnable(	RenderState.m_AlphaBlendEnable );

	if( m_RenderState.m_AlphaBlendEnable == EABE_True )
	{
		SetBlend( RenderState.m_SrcBlend, RenderState.m_DestBlend );
	}
}

void RendererCommon::ApplySamplerState( const uint SamplerStage, const SSamplerState& SamplerState )
{
	SetTexture(			SamplerStage, SamplerState.m_Texture );
	SetAddressing(		SamplerStage, SamplerState.m_AddressU, SamplerState.m_AddressV );
	SetMinMipFilters(	SamplerStage, SamplerState.m_MinFilter, SamplerState.m_MipFilter );
	SetMagFilter(		SamplerStage, SamplerState.m_MagFilter );
}

void RendererCommon::ResetRenderState()
{
	// Reset our shadowed state so we'll update the D3D state properly after reset.
	m_RenderState = SRenderState();
	for( uint SamplerStage = 0; SamplerStage < MAX_TEXTURE_STAGES; ++SamplerStage )
	{
		m_SamplerStates[ SamplerStage ] = SSamplerState();
	}
}