#include "core.h"
#include "linebatcher.h"
#include "array.h"
#include "irenderer.h"
#include "mesh.h"
#include "ivertexdeclaration.h"
#include "configmanager.h"
#include "mathcore.h"
#include "frustum.h"
#include "view.h"
#include "ivertexbuffer.h"
#include "iindexbuffer.h"
#include "shadermanager.h"
#include "texturemanager.h"

LineBatcher::LineBatcher( IRenderer* pRenderer, uint MaterialFlags, bool IsDebug )
:	m_Positions()
,	m_Colors()
,	m_Indices()
,	m_AddPositions()
,	m_AddColors()
,	m_AddIndices()
#if BUILD_DEV
,	m_MaterialFlags( MaterialFlags | ( IsDebug ? MAT_DEBUG_ALWAYS : MAT_ALWAYS ) )
#else
,	m_MaterialFlags( MaterialFlags | MAT_ALWAYS )
#endif
,	m_Renderer( pRenderer )
,	m_IsDebug( IsDebug )
{
	// Don't reallocate every frame
	m_Positions.SetDeflate( false );
	m_Colors.SetDeflate( false );
	m_Indices.SetDeflate( false );

	// Don't reallocate temp buffers
	m_AddPositions.SetDeflate( false );
	m_AddColors.SetDeflate( false );
	m_AddIndices.SetDeflate( false );
}

LineBatcher::~LineBatcher()
{
	m_Positions.Clear();
	m_Colors.Clear();
	m_Indices.Clear();

	m_AddPositions.Clear();
	m_AddColors.Clear();
	m_AddIndices.Clear();
}

void LineBatcher::Add( const Array< Vector >& m_AddPositions, const Array< uint >& m_AddColors, const Array< index_t >& m_AddIndices )
{
#if BUILD_RELEASE
	if( m_IsDebug )
	{
		return;
	}
#endif

	ASSERT( m_AddPositions.Size() == m_AddColors.Size() );

	uint OldSize = m_Positions.Size();
	uint NewSize = OldSize + m_AddPositions.Size();

	m_Positions.Resize( NewSize );
	m_Colors.Resize( NewSize );
	for( uint i = OldSize, j = 0; i < NewSize; ++i, ++j )
	{
		m_Positions[i] = m_AddPositions[j];
		m_Colors[i] = m_AddColors[j];
	}

	uint OldIndicesSize = m_Indices.Size();
	uint NewIndicesSize = OldIndicesSize + m_AddIndices.Size();
	m_Indices.Resize( NewIndicesSize );
	for( uint i = OldIndicesSize, j = 0; i < NewIndicesSize; ++i, ++j )
	{
		m_Indices[i] = m_AddIndices[j] + ( index_t )OldSize;
	}
}

Mesh* LineBatcher::Render()
{
#if BUILD_RELEASE
	if( m_IsDebug )
	{
		return NULL;
	}
#endif

	if( 0 == m_Positions.Size() )
	{
		return NULL;
	}

	IVertexBuffer* VertexBuffer				= m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VertexDeclaration	= m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS );
	IIndexBuffer* IndexBuffer				= m_Renderer->CreateIndexBuffer();

	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = m_Positions.Size();
	InitStruct.Positions = m_Positions.GetData();
	InitStruct.Colors = m_Colors.GetData();
	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( m_Indices.Size(), m_Indices.GetData() );
	IndexBuffer->SetPrimitiveType( EPT_LINELIST );
	Mesh* LinesMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );

#if BUILD_DEBUG
	LinesMesh->m_Name = "LineBatch";
	LinesMesh->m_IsDebugMesh = m_IsDebug;
#endif

	LinesMesh->SetMaterialDefinition( DEFAULT_NEWMATERIAL, m_Renderer );
	LinesMesh->SetTexture( 0, m_Renderer->GetTextureManager()->GetTexture( DEFAULT_TEXTURE, TextureManager::ETL_Permanent ) );
	LinesMesh->SetMaterialFlags( m_MaterialFlags );

	m_Renderer->AddMesh( LinesMesh );

	// Reset everything to its starting state
	m_Positions.Clear();
	m_Colors.Clear();
	m_Indices.Clear();

	return LinesMesh;
}

void LineBatcher::DrawLine( const Vector& Start, const Vector& End, unsigned int Color )
{
#if BUILD_RELEASE
	if( m_IsDebug )
	{
		return;
	}
#endif

	m_AddPositions.Resize( 2 );
	m_AddColors.Resize( 2 );
	m_AddIndices.Resize( 2 );

	m_AddPositions[0] = Start;
	m_AddPositions[1] = End;
	m_AddColors[0] = Color;
	m_AddColors[1] = Color;
	m_AddIndices[0] = 0;
	m_AddIndices[1] = 1;

	Add( m_AddPositions, m_AddColors, m_AddIndices );
}

void LineBatcher::DrawTriangle( const Vector& A, const Vector& B, const Vector& C, unsigned int Color )
{
#if BUILD_RELEASE
	if( m_IsDebug )
	{
		return;
	}
#endif

	m_AddPositions.Resize( 3 );
	m_AddColors.Resize( 3 );
	m_AddIndices.Resize( 6 );

	m_AddPositions[0] = A;
	m_AddPositions[1] = B;
	m_AddPositions[2] = C;
	m_AddColors[0] = Color;
	m_AddColors[1] = Color;
	m_AddColors[2] = Color;
	m_AddIndices[0] = 0;
	m_AddIndices[1] = 1;
	m_AddIndices[2] = 0;
	m_AddIndices[3] = 2;
	m_AddIndices[4] = 1;
	m_AddIndices[5] = 2;

	Add( m_AddPositions, m_AddColors, m_AddIndices );
}

void LineBatcher::DrawBox( const Vector& Min, const Vector& Max, unsigned int Color )
{
#if BUILD_RELEASE
	if( m_IsDebug )
	{
		return;
	}
#endif

	m_AddPositions.Resize( 8 );
	m_AddColors.Resize( 8 );
	m_AddIndices.Resize( 24 );

	m_AddPositions[0] = Vector( Min.x, Min.y, Min.z );
	m_AddPositions[1] = Vector( Min.x, Min.y, Max.z );
	m_AddPositions[2] = Vector( Min.x, Max.y, Min.z );
	m_AddPositions[3] = Vector( Min.x, Max.y, Max.z );
	m_AddPositions[4] = Vector( Max.x, Min.y, Min.z );
	m_AddPositions[5] = Vector( Max.x, Min.y, Max.z );
	m_AddPositions[6] = Vector( Max.x, Max.y, Min.z );
	m_AddPositions[7] = Vector( Max.x, Max.y, Max.z );
	m_AddColors[0] = Color;
	m_AddColors[1] = Color;
	m_AddColors[2] = Color;
	m_AddColors[3] = Color;
	m_AddColors[4] = Color;
	m_AddColors[5] = Color;
	m_AddColors[6] = Color;
	m_AddColors[7] = Color;
	m_AddIndices[0] = 0;
	m_AddIndices[1] = 1;
	m_AddIndices[2] = 0;
	m_AddIndices[3] = 2;
	m_AddIndices[4] = 0;
	m_AddIndices[5] = 4;
	m_AddIndices[6] = 1;
	m_AddIndices[7] = 3;
	m_AddIndices[8] = 1;
	m_AddIndices[9] = 5;
	m_AddIndices[10] = 2;
	m_AddIndices[11] = 3;
	m_AddIndices[12] = 2;
	m_AddIndices[13] = 6;
	m_AddIndices[14] = 3;
	m_AddIndices[15] = 7;
	m_AddIndices[16] = 4;
	m_AddIndices[17] = 5;
	m_AddIndices[18] = 4;
	m_AddIndices[19] = 6;
	m_AddIndices[20] = 5;
	m_AddIndices[21] = 7;
	m_AddIndices[22] = 6;
	m_AddIndices[23] = 7;

	Add( m_AddPositions, m_AddColors, m_AddIndices );
}

void LineBatcher::DrawX( const Vector& Center, const float Radius, const unsigned int Color )
{
#if BUILD_RELEASE
	if( m_IsDebug )
	{
		return;
	}
#endif

	m_AddPositions.Resize( 8 );
	m_AddColors.Resize( 8 );
	m_AddIndices.Resize( 8 );

	m_AddPositions[0] = Vector( Center.x + Radius, Center.y + Radius, Center.z + Radius );
	m_AddPositions[1] = Vector( Center.x + Radius, Center.y + Radius, Center.z - Radius );
	m_AddPositions[2] = Vector( Center.x + Radius, Center.y - Radius, Center.z + Radius );
	m_AddPositions[3] = Vector( Center.x + Radius, Center.y - Radius, Center.z - Radius );
	m_AddPositions[4] = Vector( Center.x - Radius, Center.y + Radius, Center.z + Radius );
	m_AddPositions[5] = Vector( Center.x - Radius, Center.y + Radius, Center.z - Radius );
	m_AddPositions[6] = Vector( Center.x - Radius, Center.y - Radius, Center.z + Radius );
	m_AddPositions[7] = Vector( Center.x - Radius, Center.y - Radius, Center.z - Radius );
	m_AddColors[0] = Color;
	m_AddColors[1] = Color;
	m_AddColors[2] = Color;
	m_AddColors[3] = Color;
	m_AddColors[4] = Color;
	m_AddColors[5] = Color;
	m_AddColors[6] = Color;
	m_AddColors[7] = Color;
	m_AddIndices[0] = 0;
	m_AddIndices[1] = 7;
	m_AddIndices[2] = 1;
	m_AddIndices[3] = 6;
	m_AddIndices[4] = 2;
	m_AddIndices[5] = 5;
	m_AddIndices[6] = 3;
	m_AddIndices[7] = 4;

	Add( m_AddPositions, m_AddColors, m_AddIndices );
}

void LineBatcher::DrawFrustum( const View& View, unsigned int Color )
{
#if BUILD_RELEASE
	if( m_IsDebug )
	{
		return;
	}
#endif

	m_AddPositions.Resize( 8 );
	m_AddColors.Resize( 8 );
	m_AddIndices.Resize( 24 );

	Frustum Frustum;
	View.ApplyToFrustum( Frustum );
	Frustum.GetCorners( m_AddPositions.GetData() );

	m_AddColors[0] = Color;
	m_AddColors[1] = Color;
	m_AddColors[2] = Color;
	m_AddColors[3] = Color;
	m_AddColors[4] = Color;
	m_AddColors[5] = Color;
	m_AddColors[6] = Color;
	m_AddColors[7] = Color;
	m_AddIndices[0] = 0;
	m_AddIndices[1] = 1;
	m_AddIndices[2] = 0;
	m_AddIndices[3] = 2;
	m_AddIndices[4] = 0;
	m_AddIndices[5] = 4;
	m_AddIndices[6] = 1;
	m_AddIndices[7] = 3;
	m_AddIndices[8] = 1;
	m_AddIndices[9] = 5;
	m_AddIndices[10] = 2;
	m_AddIndices[11] = 3;
	m_AddIndices[12] = 2;
	m_AddIndices[13] = 6;
	m_AddIndices[14] = 3;
	m_AddIndices[15] = 7;
	m_AddIndices[16] = 4;
	m_AddIndices[17] = 5;
	m_AddIndices[18] = 4;
	m_AddIndices[19] = 6;
	m_AddIndices[20] = 5;
	m_AddIndices[21] = 7;
	m_AddIndices[22] = 6;
	m_AddIndices[23] = 7;

	Add( m_AddPositions, m_AddColors, m_AddIndices );
}

void LineBatcher::DrawSphere( const Vector& Center, float Radius, unsigned int Color )
{
#if BUILD_RELEASE
	if( m_IsDebug )
	{
		return;
	}
#endif

	STATICHASH( DebugSphereResolution );
	uint Resolution = ConfigManager::GetInt( sDebugSphereResolution, 16 );
	uint TwoR = Resolution * 2;

	m_AddPositions.Resize( Resolution * 3 );
	m_AddColors.Resize( Resolution * 3 );
	m_AddIndices.Resize( Resolution * 6 );

	for( uint i = 0; i < Resolution * 3; ++i )
	{
		m_AddColors[ i ] = Color;
	}

	float Mult = TWOPI / (float)Resolution;

	for( uint i = 0; i < Resolution; ++i )
	{
		float u = Radius * Cos( (float)i * Mult );
		float v = Radius * Sin( (float)i * Mult );
		m_AddPositions[ i ] = Vector( Center.x + u, Center.y + v, Center.z );
		m_AddPositions[ i + Resolution ] = Vector( Center.x + u, Center.y, Center.z + v );
		m_AddPositions[ i + TwoR ] = Vector( Center.x, Center.y + u, Center.z + v );
		m_AddIndices[ i * 2 ] = ( index_t )( i );
		m_AddIndices[ i * 2 + 1 ] = ( index_t )( ( i + 1 ) % Resolution );
		m_AddIndices[ i * 2 + TwoR ] = ( index_t )( i + Resolution );
		m_AddIndices[ i * 2 + TwoR + 1 ] = ( index_t )( ( ( i + 1 ) % Resolution ) + Resolution );
		m_AddIndices[ i * 2 + TwoR * 2 ] = ( index_t )( i + TwoR );
		m_AddIndices[ i * 2 + TwoR * 2 + 1 ] = ( index_t )( ( ( i + 1 ) % Resolution ) + TwoR );
	}

	Add( m_AddPositions, m_AddColors, m_AddIndices );
}

void LineBatcher::DrawEllipsoid( const Vector& Center, const Vector& Extents, unsigned int Color )
{
#if BUILD_RELEASE
	if( m_IsDebug )
	{
		return;
	}
#endif

	STATICHASH( DebugSphereResolution );
	uint Resolution = ConfigManager::GetInt( sDebugSphereResolution, 16 );
	uint TwoR = Resolution * 2;

	m_AddPositions.Resize( Resolution * 3 );
	m_AddColors.Resize( Resolution * 3 );
	m_AddIndices.Resize( Resolution * 6 );

	for( uint i = 0; i < Resolution * 3; ++i )
	{
		m_AddColors[ i ] = Color;
	}

	float Mult = TWOPI / (float)Resolution;

	for( uint i = 0; i < Resolution; ++i )
	{
		float u = Cos( (float)i * Mult );
		float v = Sin( (float)i * Mult );
		m_AddPositions[ i ] = Vector( Center.x + u * Extents.x, Center.y + v * Extents.y, Center.z );
		m_AddPositions[ i + Resolution ] = Vector( Center.x + u * Extents.x, Center.y, Center.z + v * Extents.z );
		m_AddPositions[ i + TwoR ] = Vector( Center.x, Center.y + u * Extents.y, Center.z + v * Extents.z );
		m_AddIndices[ i * 2 ] = ( index_t )( i );
		m_AddIndices[ i * 2 + 1 ] = ( index_t )( ( i + 1 ) % Resolution );
		m_AddIndices[ i * 2 + TwoR ] = ( index_t )( i + Resolution );
		m_AddIndices[ i * 2 + TwoR + 1 ] = ( index_t )( ( ( i + 1 ) % Resolution ) + Resolution );
		m_AddIndices[ i * 2 + TwoR * 2 ] = ( index_t )( i + TwoR );
		m_AddIndices[ i * 2 + TwoR * 2 + 1 ] = ( index_t )( ( ( i + 1 ) % Resolution ) + TwoR );
	}

	Add( m_AddPositions, m_AddColors, m_AddIndices );
}

void LineBatcher::DrawGradientLine( const Vector& Start, const Vector& End, unsigned int ColorA, unsigned int ColorB )
{
#if BUILD_RELEASE
	if( m_IsDebug )
	{
		return;
	}
#endif

	m_AddPositions.Resize( 2 );
	m_AddColors.Resize( 2 );
	m_AddIndices.Resize( 2 );

	m_AddPositions[0] = Start;
	m_AddPositions[1] = End;
	m_AddColors[0] = ColorA;
	m_AddColors[1] = ColorB;
	m_AddIndices[0] = 0;
	m_AddIndices[1] = 1;

	Add( m_AddPositions, m_AddColors, m_AddIndices );
}

void LineBatcher::DrawIrradianceCross( const Vector& Center, const float Radius, const Vector4 m_AddColors[6] )
{
#if BUILD_RELEASE
	if( m_IsDebug )
	{
		return;
	}
#endif

	Vector X( Radius, 0.0f, 0.0f );
	Vector Y( 0.0f, Radius, 0.0f );
	Vector Z( 0.0f, 0.0f, Radius );

	DrawGradientLine( Center + X, Center - X, m_AddColors[0].ToColor(), m_AddColors[1].ToColor() );
	DrawGradientLine( Center + Y, Center - Y, m_AddColors[2].ToColor(), m_AddColors[3].ToColor() );
	DrawGradientLine( Center + Z, Center - Z, m_AddColors[4].ToColor(), m_AddColors[5].ToColor() );
}