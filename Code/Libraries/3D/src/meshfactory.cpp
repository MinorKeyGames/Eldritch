#include "core.h"
#include "3d.h"
#include "meshfactory.h"
#include "mesh.h"
#include "ivertexbuffer.h"
#include "ivertexdeclaration.h"
#include "iindexbuffer.h"
#include "vector.h"
#include "vector4.h"
#include "vector2.h"
#include "bonearray.h"
#include "matrix.h"
#include "view.h"
#include "frustum.h"
#include "mathcore.h"
#include "idatastream.h"
#include "collisiontriangle.h"
#include "configmanager.h"
#include "dynamicmeshmanager.h"
#include "packstream.h"
#include "simplestring.h"
#include "animation.h"
#include "hashedstring.h"
#include "irenderer.h"
#include "texturemanager.h"

#include <stdio.h>

SCompiledMeshHeader::SCompiledMeshHeader()
:	m_MagicID( 'SMCD' )
,	m_NumVertices(0)
,	m_NumIndices(0)
,	m_NumFrames(0)
,	m_NumCollisionTris(0)
,	m_NumBones(0)
,	m_NumAnims(0)
,	m_NumMaterials(0)
,	m_LongIndices( false )
,	m_HasUVs( false )
,	m_HasColors( false )
,	m_HasNormals( false )
,	m_HasTangents( false )
,	m_HasSkeleton( false )
{
}

MeshFactory::MeshFactory( IRenderer* Renderer ) : m_Renderer( Renderer ) {}

Mesh* MeshFactory::CreateQuad( float Length, EPlane Plane, bool TwoSided )
{
	return CreatePlane( Length, Length, 1, 1, Plane, TwoSided );
}

Mesh* MeshFactory::CreatePlane( float Length, float Width, int LengthSegments, int WidthSegments, EPlane Plane, bool TwoSided )
{
	int LenP = LengthSegments + 1;
	int WidP = WidthSegments + 1;
	int NumVertices = LenP * WidP;
	int IndicesMultiplier = ( TwoSided ) ? 12 : 6;
	int NumIndices = IndicesMultiplier * LengthSegments * WidthSegments;

	Vector* Positions = new Vector[ NumVertices ];
	uint* Colors = new uint[ NumVertices ];
	Vector2* UVs = new Vector2[ NumVertices ];
	Vector* Normals = new Vector[ NumVertices ];
	Vector4* Tangents = new Vector4[ NumVertices ];
	index_t* Indices = new index_t[ NumIndices ];

	float RecL = Length / (float)LengthSegments;
	float RecW = Width / (float)WidthSegments;
	float HalfL = Length * .5f;
	float HalfW = Width * .5f;
	int Index;

	for( int j = 0; j < WidP; ++j )
	{
		for( int i = 0; i < LenP; ++i )
		{
			Index = j * LenP + i;
			if( Plane == XY_PLANE )
			{
				Positions[ Index ] = Vector( i * RecL - HalfL, HalfW - j * RecW, 0.f );
				Colors[ Index ] = 0xffffffff;
				UVs[ Index ] = Vector2( (float)i / LengthSegments, (float)j / WidthSegments );
				Normals[ Index ] = Vector( 0.f, 0.f, 1.f );
				Tangents[ Index ] = Vector4( 1.f, 0.f, 0.f, 1.f );	// Bitangent = (Normal x Tangent) * Bitangent orientation
			}
			else if( Plane == XZ_PLANE )
			{
				// Because XZ plane is typically used for ortho mode (UI) and my ortho
				// projection flips the vertical component, positions and UVs are backwards
				// here compared to XY and YZ planes. Ugly, but whatever. This is legacy code.
				Positions[ Index ] = Vector( i * RecL - HalfL, 0.f, j * RecW - HalfW );
				Colors[ Index ] = 0xffffffff;
				UVs[ Index ] = Vector2( (float)i / LengthSegments, (float)j / WidthSegments );
				Normals[ Index ] = Vector( 0.f, -1.f, 0.f );
				Tangents[ Index ] = Vector4( 1.f, 0.f, 0.f, 1.f );
			}
			else if( Plane == YZ_PLANE )
			{
				Positions[ Index ] = Vector( 0.f, i * RecL - HalfL, HalfW - j * RecW );
				Colors[ Index ] = 0xffffffff;
				UVs[ Index ] = Vector2( (float)i / LengthSegments, (float)j / WidthSegments );
				Normals[ Index ] = Vector( 1.f, 0.f, 0.f );
				Tangents[ Index ] = Vector4( 0.f, 1.f, 0.f, 1.f );
			}
		}
	}

	int VertexIndex = 0;
	int IndexIndex = 0;
	for( int j = 0; j < WidthSegments; ++j )
	{
		for( int i = 0; i < LengthSegments; ++i )
		{
			Indices[ IndexIndex++ ] = (index_t)( VertexIndex );
			Indices[ IndexIndex++ ] = (index_t)( VertexIndex + LenP );
			Indices[ IndexIndex++ ] = (index_t)( VertexIndex + LenP + 1 );
			Indices[ IndexIndex++ ] = (index_t)( VertexIndex );
			Indices[ IndexIndex++ ] = (index_t)( VertexIndex + LenP + 1 );
			Indices[ IndexIndex++ ] = (index_t)( VertexIndex + 1 );

			if( TwoSided )
			{
				Indices[ IndexIndex++ ] = (index_t)( VertexIndex );
				Indices[ IndexIndex++ ] = (index_t)( VertexIndex + LenP + 1 );
				Indices[ IndexIndex++ ] = (index_t)( VertexIndex + LenP );
				Indices[ IndexIndex++ ] = (index_t)( VertexIndex );
				Indices[ IndexIndex++ ] = (index_t)( VertexIndex + 1 );
				Indices[ IndexIndex++ ] = (index_t)( VertexIndex + LenP + 1 );
			}
			++VertexIndex;
		}
		++VertexIndex;
	}

	IVertexBuffer*		NewVB = m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VDecl = m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS | VD_UVS | VD_NORMALS | VD_TANGENTS );
	IIndexBuffer*		NewIB = m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = NumVertices;
	InitStruct.Positions = Positions;
	InitStruct.Colors = Colors;
	InitStruct.UVs = UVs;
	InitStruct.Normals = Normals;
	InitStruct.Tangents = Tangents;
	NewVB->Init( InitStruct );
	NewIB->Init( NumIndices, Indices );
	NewIB->SetPrimitiveType( EPT_TRIANGLELIST );
	Mesh* NewMesh = new Mesh( NewVB, VDecl, NewIB );

	if( Plane == XY_PLANE )
	{
		NewMesh->m_AABB = AABB( Vector( -HalfL, -HalfW, 0.0f ), Vector( HalfL, HalfW, 0.0f ) );
	}
	else if( Plane == XZ_PLANE )
	{
		NewMesh->m_AABB = AABB( Vector( -HalfL, 0.0f, -HalfW ), Vector( HalfL, 0.0f, HalfW ) );
	}
	else if( Plane == YZ_PLANE )
	{
		NewMesh->m_AABB = AABB( Vector( 0.0f, -HalfL, -HalfW ), Vector( 0.0f, HalfL, HalfW ) );
	}

#if BUILD_DEBUG
	NewMesh->m_Name = "Plane";
#endif

	delete Positions;
	delete Colors;
	delete UVs;
	delete Normals;
	delete Tangents;
	delete Indices;

	return NewMesh;
}

// Creates sprite in the XZ plane for use as a UI element. Doesn't
// have normals or tangents like a particle sprite would need.
Mesh* MeshFactory::CreateSprite()
{
	Vector Positions[4];
	Vector2 UVs[4];
	index_t Indices[6];

	Positions[0] = Vector( -0.5f, 0.0f, -0.5f );
	Positions[1] = Vector( 0.5f, 0.0f, -0.5f );
	Positions[2] = Vector( -0.5f, 0.0f, 0.5f );
	Positions[3] = Vector( 0.5f, 0.0f, 0.5f );
	UVs[0] = Vector2( 0.0f, 0.0f );
	UVs[1] = Vector2( 1.0f, 0.0f );
	UVs[2] = Vector2( 0.0f, 1.0f );
	UVs[3] = Vector2( 1.0f, 1.0f );
	Indices[0] = 0;
	Indices[1] = 2;
	Indices[2] = 1;
	Indices[3] = 2;
	Indices[4] = 3;
	Indices[5] = 1;

	IVertexBuffer*		VertexBuffer		= m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VertexDeclaration	= m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_UVS );
	IIndexBuffer*		IndexBuffer			= m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = 4;
	InitStruct.Positions = Positions;
	InitStruct.UVs = UVs;
	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( 6, Indices );
	IndexBuffer->SetPrimitiveType( EPT_TRIANGLELIST );
	Mesh* pMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );

	pMesh->m_AABB = AABB( Vector( -0.5f, 0.0f, -0.5f ), Vector( 0.5f, 0.0f, 0.5f ) );

#if BUILD_DEBUG
	pMesh->m_Name = "Sprite";
#endif

	return pMesh;
}

Mesh* MeshFactory::CreateGrid( float Length, float Width, int LengthSegments, int WidthSegments, EPlane Plane )
{
	int LenP = LengthSegments + 1;
	int WidP = WidthSegments + 1;
	int NumVertices = 2 * ( LenP + WidP - 2 );
	int NumIndices = 2 * ( LenP + WidP );

	Vector* Positions = new Vector[ NumVertices ];
	uint*	Colors = new uint[ NumVertices ];
	index_t* Indices = new index_t[ NumIndices ];

	float RecL = Length / (float)LengthSegments;
	float RecW = Width / (float)WidthSegments;
	float HalfL = Length * .5f;
	float HalfW = Width * .5f;
	int Index;

	// Indexing scheme (e.g., 3 segments):
	// 0 8 10 1
	// 2      3
	// 4      5
	// 6 9 11 7
	// This avoids redundant vertices, but also makes it a bit
	// harder to generate--see the second loop below.

	for( int j = 0; j < WidP; ++j )
	{
		Index = j << 1;
		if( Plane == XY_PLANE )
		{
			Positions[ Index ] = Vector( -HalfL, j * RecW - HalfW, 0.f );
			Positions[ Index + 1 ] = Vector( HalfL, j * RecW - HalfW, 0.f );
		}
		else if( Plane == XZ_PLANE )
		{
			Positions[ Index ] = Vector( -HalfL, 0.f, j * RecW - HalfW );
			Positions[ Index + 1 ] = Vector( HalfL, 0.f, j * RecW - HalfW );
		}
		else if( Plane == YZ_PLANE )
		{
			Positions[ Index ] = Vector( 0.f, -HalfL, j * RecW - HalfW );
			Positions[ Index + 1 ] = Vector( 0.f, HalfL, j * RecW - HalfW );
		}
		Colors[ Index ] = 0xffffffff;
		Colors[ Index + 1 ] = 0xffffffff;
		Indices[ Index ] = (index_t)Index;
		Indices[ Index + 1 ] = (index_t)Index + 1;
	}

	int VertexIndex, IndexIndex;
	for( int i = 0; i < LenP; ++i )
	{
		VertexIndex = ( WidP << 1 ) + ( ( i - 1 ) << 1 );
		IndexIndex = ( WidP << 1 ) + ( i << 1 );
		if( i == 0 )
		{
			Indices[ IndexIndex ] = 0;
			Indices[ IndexIndex + 1 ] = (index_t)( WidthSegments << 1 );
		}
		else if( i == LengthSegments )
		{
			Indices[ IndexIndex ] = 1;
			Indices[ IndexIndex + 1 ] = (index_t)( ( WidthSegments << 1 ) + 1 );
		}
		else
		{
			if( Plane == XY_PLANE )
			{
				Positions[ VertexIndex ] = Vector( i * RecL - HalfL, -HalfW, 0.f );
				Positions[ VertexIndex + 1 ] = Vector( i * RecL - HalfL, HalfW, 0.f );
			}
			else if( Plane == XZ_PLANE )
			{
				Positions[ VertexIndex ] = Vector( i * RecL - HalfL, 0.f, -HalfW );
				Positions[ VertexIndex + 1 ] = Vector( i * RecL - HalfL, 0.f, HalfW );
			}
			else if( Plane == YZ_PLANE )
			{
				Positions[ VertexIndex ] = Vector( 0.f, i * RecL - HalfL, -HalfW );
				Positions[ VertexIndex + 1 ] = Vector( 0.f, i * RecL - HalfL, HalfW );
			}
			Colors[ VertexIndex ] = 0xffffffff;
			Colors[ VertexIndex + 1 ] = 0xffffffff;
			Indices[ IndexIndex ] = (index_t)VertexIndex;
			Indices[ IndexIndex + 1 ] = (index_t)VertexIndex + 1;
		}
	}

	IVertexBuffer*		NewVB = m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VDecl = m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS );
	IIndexBuffer*		NewIB = m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = NumVertices;
	InitStruct.Positions = Positions;
	InitStruct.Colors = Colors;
	NewVB->Init( InitStruct );
	NewIB->Init( NumIndices, Indices );
	NewIB->SetPrimitiveType( EPT_LINELIST );
	Mesh* NewMesh = new Mesh( NewVB, VDecl, NewIB );

	if( Plane == XY_PLANE )
	{
		NewMesh->m_AABB = AABB( Vector( -HalfL, -HalfW, 0.0f ), Vector( HalfL, HalfW, 0.0f ) );
	}
	else if( Plane == XZ_PLANE )
	{
		NewMesh->m_AABB = AABB( Vector( -HalfL, 0.0f, -HalfW ), Vector( HalfL, 0.0f, HalfW ) );
	}
	else if( Plane == YZ_PLANE )
	{
		NewMesh->m_AABB = AABB( Vector( 0.0f, -HalfL, -HalfW ), Vector( 0.0f, HalfL, HalfW ) );
	}

#if BUILD_DEBUG
	NewMesh->m_Name = "Grid";
#endif

	delete Positions;
	delete Colors;
	delete Indices;

	return NewMesh;
}

Mesh* MeshFactory::CreateCylinder( float Radius, float Height, int RadialSegments, int HeightSegments )
{
	int RP = RadialSegments + 1;
	int HP = HeightSegments + 1;
	int NumVertices = ( RP * HP ) + ( 2 * RP );	// Main can + caps (with center vertices, but no duplicated vertex)
	int NumIndices = ( 6 * RadialSegments * HeightSegments ) + ( 6 * RadialSegments );

	Vector* Positions = new Vector[ NumVertices ];
	uint* Colors = new uint[ NumVertices ];
	Vector2* UVs = new Vector2[ NumVertices ];
	Vector* Normals = new Vector[ NumVertices ];
	Vector4* Tangents = new Vector4[ NumVertices ];
	index_t* Indices = new index_t[ NumIndices ];

	float RecR = 1.f / RadialSegments;
	float RecH = Height / HeightSegments;
	float HalfH = Height * .5f;
	float Angle;
	int Index;

	// Building the vertices for the main can
	for( int j = 0; j < HP; ++j )
	{
		for( int i = 0; i < RP; ++i )
		{
			Angle = 2 * PI * ( i * RecR );
			Index = ( j * RP ) + i;

			float CosA = Cos( Angle );
			float SinA = Sin( Angle );

			Positions[ Index ] = Vector( Radius * CosA, Radius * SinA, j * RecH - HalfH );
			Colors[ Index ] = 0xffffffff;
			UVs[ Index ] = Vector2( 2.f * (float)i / RadialSegments, (float)j / HeightSegments );	// NOTE: Multiplying u by two to repeat texture
			Normals[ Index ] = Vector( CosA, SinA, 0.f );
			Tangents[ Index ] = Vector4( -SinA, CosA, 0.f, 1.f );	// Bitangent = (Normal x Tangent) * Bitangent orientation
		}
	}

	int IndexTop = HP * RP;
	int IndexBottom = IndexTop + RP;

	// Building the vertices for the cap edges
	for( int i = 0; i < RadialSegments; ++i )
	{
		Angle = 2 * PI * ( i * RecR );
		float CosA = Cos( Angle );
		float SinA = Sin( Angle );

		Positions[ IndexTop ] = Vector( Radius * CosA, Radius * SinA, HalfH );
		Colors[ IndexTop ] = 0xffffffff;
		UVs[ IndexTop ] = Vector2( ( CosA + 1.f ) * .5f, ( SinA + 1.f ) * .5f );
		Normals[ IndexTop ] = Vector( 0.f, 0.f, 1.f );
		Tangents[ IndexTop ] = Vector4( -SinA, CosA, 0.f, 1.f );	// Bitangent = (Normal x Tangent) * Bitangent orientation

		Positions[ IndexBottom ] = Vector( Radius * CosA, Radius * SinA, -HalfH );
		Colors[ IndexBottom ] = 0xffffffff;
		UVs[ IndexBottom ] = Vector2( ( CosA + 1.f ) * .5f, 1.f - ( ( SinA + 1.f ) * .5f ) );
		Normals[ IndexBottom ] = Vector( 0.f, 0.f, -1.f );
		Tangents[ IndexBottom ] = Vector4( SinA, -CosA, 0.f, 1.f );	// Bitangent = (Normal x Tangent) * Bitangent orientation

		++IndexTop;
		++IndexBottom;
	}

	// And the cap centers
	Positions[ IndexTop ] = Vector( 0.f, 0.f, HalfH );
	Colors[ IndexTop ] = 0xffffffff;
	UVs[ IndexTop ] = Vector2( .5f, .5f );
	Normals[ IndexTop ] = Vector( 0.f, 0.f, 1.f );
	Tangents[ IndexTop ] = Vector4( 0.f, 1.f, 0.f, 1.f );	// What's the correct tangent for the center?

	Positions[ IndexBottom ] = Vector( 0.f, 0.f, -HalfH );
	Colors[ IndexBottom ] = 0xffffffff;
	UVs[ IndexBottom ] = Vector2( .5f, .5f );
	Normals[ IndexBottom ] = Vector( 0.f, 0.f, -1.f );
	Tangents[ IndexBottom ] = Vector4( 0.f, -1.f, 0.f, 1.f );	// What's the correct tangent for the center?

	// Now the indices for the main can
	int VertexIndex = 0;
	int IndexIndex = 0;
	for( int j = 0; j < HeightSegments; ++j )
	{
		for( int i = 0; i < RadialSegments; ++i )
		{
			Indices[ IndexIndex++ ] = (index_t)( VertexIndex );
			Indices[ IndexIndex++ ] = (index_t)( VertexIndex + 1 );
			Indices[ IndexIndex++ ] = (index_t)( VertexIndex + RP );
			Indices[ IndexIndex++ ] = (index_t)( VertexIndex + 1 );
			Indices[ IndexIndex++ ] = (index_t)( VertexIndex + RP + 1 );
			Indices[ IndexIndex++ ] = (index_t)( VertexIndex + RP );
			++VertexIndex;
		}
		++VertexIndex;
	}
	VertexIndex += RP;	// Advance past the top row to the start of the caps

	// Indices for the caps
	// IndexTop/Bottom are the center vertices
	int RM = RadialSegments - 1;
	for( int i = 0; i < RM; ++i )
	{
		Indices[ IndexIndex++ ] = (index_t)( VertexIndex );
		Indices[ IndexIndex++ ] = (index_t)( VertexIndex + 1 );
		Indices[ IndexIndex++ ] = (index_t)IndexTop;
		++VertexIndex;
	}
	// Last one has to be handled separate because there is no duplicated vert on caps
	Indices[ IndexIndex++ ] = (index_t)( VertexIndex );
	Indices[ IndexIndex++ ] = (index_t)( VertexIndex - RM );
	Indices[ IndexIndex++ ] = (index_t)IndexTop;
	VertexIndex += 2;
	for( int i = 0; i < RM; ++i )
	{
		Indices[ IndexIndex++ ] = (index_t)( VertexIndex );
		Indices[ IndexIndex++ ] = (index_t)IndexBottom;
		Indices[ IndexIndex++ ] = (index_t)( VertexIndex + 1 );
		++VertexIndex;
	}
	// Last one has to be handled separate because there is no duplicated vert on caps
	Indices[ IndexIndex++ ] = (index_t)( VertexIndex );
	Indices[ IndexIndex++ ] = (index_t)IndexBottom;
	Indices[ IndexIndex++ ] = (index_t)( VertexIndex - RM );

	IVertexBuffer*		NewVB = m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VDecl = m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS | VD_UVS | VD_NORMALS | VD_TANGENTS );
	IIndexBuffer*		NewIB = m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = NumVertices;
	InitStruct.Positions = Positions;
	InitStruct.Colors = Colors;
	InitStruct.UVs = UVs;
	InitStruct.Normals = Normals;
	InitStruct.Tangents = Tangents;
	NewVB->Init( InitStruct );
	NewIB->Init( NumIndices, Indices );
	NewIB->SetPrimitiveType( EPT_TRIANGLELIST );
	Mesh* NewMesh = new Mesh( NewVB, VDecl, NewIB );

	NewMesh->m_AABB = AABB( Vector( -Radius, -Radius, -HalfH ), Vector( Radius, Radius, HalfH ) );

#if BUILD_DEBUG
	NewMesh->m_Name = "Cylinder";
#endif

	delete Positions;
	delete Colors;
	delete UVs;
	delete Normals;
	delete Tangents;
	delete Indices;

	return NewMesh;
}

Mesh* MeshFactory::CreateCube( float Length )
{
	return CreateBox( Length, Length, Length, 1, 1, 1 );
}

Mesh* MeshFactory::CreateBox( float Length, float Width, float Height, int LengthSegments, int WidthSegments, int HeightSegments )
{
	int LP = LengthSegments + 1;
	int WP = WidthSegments + 1;
	int HP = HeightSegments + 1;
	int NumVertices = 2 * ( LP * WP + LP * HP + WP * HP );
	int NumIndices = 12 * ( LengthSegments * WidthSegments + LengthSegments * HeightSegments + WidthSegments * HeightSegments );

	Vector* Positions = new Vector[ NumVertices ];
	uint* Colors = new uint[ NumVertices ];
	Vector2* UVs = new Vector2[ NumVertices ];
	Vector* Normals = new Vector[ NumVertices ];
	Vector4* Tangents = new Vector4[ NumVertices ];
	index_t* Indices = new index_t[ NumIndices ];

	float RecL = Length / (float)LengthSegments;
	float RecW = Width / (float)WidthSegments;
	float RecH = Height / (float)HeightSegments;
	float HalfL = Length * .5f;
	float HalfW = Width * .5f;
	float HalfH = Height * .5f;

	// Top and bottom faces of box
	int Index1 = 0;
	int Index2 = WP * LP;
	for( int j = 0; j < WP; ++j )
	{
		for( int i = 0; i < LP; ++i )
		{
			// Index1 = Top, Index2 = Bottom
			Positions[ Index1 ] = Vector( i * RecL - HalfL, j * RecW - HalfW, HalfH );
			Positions[ Index2 ] = Vector( i * RecL - HalfL, j * RecW - HalfW, -HalfH );
			Colors[ Index1 ] = 0xffffffff;
			Colors[ Index2 ] = 0xffffffff;
			UVs[ Index1 ] = Vector2( (float)i / LengthSegments, (float)j / WidthSegments );
			UVs[ Index2 ] = Vector2( (float)i / LengthSegments, (float)( WidthSegments - j ) / WidthSegments );
			Normals[ Index1 ] = Vector( 0.f, 0.f, 1.f );
			Normals[ Index2 ] = Vector( 0.f, 0.f, -1.f );
			Tangents[ Index1 ] = Vector4( 1.f, 0.f, 0.f, 1.f );	// Bitangent = (Normal x Tangent) * Bitangent orientation
			Tangents[ Index2 ] = Vector4( -1.f, 0.f, 0.f, 1.f );
			++Index1;
			++Index2;
		}
	}

	// Front and back faces of box
	Index1 = 2 * WP * LP;
	Index2 = Index1 + HP * LP;
	for( int j = 0; j < HP; ++j )
	{
		for( int i = 0; i < LP; ++i )
		{
			// Index1 = Front, Index2 = Back
			Positions[ Index1 ] = Vector( i * RecL - HalfL, -HalfW, j * RecH - HalfH );
			Positions[ Index2 ] = Vector( i * RecL - HalfL, HalfW, j * RecH - HalfH );
			Colors[ Index1 ] = 0xffffffff;
			Colors[ Index2 ] = 0xffffffff;
			UVs[ Index1 ] = Vector2( (float)i / LengthSegments, (float)j / HeightSegments );
			UVs[ Index2 ] = Vector2( (float)( LengthSegments - i ) / LengthSegments, (float)j / HeightSegments );
			Normals[ Index1 ] = Vector( 0.f, -1.f, 0.f );
			Normals[ Index2 ] = Vector( 0.f, 1.f, 0.f );
			Tangents[ Index1 ] = Vector4( 1.f, 0.f, 0.f, 1.f );	// Bitangent = (Normal x Tangent) * Bitangent orientation
			Tangents[ Index2 ] = Vector4( -1.f, 0.f, 0.f, 1.f );
			++Index1;
			++Index2;
		}
	}

	// Left and right faces of box
	Index1 = 2 * ( WP * LP + HP * LP );
	Index2 = Index1 + HP * WP;
	for( int j = 0; j < HP; ++j )
	{
		for( int i = 0; i < WP; ++i )
		{
			// Index1 = Right, Index2 = Left
			Positions[ Index1 ] = Vector( HalfL, i * RecW - HalfW, j * RecH - HalfH );
			Positions[ Index2 ] = Vector( -HalfL, i * RecW - HalfW, j * RecH - HalfH );
			Colors[ Index1 ] = 0xffffffff;
			Colors[ Index2 ] = 0xffffffff;
			UVs[ Index1 ] = Vector2( (float)i / WidthSegments, (float)j / HeightSegments );
			UVs[ Index2 ] = Vector2( (float)( WidthSegments - i ) / WidthSegments, (float)j / HeightSegments );
			Normals[ Index1 ] = Vector( 1.f, 0.f, 0.f );
			Normals[ Index2 ] = Vector( -1.f, 0.f, 0.f );
			Tangents[ Index1 ] = Vector4( 0.f, 1.f, 0.f, 1.f );	// Bitangent = (Normal x Tangent) * Bitangent orientation
			Tangents[ Index2 ] = Vector4( 0.f, -1.f, 0.f, 1.f );
			++Index1;
			++Index2;
		}
	}

	// Top and bottom faces of box
	int VertexIndex1 = 0;
	int VertexIndex2 = WP * LP;
	int IndexIndex1 = 0;
	int IndexIndex2 = 6 * LengthSegments * WidthSegments;
	for( int j = 0; j < WidthSegments; ++j )
	{
		for( int i = 0; i < LengthSegments; ++i )
		{
			// Top
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + 1 );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + LP );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + 1 );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + LP + 1 );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + LP );
			// Bottom
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 + LP );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 + LP + 1 );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 + LP + 1 );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 + 1 );
			++VertexIndex1;
			++VertexIndex2;
		}
		++VertexIndex1;
		++VertexIndex2;
	}

	// Front and back faces of box
	VertexIndex1 = 2 * WP * LP;
	VertexIndex2 = VertexIndex1 + HP * LP;
	IndexIndex1 = 12 * LengthSegments * WidthSegments;
	IndexIndex2 = IndexIndex1 + ( 6 * LengthSegments * HeightSegments );
	for( int j = 0; j < HeightSegments; ++j )
	{
		for( int i = 0; i < LengthSegments; ++i )
		{
			// Front
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + 1 );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + LP );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + 1 );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + LP + 1 );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + LP );
			// Back
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 + LP );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 + LP + 1 );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 + LP + 1 );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 + 1 );
			++VertexIndex1;
			++VertexIndex2;
		}
		++VertexIndex1;
		++VertexIndex2;
	}

	// Left and right faces of box
	VertexIndex1 = 2 * ( WP * LP + HP * LP );
	VertexIndex2 = VertexIndex1 + HP * WP;
	IndexIndex1 = 12 * ( LengthSegments * WidthSegments + LengthSegments * HeightSegments );
	IndexIndex2 = IndexIndex1 + ( 6 * WidthSegments * HeightSegments );
	for( int j = 0; j < HeightSegments; ++j )
	{
		for( int i = 0; i < WidthSegments; ++i )
		{
			// Right
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + 1 );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + WP );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + 1 );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + WP + 1 );
			Indices[ IndexIndex1++ ] = (index_t)( VertexIndex1 + WP );
			// Left
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 + WP );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 + WP + 1 );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 + WP + 1 );
			Indices[ IndexIndex2++ ] = (index_t)( VertexIndex2 + 1 );
			++VertexIndex1;
			++VertexIndex2;
		}
		++VertexIndex1;
		++VertexIndex2;
	}

	IVertexBuffer*		NewVB = m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VDecl = m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS | VD_UVS | VD_NORMALS | VD_TANGENTS );
	IIndexBuffer*		NewIB = m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = NumVertices;
	InitStruct.Positions = Positions;
	InitStruct.Colors = Colors;
	InitStruct.UVs = UVs;
	InitStruct.Normals = Normals;
	InitStruct.Tangents = Tangents;
	NewVB->Init( InitStruct );
	NewIB->Init( NumIndices, Indices );
	NewIB->SetPrimitiveType( EPT_TRIANGLELIST );
	Mesh* NewMesh = new Mesh( NewVB, VDecl, NewIB );

	NewMesh->m_AABB = AABB( Vector( -HalfL, -HalfW, -HalfH ), Vector( HalfL, HalfW, HalfH ) );

	delete Positions;
	delete Colors;
	delete UVs;
	delete Normals;
	delete Tangents;
	delete Indices;

#if BUILD_DEBUG
	NewMesh->m_Name = "Box";
#endif

	return NewMesh;
}

Mesh* MeshFactory::CreateSphere( float Radius, int LongitudinalSegments, int LatitudinalSegments )
{
	// TODO: This
	Unused( Radius );
	Unused( LongitudinalSegments );
	Unused( LatitudinalSegments );
	WARN;
	return NULL;
}

Mesh* MeshFactory::CreateGeosphere( float Radius, int Refinement )
{
	// TODO: This
	// Use the method of subdividing the faces of an icosahedron (an octahedron won't work) and pushing them out to unit length.
	Unused( Radius );
	Unused( Refinement );
	WARN;
	return NULL;
}

Mesh* MeshFactory::CreateTorus( float OuterRadius, float InnerRadius, int OuterSegments, int InnerSegments )
{
	// TODO: This
	Unused( OuterRadius );
	Unused( InnerRadius );
	Unused( OuterSegments );
	Unused( InnerSegments );
	WARN;
	return NULL;
}

Mesh* MeshFactory::CreateCapsule( float Radius, float InnerLength, int CylinderSegments, int LongitudinalSegments, int LatitudinalSegments )
{
	// TODO: This
	Unused( Radius );
	Unused( InnerLength );
	Unused( CylinderSegments );
	Unused( LongitudinalSegments );
	Unused( LatitudinalSegments );
	WARN;
	return NULL;
}

Mesh* MeshFactory::CreateCone( float Radius, float Height, int RadialSegments, int HeightSegments )
{
	// TODO: This
	Unused( Radius );
	Unused( Height );
	Unused( RadialSegments );
	Unused( HeightSegments );
	WARN;
	return NULL;
}

Mesh* MeshFactory::CreateDebugLine( const Vector& Start, const Vector& End, unsigned int Color )
{
	Vector Positions[2];
	uint Colors[2];
	index_t Indices[2];

	Positions[0] = Start;
	Positions[1] = End;
	Colors[0] = Color;
	Colors[1] = Color;
	Indices[0] = 0;
	Indices[1] = 1;

	IVertexBuffer* VertexBuffer				= m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VertexDeclaration	= m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS );
	IIndexBuffer* IndexBuffer				= m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = 2;
	InitStruct.Positions = Positions;
	InitStruct.Colors = Colors;
	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( 2, Indices );
	IndexBuffer->SetPrimitiveType( EPT_LINELIST );
	Mesh* LineMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );

	LineMesh->m_AABB = AABB(
		Vector( Min( Start.x, End.x ), Min( Start.y, End.y ), Min( Start.z, End.z ) ),
		Vector( Max( Start.x, End.x ), Max( Start.y, End.y ), Max( Start.z, End.z ) ) );

#if BUILD_DEBUG
	LineMesh->m_Name = "Line";
#endif

	return LineMesh;
}

Mesh* MeshFactory::CreateDebugTriangle( const Vector& V1, const Vector& V2, const Vector& V3, unsigned int Color )
{
	Vector Positions[3];
	uint Colors[3];
	index_t Indices[6];

	Positions[0] = Vector( V1.x, V1.y, V1.z );
	Positions[1] = Vector( V2.x, V2.y, V2.z );
	Positions[2] = Vector( V3.x, V3.y, V3.z );
	Colors[0] = Color;
	Colors[1] = Color;
	Colors[2] = Color;
	Indices[0] = 0;
	Indices[1] = 1;
	Indices[2] = 1;
	Indices[3] = 2;
	Indices[4] = 2;
	Indices[5] = 0;

	IVertexBuffer*		VertexBuffer		= m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VertexDeclaration	= m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS );
	IIndexBuffer*		IndexBuffer			= m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = 3;
	InitStruct.Positions = Positions;
	InitStruct.Colors = Colors;
	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( 6, Indices );
	IndexBuffer->SetPrimitiveType( EPT_LINELIST );
	Mesh* TriMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );

	TriMesh->m_AABB = AABB(
		Vector(
			Min( Min( V1.x, V2.x ), V3.x ),
			Min( Min( V1.y, V2.y ), V3.y ),
			Min( Min( V1.z, V2.z ), V3.z ) ),
		Vector(
			Max( Max( V1.x, V2.x ), V3.x ),
			Max( Max( V1.y, V2.y ), V3.y ),
			Max( Max( V1.z, V2.z ), V3.z ) ) );

#if BUILD_DEBUG
	TriMesh->m_Name = "Triangle";
#endif

	return TriMesh;
}

// Ordered:
// V1 V2
// V3 V4
Mesh* MeshFactory::CreateDebugQuad( const Vector& V1, const Vector& V2, const Vector& V3, const Vector& V4, unsigned int Color )
{
	Vector Positions[4];
	uint Colors[4];
	index_t Indices[8];

	Positions[0] = V1;
	Positions[1] = V2;
	Positions[2] = V3;
	Positions[3] = V4;
	Colors[0] = Color;
	Colors[1] = Color;
	Colors[2] = Color;
	Colors[3] = Color;
	Indices[0] = 0;
	Indices[1] = 1;
	Indices[2] = 1;
	Indices[3] = 3;
	Indices[4] = 3;
	Indices[5] = 2;
	Indices[6] = 2;
	Indices[7] = 0;

	IVertexBuffer*		VertexBuffer		= m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VertexDeclaration	= m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS );
	IIndexBuffer*		IndexBuffer			= m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = 4;
	InitStruct.Positions = Positions;
	InitStruct.Colors = Colors;
	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( 8, Indices );
	IndexBuffer->SetPrimitiveType( EPT_LINELIST );
	Mesh* QuadMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );

	const float MinX = Min( Min( Min( V1.x, V2.x ), V3.x ), V4.x );
	const float MinY = Min( Min( Min( V1.y, V2.y ), V3.y ), V4.y );
	const float MinZ = Min( Min( Min( V1.z, V2.z ), V3.z ), V4.z );
	const float MaxX = Max( Max( Max( V1.x, V2.x ), V3.x ), V4.x );
	const float MaxY = Max( Max( Max( V1.y, V2.y ), V3.y ), V4.y );
	const float MaxZ = Max( Max( Max( V1.z, V2.z ), V3.z ), V4.z );
	const Vector MinV = Vector( MinX, MinY, MinZ );
	const Vector MaxV = Vector( MaxX, MaxY, MaxZ );

	QuadMesh->m_AABB = AABB( MinV, MaxV );

#if BUILD_DEBUG
	QuadMesh->m_Name = "DebugQuad";
#endif

	return QuadMesh;
}

Mesh* MeshFactory::CreateDebugBox( const Vector& Min, const Vector& Max, unsigned int Color )
{
	Vector Positions[8];
	uint Colors[8];
	index_t Indices[24];

	Positions[0] = Vector( Min.x, Min.y, Min.z );
	Positions[1] = Vector( Min.x, Min.y, Max.z );
	Positions[2] = Vector( Min.x, Max.y, Min.z );
	Positions[3] = Vector( Min.x, Max.y, Max.z );
	Positions[4] = Vector( Max.x, Min.y, Min.z );
	Positions[5] = Vector( Max.x, Min.y, Max.z );
	Positions[6] = Vector( Max.x, Max.y, Min.z );
	Positions[7] = Vector( Max.x, Max.y, Max.z );
	Colors[0] = Color;
	Colors[1] = Color;
	Colors[2] = Color;
	Colors[3] = Color;
	Colors[4] = Color;
	Colors[5] = Color;
	Colors[6] = Color;
	Colors[7] = Color;
	Indices[0] = 0;
	Indices[1] = 1;
	Indices[2] = 0;
	Indices[3] = 2;
	Indices[4] = 0;
	Indices[5] = 4;
	Indices[6] = 1;
	Indices[7] = 3;
	Indices[8] = 1;
	Indices[9] = 5;
	Indices[10] = 2;
	Indices[11] = 3;
	Indices[12] = 2;
	Indices[13] = 6;
	Indices[14] = 3;
	Indices[15] = 7;
	Indices[16] = 4;
	Indices[17] = 5;
	Indices[18] = 4;
	Indices[19] = 6;
	Indices[20] = 5;
	Indices[21] = 7;
	Indices[22] = 6;
	Indices[23] = 7;

	IVertexBuffer*		VertexBuffer		= m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VertexDeclaration	= m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS );
	IIndexBuffer*		IndexBuffer			= m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = 8;
	InitStruct.Positions = Positions;
	InitStruct.Colors = Colors;
	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( 24, Indices );
	IndexBuffer->SetPrimitiveType( EPT_LINELIST );
	Mesh* BoxMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );

	BoxMesh->m_AABB = AABB( Min, Max );

#if BUILD_DEBUG
	BoxMesh->m_Name = "DebugBox";
#endif

	return BoxMesh;
}

Mesh* MeshFactory::CreateDebugFrustum( const View& rView, unsigned int Color )
{
	Vector Positions[8];
	uint Colors[8];
	index_t Indices[24];

	Frustum frustum;
	rView.ApplyToFrustum( frustum );
	frustum.GetCorners( Positions );

	Colors[0] = Color;
	Colors[1] = Color;
	Colors[2] = Color;
	Colors[3] = Color;
	Colors[4] = Color;
	Colors[5] = Color;
	Colors[6] = Color;
	Colors[7] = Color;
	Indices[0] = 0;
	Indices[1] = 1;
	Indices[2] = 0;
	Indices[3] = 2;
	Indices[4] = 0;
	Indices[5] = 4;
	Indices[6] = 1;
	Indices[7] = 3;
	Indices[8] = 1;
	Indices[9] = 5;
	Indices[10] = 2;
	Indices[11] = 3;
	Indices[12] = 2;
	Indices[13] = 6;
	Indices[14] = 3;
	Indices[15] = 7;
	Indices[16] = 4;
	Indices[17] = 5;
	Indices[18] = 4;
	Indices[19] = 6;
	Indices[20] = 5;
	Indices[21] = 7;
	Indices[22] = 6;
	Indices[23] = 7;

	IVertexBuffer*		VertexBuffer		= m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VertexDeclaration	= m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS );
	IIndexBuffer*		IndexBuffer			= m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = 8;
	InitStruct.Positions = Positions;
	InitStruct.Colors = Colors;
	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( 24, Indices );
	IndexBuffer->SetPrimitiveType( EPT_LINELIST );
	Mesh* FrustumMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );

	float MinX = Positions[0].x;
	float MinY = Positions[0].y;
	float MinZ = Positions[0].z;
	float MaxX = Positions[0].x;
	float MaxY = Positions[0].y;
	float MaxZ = Positions[0].z;
	for( int i = 1; i < 8; ++i )
	{
		if( Positions[i].x < MinX ) MinX = Positions[i].x;
		if( Positions[i].y < MinY ) MinY = Positions[i].y;
		if( Positions[i].z < MinZ ) MinZ = Positions[i].z;
		if( Positions[i].x < MaxX ) MaxX = Positions[i].x;
		if( Positions[i].y < MaxY ) MaxY = Positions[i].y;
		if( Positions[i].z < MaxZ ) MaxZ = Positions[i].z;
	}

	FrustumMesh->m_AABB = AABB( Vector( MinX, MinY, MinZ ), Vector( MaxX, MaxY, MaxZ ) );

#if BUILD_DEBUG
	FrustumMesh->m_Name = "DebugFrustum";
#endif

	return FrustumMesh;
}

Mesh* MeshFactory::CreateDebugSphere( const Vector& Center, float Radius, unsigned int Color )
{
	Vector Positions[ 16 * 3 ];
	uint Colors[ 16 * 3 ];
	index_t Indices[ 16 * 6 ];

	for( uint i = 0; i < 16 * 3; ++i )
	{
		Colors[ i ] = Color;
	}

	float Mult = 2.0f * PI / 16.0f;

	for( uint i = 0; i < 16; ++i )
	{
		float u = Radius * Cos( (float)i * Mult );
		float v = Radius * Sin( (float)i * Mult );
		Positions[ i ] = Vector( Center.x + u, Center.y + v, Center.z );
		Positions[ i + 16 ] = Vector( Center.x + u, Center.y, Center.z + v );
		Positions[ i + 32 ] = Vector( Center.x, Center.y + u, Center.z + v );
		Indices[ i * 2 ] = ( index_t )( i );
		Indices[ i * 2 + 1 ] = ( index_t )( ( i + 1 ) % 16 );
		Indices[ i * 2 + 32 ] = ( index_t )( i + 16 );
		Indices[ i * 2 + 33 ] = ( index_t )( ( ( i + 1 ) % 16 ) + 16 );
		Indices[ i * 2 + 64] = ( index_t )( i + 32 );
		Indices[ i * 2 + 65 ] = ( index_t )( ( ( i + 1 ) % 16 ) + 32 );
	}

	IVertexBuffer*		VertexBuffer		= m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VertexDeclaration	= m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS );
	IIndexBuffer*		IndexBuffer			= m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = 48;
	InitStruct.Positions = Positions;
	InitStruct.Colors = Colors;
	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( 96, Indices );
	IndexBuffer->SetPrimitiveType( EPT_LINELIST );
	Mesh* SphereMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );

	Vector Radii( Radius, Radius, Radius );
	SphereMesh->m_AABB = AABB( Center - Radii, Center + Radii );

#if BUILD_DEBUG
	SphereMesh->m_Name = "DebugSphere";
#endif

	return SphereMesh;
}

Mesh* MeshFactory::CreateDebugEllipsoid( const Vector& Center, const Vector& Extents, unsigned int Color )
{
	Vector Positions[ 16 * 3 ];
	uint Colors[ 16 * 3 ];
	index_t Indices[ 16 * 6 ];

	for( uint i = 0; i < 16 * 3; ++i )
	{
		Colors[ i ] = Color;
	}

	float Mult = 2.0f * PI / 16.0f;

	for( uint i = 0; i < 16; ++i )
	{
		float u = Cos( (float)i * Mult );
		float v = Sin( (float)i * Mult );
		Positions[ i ] = Vector( Center.x + u * Extents.x, Center.y + v * Extents.y, Center.z );
		Positions[ i + 16 ] = Vector( Center.x + u * Extents.x, Center.y, Center.z + v * Extents.z );
		Positions[ i + 32 ] = Vector( Center.x, Center.y + u * Extents.y, Center.z + v * Extents.z );
		Indices[ i * 2 ] = ( index_t )( i );
		Indices[ i * 2 + 1 ] = ( index_t )( ( i + 1 ) % 16 );
		Indices[ i * 2 + 32 ] = ( index_t )( i + 16 );
		Indices[ i * 2 + 33 ] = ( index_t )( ( ( i + 1 ) % 16 ) + 16 );
		Indices[ i * 2 + 64] = ( index_t )( i + 32 );
		Indices[ i * 2 + 65 ] = ( index_t )( ( ( i + 1 ) % 16 ) + 32 );
	}

	IVertexBuffer*		VertexBuffer		= m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VertexDeclaration	= m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS );
	IIndexBuffer*		IndexBuffer			= m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = 48;
	InitStruct.Positions = Positions;
	InitStruct.Colors = Colors;
	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( 96, Indices );
	IndexBuffer->SetPrimitiveType( EPT_LINELIST );
	Mesh* EllipsoidMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );

	EllipsoidMesh->m_AABB = AABB( Center - Extents, Center + Extents );

#if BUILD_DEBUG
	EllipsoidMesh->m_Name = "DebugEllipsoid";
#endif

	return EllipsoidMesh;
}

Mesh* MeshFactory::CreateDebugCross( const Vector& Center, const float Length, unsigned int Color )
{
	Vector Positions[6];
	uint Colors[6];
	index_t Indices[6];

	Positions[0] = Vector( Center.x - Length, Center.y, Center.z );
	Positions[1] = Vector( Center.x + Length, Center.y, Center.z );
	Positions[2] = Vector( Center.x, Center.y - Length, Center.z );
	Positions[3] = Vector( Center.x, Center.y + Length, Center.z );
	Positions[4] = Vector( Center.x, Center.y, Center.z - Length );
	Positions[5] = Vector( Center.x, Center.y, Center.z + Length );
	Colors[0] = Color;
	Colors[1] = Color;
	Colors[2] = Color;
	Colors[3] = Color;
	Colors[4] = Color;
	Colors[5] = Color;
	Indices[0] = 0;
	Indices[1] = 1;
	Indices[2] = 2;
	Indices[3] = 3;
	Indices[4] = 4;
	Indices[5] = 5;

	IVertexBuffer*		VertexBuffer		= m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VertexDeclaration	= m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS );
	IIndexBuffer*		IndexBuffer			= m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = 6;
	InitStruct.Positions = Positions;
	InitStruct.Colors = Colors;
	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( 6, Indices );
	IndexBuffer->SetPrimitiveType( EPT_LINELIST );
	Mesh* CrossMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );

	const Vector Extents( Length, Length, Length );
	CrossMesh->m_AABB = AABB( Center - Extents, Center + Extents );

#if BUILD_DEBUG
	CrossMesh->m_Name = "DebugCross";
#endif

	return CrossMesh;
}

Mesh* MeshFactory::CreateDebugArrow( const Vector& Root, const Angles& Direction, const float Length, unsigned int Color )
{
	const Matrix RotationMatrix = Direction.ToMatrix();

	const float		HeadLength		= Length * 0.25f;
	const Vector	ArrowOffset		= RotationMatrix * Vector( 0.0f,		Length,			0.0f );
	const Vector	ArrowHead		= Root + ArrowOffset;
	const Vector	ArrowHeadUp		= RotationMatrix * Vector( 0.0f,		-HeadLength,	HeadLength );
	const Vector	ArrowHeadDown	= RotationMatrix * Vector( 0.0f,		-HeadLength,	-HeadLength );
	const Vector	ArrowHeadLeft	= RotationMatrix * Vector( -HeadLength,	-HeadLength,	0.0f );
	const Vector	ArrowHeadRight	= RotationMatrix * Vector( HeadLength,	-HeadLength,	0.0f );

	Vector	Positions[6];
	uint	Colors[6];
	index_t	Indices[10];

	Positions[0] = Root;
	Positions[1] = ArrowHead;
	Positions[2] = ArrowHead + ArrowHeadUp;
	Positions[3] = ArrowHead + ArrowHeadDown;
	Positions[4] = ArrowHead + ArrowHeadLeft;
	Positions[5] = ArrowHead + ArrowHeadRight;

	Colors[0] = Color;
	Colors[1] = Color;
	Colors[2] = Color;
	Colors[3] = Color;
	Colors[4] = Color;
	Colors[5] = Color;

	Indices[0] = 0;
	Indices[1] = 1;
	Indices[2] = 1;
	Indices[3] = 2;
	Indices[4] = 1;
	Indices[5] = 3;
	Indices[6] = 1;
	Indices[7] = 4;
	Indices[8] = 1;
	Indices[9] = 5;

	IVertexBuffer*		VertexBuffer		= m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VertexDeclaration	= m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS );
	IIndexBuffer*		IndexBuffer			= m_Renderer->CreateIndexBuffer();

	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices	= 6;
	InitStruct.Positions	= Positions;
	InitStruct.Colors		= Colors;

	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( 10, Indices );
	IndexBuffer->SetPrimitiveType( EPT_LINELIST );

	Mesh* ArrowMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );

	const Vector Extents( Length, Length, Length );
	ArrowMesh->m_AABB = AABB( Root - Extents, Root + Extents );

#if BUILD_DEBUG
	ArrowMesh->m_Name = "DebugArrow";
#endif

	return ArrowMesh;
}

Mesh* MeshFactory::CreateDebugChar( float Width, float Height, const Vector2& UV1, const Vector2& UV2, EPlane Plane, bool TwoSided )
{
	int NumIndices = TwoSided ? 12 : 6;
	Vector Positions[4];
	uint Colors[4];
	Vector2 UVs[4];
	index_t* Indices = new index_t[ NumIndices ];

	float HalfW = Width * .5f;
	float HalfH = Height * .5f;

	if( Plane == XY_PLANE )
	{
		Positions[0] = Vector( -HalfW, -HalfH, 0 );
		Positions[1] = Vector( HalfW, -HalfH, 0 );
		Positions[2] = Vector( -HalfW, HalfH, 0 );
		Positions[3] = Vector( HalfW, HalfH, 0 );
	}
	else if( Plane == XZ_PLANE )
	{
		Positions[0] = Vector( -HalfW, 0, -HalfH );
		Positions[1] = Vector( HalfW, 0, -HalfH );
		Positions[2] = Vector( -HalfW, 0, HalfH );
		Positions[3] = Vector( HalfW, 0, HalfH );
	}
	else if( Plane == YZ_PLANE )
	{
		Positions[0] = Vector( 0, -HalfW, -HalfH );
		Positions[1] = Vector( 0, HalfW, -HalfH );
		Positions[2] = Vector( 0, -HalfW, HalfH );
		Positions[3] = Vector( 0, HalfW, HalfH );
	}

	Colors[0] = 0xffffffff;
	Colors[1] = 0xffffffff;
	Colors[2] = 0xffffffff;
	Colors[3] = 0xffffffff;

	UVs[0] = Vector2( UV1.uv_u, UV2.uv_v );
	UVs[1] = Vector2( UV2.uv_u, UV2.uv_v );
	UVs[2] = Vector2( UV1.uv_u, UV1.uv_v );
	UVs[3] = Vector2( UV2.uv_u, UV1.uv_v );

	Indices[0] = 0;
	Indices[1] = 1;
	Indices[2] = 2;
	Indices[3] = 1;
	Indices[4] = 3;
	Indices[5] = 2;

	if( TwoSided )
	{
		Indices[6] = 0;
		Indices[7] = 2;
		Indices[8] = 1;
		Indices[9] = 1;
		Indices[10] = 2;
		Indices[11] = 3;
	}

	IVertexBuffer*		VertexBuffer		= m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	VertexDeclaration	= m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_COLORS | VD_UVS );
	IIndexBuffer*		IndexBuffer			= m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices = 4;
	InitStruct.Positions = Positions;
	InitStruct.Colors = Colors;
	InitStruct.UVs = UVs;
	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( NumIndices, Indices );
	IndexBuffer->SetPrimitiveType( EPT_TRIANGLELIST );
	Mesh* CharMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );

	if( Plane == XY_PLANE )
	{
		CharMesh->m_AABB = AABB( Vector( -HalfW, -HalfH, 0.0f ), Vector( HalfW, HalfH, 0.0f ) );
	}
	else if( Plane == XZ_PLANE )
	{
		CharMesh->m_AABB = AABB( Vector( -HalfW, 0.0f, -HalfH ), Vector( HalfW, 0.0f, HalfH ) );
	}
	else if( Plane == YZ_PLANE )
	{
		CharMesh->m_AABB = AABB( Vector( 0.0f, -HalfW, -HalfH ), Vector( 0.0f, HalfW, HalfH ) );
	}

#if BUILD_DEBUG
	CharMesh->m_Name = "DebugChar";
#endif

	delete Indices;

	return CharMesh;
}

void MeshFactory::GetDynamicMesh( const char* Filename, Mesh* pNewMesh )
{
	// Expect an allocated but uninitialized mesh
	ASSERT( pNewMesh );
	ASSERT( !pNewMesh->m_VertexBuffer );
	ASSERT( !pNewMesh->m_IndexBuffer );
	ASSERT( !pNewMesh->m_VertexDeclaration );
	ASSERT( !pNewMesh->m_Bones );

	Mesh* pManagedMesh = DynamicMeshManager::GetInstance()->GetMesh( this, Filename );
	ASSERT( pManagedMesh );

	pNewMesh->Initialize(
		pManagedMesh->m_VertexBuffer,
		pManagedMesh->m_VertexDeclaration,
		pManagedMesh->m_IndexBuffer,
		pManagedMesh->m_Bones );
	pNewMesh->m_Material = pManagedMesh->m_Material;
	pNewMesh->m_AABB = pManagedMesh->m_AABB;
}

void MeshFactory::WarmDynamicMesh( const char* Filename )
{
	DynamicMeshManager::GetInstance()->GetMesh( this, Filename );
}

// NOTE: Filename is only used for mapping anim events. It can be ignored (NULL) for static meshes.
// If pMesh is provided, an existing mesh will be initialized from this stream. In most cases, this should be null.
Mesh* MeshFactory::Read(
	const IDataStream& Stream,
	const char* Filename,
	Mesh* pMesh /*= NULL*/,
	SimpleString* pDiffuseMaterial /*= NULL*/,
	SReadMeshCallback Callback /*= SReadMeshCallback()*/ )
{
	SCompiledMeshHeader Header;

	Stream.Read( sizeof( Header ), &Header );

	ASSERT( ( sizeof( index_t ) == 4 && Header.m_LongIndices ) || ( sizeof( index_t ) == 2 && !Header.m_LongIndices ) );
	ASSERT( Header.m_MagicID == 'SMCD' );

	Vector*				Positions		= new Vector[ Header.m_NumVertices ];
	uint*				Colors			= ( Header.m_HasColors ) ? new uint[ Header.m_NumVertices ] : NULL;
#if USE_HDR
	STATICHASH( UseHDR );
	bool				UseFloatColors = ConfigManager::GetBool( sUseHDR );
	Vector4*			FloatColors1	= ( Header.m_HasColors ) ? new Vector4[ Header.m_NumVertices ] : NULL;
	Vector4*			FloatColors2	= ( Header.m_HasColors ) ? new Vector4[ Header.m_NumVertices ] : NULL;
	Vector4*			FloatColors3	= ( Header.m_HasColors ) ? new Vector4[ Header.m_NumVertices ] : NULL;
#endif
	Vector2*			UVs				= ( Header.m_HasUVs ) ? new Vector2[ Header.m_NumVertices ] : NULL;
	Vector*				Normals			= ( Header.m_HasNormals ) ? new Vector[ Header.m_NumVertices ] : NULL;
	Vector4*			Tangents		= ( Header.m_HasTangents ) ? new Vector4[ Header.m_NumVertices ] : NULL;
	SBoneData*			BoneIndices		= ( Header.m_HasSkeleton ) ? new SBoneData[ Header.m_NumVertices ] : NULL;
	SBoneData*			BoneWeights		= ( Header.m_HasSkeleton ) ? new SBoneData[ Header.m_NumVertices ] : NULL;
	index_t*			Indices			= new index_t[ Header.m_NumIndices ];
	HashedString*		BoneNames		= ( Header.m_HasSkeleton ) ? new HashedString[ Header.m_NumBones ] : NULL;
	SBone*				Bones			= ( Header.m_HasSkeleton ) ? new SBone[ Header.m_NumFrames * Header.m_NumBones ] : NULL;
	Animation*			Animations		= ( Header.m_HasSkeleton ) ? new Animation[ Header.m_NumAnims ] : NULL;
	SMaterial*			Materials		= ( Header.m_NumMaterials ) ? new SMaterial[ Header.m_NumMaterials ] : NULL;
	CollisionTriangle*	CollisionTris	= ( Header.m_NumCollisionTris ) ? new CollisionTriangle[ Header.m_NumCollisionTris ] : NULL;

	uint			CollisionMeshFlags = 0;
	AABB			BoundAABB;
	uint			VertexSignature = 0;

	//------------ Read ------------
	// Read positions
	VertexSignature |= VD_POSITIONS;
	Stream.Read( sizeof( Vector ) * Header.m_NumVertices, Positions );

	// Set colors
	if( Header.m_HasColors )
	{
		Stream.Read( sizeof( uint ) * Header.m_NumVertices, Colors );
#if USE_HDR
		Stream.Read( sizeof( Vector4 ) * Header.m_NumVertices, FloatColors1 );
		Stream.Read( sizeof( Vector4 ) * Header.m_NumVertices, FloatColors2 );
		Stream.Read( sizeof( Vector4 ) * Header.m_NumVertices, FloatColors3 );
		if( UseFloatColors )
		{
			VertexSignature |= VD_BASISCOLORS;
		}
		else
		{
			VertexSignature |= VD_COLORS;
		}
#else
		VertexSignature |= VD_COLORS;
#endif
	}

	// Read UVs
	if( UVs )
	{
		VertexSignature |= VD_UVS;
		Stream.Read( sizeof( Vector2 ) * Header.m_NumVertices, UVs );
	}

	// Read normals
	if( Normals )
	{
		VertexSignature |= VD_NORMALS;
		Stream.Read( sizeof( Vector ) * Header.m_NumVertices, Normals );
	}

	// Read tangents
	if( Tangents )
	{
		VertexSignature |= VD_TANGENTS;
		Stream.Read( sizeof( Vector4 ) * Header.m_NumVertices, Tangents );
	}

	// Read bone indices
	if( BoneIndices )
	{
		VertexSignature |= VD_BONEINDICES;
		Stream.Read( sizeof( SBoneData ) * Header.m_NumVertices, BoneIndices );
	}

	// Read bone weights
	if( BoneWeights )
	{
		VertexSignature |= VD_BONEWEIGHTS;
		Stream.Read( sizeof( SBoneData ) * Header.m_NumVertices, BoneWeights );
	}

	// Read indices
	Stream.Read( sizeof( index_t ) * Header.m_NumIndices, Indices );

	// Read bone names
	if( BoneNames )
	{
		Stream.Read( sizeof( HashedString ) * Header.m_NumBones, BoneNames );
	}

	// Read bones per frame
	if( Bones )
	{
		Stream.Read( sizeof( SBone ) * Header.m_NumFrames * Header.m_NumBones, Bones );
	}

	// Read animations
	if( Animations )
	{
		Stream.Read( sizeof( Animation ) * Header.m_NumAnims, Animations );
	}

	if( Header.m_NumMaterials )
	{
		Stream.Read( sizeof( SMaterial ) * Header.m_NumMaterials, Materials );
	}

	if( Header.m_NumCollisionTris )
	{
		Stream.Read( sizeof( CollisionTriangle ) * Header.m_NumCollisionTris, CollisionTris );
		CollisionMeshFlags = Stream.ReadUInt32();
	}

	Stream.Read( sizeof( AABB ), &BoundAABB );
	//------------ End reading ------------

	if( Callback.m_Callback )
	{
		SReadMeshBuffers Buffers;

		Buffers.m_Header = Header;
		Buffers.m_Positions = Positions;
		Buffers.m_Colors = Colors;
#if USE_HDR
		Buffers.m_FloatColors1 = FloatColors1;
		Buffers.m_FloatColors2 = FloatColors2;
		Buffers.m_FloatColors3 = FloatColors3;
#endif
		Buffers.m_UVs = UVs;
		Buffers.m_Normals = Normals;
		Buffers.m_Tangents = Tangents;
		Buffers.m_BoneIndices= BoneIndices;
		Buffers.m_BoneWeights = BoneWeights;
		Buffers.m_Indices = Indices;
		Buffers.m_BoneNames = BoneNames;
		Buffers.m_Bones = Bones;
		Buffers.m_Animations = Animations;
		Buffers.m_Materials = Materials;
		Buffers.m_CollisionTris = CollisionTris;

		Callback.m_Callback( Callback.m_Void, Buffers );
	}

	BoneArray* pBoneArray = NULL;
	if( Bones )
	{
		ASSERT( Filename );
		pBoneArray = new BoneArray;
		pBoneArray->Init( BoneNames, Bones, Filename, Animations, Header.m_NumFrames, Header.m_NumBones, Header.m_NumAnims );
	}

	IVertexBuffer*		VertexBuffer		= NULL;
	IVertexDeclaration*	VertexDeclaration	= NULL;
	IIndexBuffer*		IndexBuffer			= NULL;
	if( Header.m_NumVertices )
	{
		VertexBuffer = m_Renderer->CreateVertexBuffer();

		IVertexBuffer::SInit InitStruct;
		InitStruct.NumVertices = Header.m_NumVertices;
		InitStruct.Positions = Positions;
		InitStruct.Colors = Colors;
		InitStruct.UVs = UVs;
		InitStruct.Normals = Normals;
		InitStruct.Tangents = Tangents;
		InitStruct.BoneIndices = BoneIndices;
		InitStruct.BoneWeights = BoneWeights;

#if USE_HDR
		if( UseFloatColors )
		{
			InitStruct.Colors = NULL;
			InitStruct.FloatColors1 = FloatColors1;
			InitStruct.FloatColors2 = FloatColors2;
			InitStruct.FloatColors3 = FloatColors3;
		}
#endif
		VertexBuffer->Init( InitStruct );

		VertexDeclaration = m_Renderer->GetVertexDeclaration( VertexSignature );

		IndexBuffer = m_Renderer->CreateIndexBuffer();
		IndexBuffer->Init( Header.m_NumIndices, Indices );
		IndexBuffer->SetPrimitiveType( EPT_TRIANGLELIST );
	}

	if( pMesh )
	{
		pMesh->Initialize( VertexBuffer, VertexDeclaration, IndexBuffer, pBoneArray );
	}
	else
	{
		pMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer, pBoneArray );
	}

	// Textures are assigned to the stages in the order they are
	// read from the file--later, the m_Type property could be used
	// as a semantic to indicate texture usage (TODO).
	TextureManager* pTextureManager = m_Renderer->GetTextureManager();

	// Apply default textures in case the mesh doesn't have materials
	pMesh->SetTexture( 0, pTextureManager->GetTexture( DEFAULT_TEXTURE, TextureManager::ETL_Permanent ) );
	pMesh->SetTexture( 1, pTextureManager->GetTexture( DEFAULT_NORMAL, TextureManager::ETL_Permanent ) );
	pMesh->SetTexture( 2, pTextureManager->GetTexture( DEFAULT_SPEC, TextureManager::ETL_Permanent ) );

	for( int i = 0; i < Header.m_NumMaterials; ++i )
	{
		pMesh->SetTexture( i, pTextureManager->GetTexture( Materials[i].m_Filename, TextureManager::ETL_World ) );
	}

	// This is a bit of a kludge for passing the literal texture name
	// up so it can be used to look up in the texture/material map.
	if( pDiffuseMaterial && Header.m_NumMaterials )
	{
		*pDiffuseMaterial = Materials[0].m_Filename;
	}

	pMesh->m_CollisionMesh.m_NumTris	= Header.m_NumCollisionTris;
	pMesh->m_CollisionMesh.m_Tris		= CollisionTris;
	pMesh->m_CollisionMesh.m_Flags		= CollisionMeshFlags;

	pMesh->m_AABB = BoundAABB;

#if BUILD_DEBUG
	pMesh->m_Name = "Streamed";
#endif

	SafeDeleteArray( Positions );
	SafeDeleteArray( Colors );
#if USE_HDR
	SafeDeleteArray( FloatColors1 );
	SafeDeleteArray( FloatColors2 );
	SafeDeleteArray( FloatColors3 );
#endif
	SafeDeleteArray( UVs );
	SafeDeleteArray( Normals );
	SafeDeleteArray( Tangents );
	SafeDeleteArray( BoneIndices );
	SafeDeleteArray( BoneWeights );
	SafeDeleteArray( Indices );
	SafeDeleteArray( BoneNames );
	SafeDeleteArray( Bones );
	SafeDeleteArray( Animations );
	SafeDeleteArray( Materials );

	return pMesh;
}