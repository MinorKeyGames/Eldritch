#include "core.h"
#include "meshcompiler.h"
#include "file.h"
#include "filestream.h"
#include "configmanager.h"
#include "segment.h"
#include "idatastream.h"
#include "filestream.h"
#include "mathcore.h"

#include "TinyXML/tinyxml.h"

#include <stdio.h>
#include <memory.h>

// TODO: I make a lot of implicit assumptions here about the
// order of attributes instead of using their names. Fix it.
// That's the whole point of using XML.

void GetXMLMatrix( TiXmlAttribute* Attr, Matrix& m )
{
	for( int i = 0; i < 4; ++i )
	{
		for( int j = 0; j < 4; ++j )
		{
			m.m[i][j] = (float)Attr->DoubleValue();
			Attr = Attr->Next();
		}
	}
}

void CleanStrncpy( char* Dest, const char* Src, int Num )
{
	Num = Num - 1;
	for( int i = 0; i < Num; ++i )
	{
		if( *Src )
		{
			Dest[i] = *Src;
			++Src;
		}
		else
		{
			Dest[i] = '\0';
		}
	}
	Dest[ Num ] = '\0';
}

void ReplaceExtension( char* String, const char* Ext1, const char* Ext2 )
{
	char* SubString = strstr( String, Ext1 );
	strcpy_s( SubString, 5, Ext2 );
}

void Swizzle( Quat& q )
{
	float t = q.y;
	q.y = q.z;	// This should maybe be negated
	q.z = t;
}

void Swizzle( Vector& v )
{
	float t = v.y;
	v.y = v.z;	// This should maybe be negated too?
	v.z = t;
}

MeshCompiler::~MeshCompiler()
{
}

void MeshCompiler::InitHeader( bool LongIndices )
{
	m_Header.m_MagicID = 'SMCD';
	m_Header.m_NumVertices = 0;
	m_Header.m_NumIndices = 0;
	m_Header.m_NumFrames = 0;
	m_Header.m_NumBones = 0;
	m_Header.m_NumAnims = 0;
	m_Header.m_NumMaterials = 0;
	m_Header.m_LongIndices = LongIndices;
	m_Header.m_HasUVs = false;
	m_Header.m_HasColors = false;
	m_Header.m_HasNormals = false;
	m_Header.m_HasTangents = false;
	m_Header.m_HasSkeleton = false;
}

void MeshCompiler::CompileArmature( TiXmlElement* Arm )
{
	if( Arm )
	{
		m_Header.m_HasSkeleton = true;
		TiXmlAttribute* ArmAttr = Arm->FirstAttribute();	// "frames"
		m_Header.m_NumFrames = ArmAttr->IntValue() + 1;	// + 1 for T-pose at frame 0
		for( TiXmlElement* Bone = Arm->FirstChildElement( "bonedef" ); Bone; Bone = Bone->NextSiblingElement( "bonedef" ) )
		{
			SNamedBone sbone;
			TiXmlAttribute* BoneAttr = Bone->FirstAttribute();
			const char* pName = BoneAttr->Value();
			sbone.m_Name = HashedString( pName );

			// Push an identity so frame 0 is the T-pose
			sbone.m_FrameQuats.PushBack( Quat() );
			sbone.m_FrameTransVecs.PushBack( Vector() );

			for( TiXmlElement* Frame = Bone->FirstChildElement( "frame" ); Frame; Frame = Frame->NextSiblingElement( "frame" ) )
			{
				Matrix FrameMatrix;
				TiXmlAttribute* FrameAttr = Frame->FirstAttribute();

				// In case I ever need separate frame values
				int FrameNum = FrameAttr->IntValue();
				Unused( FrameNum );

				FrameAttr = FrameAttr->Next();
				GetXMLMatrix( FrameAttr, FrameMatrix );

				Quat FrameQuat = FrameMatrix.ToQuaternion();
				Vector FrameTransVec = FrameMatrix.GetTranslationElements();

				sbone.m_FrameQuats.PushBack( FrameQuat );
				sbone.m_FrameTransVecs.PushBack( FrameTransVec );
			}

			m_Bones.PushBack( sbone );
			++m_Header.m_NumBones;
		}

		CompileAnimations( Arm );
	}
}

void MeshCompiler::CompileAnimations( TiXmlElement* Arm )
{
	// Get named animations from the keyframes
	for( TiXmlElement* Keyf = Arm->FirstChildElement( "anim" ); Keyf; Keyf = Keyf->NextSiblingElement( "anim" ) )
	{
		Animation Animation;

		// Get starting frame and name (patch up lengths later)
		TiXmlAttribute* KeyfAttr = Keyf->FirstAttribute();
		Animation.m_StartFrame = (uint16)KeyfAttr->IntValue();

		KeyfAttr = KeyfAttr->Next();
		const char* pName = KeyfAttr->Value();
		CleanStrncpy( Animation.m_Name, pName, ANIM_NAME_LENGTH );
		Animation.m_HashedName = Animation.m_Name;

		m_Animations.PushBack( Animation );
		++m_Header.m_NumAnims;
	}
	for( int i = 0; i < ( m_Header.m_NumAnims - 1 ); ++i )
	{
		Animation& ThisAnim = m_Animations[ i ];
		Animation& NextAnim = m_Animations[ i + 1 ];
		ThisAnim.m_Length = NextAnim.m_StartFrame - ThisAnim.m_StartFrame;
	}
	// Patch up the last animation's length separately
	if( m_Header.m_NumAnims )
	{
		Animation& ThisAnim = m_Animations[ m_Header.m_NumAnims - 1 ];
		ThisAnim.m_Length = (uint16)( ( m_Header.m_NumFrames ) - ThisAnim.m_StartFrame );	// -1 because of added T-pose
	}
}

void MeshCompiler::CompileFace( TiXmlElement* Face )
{
	uint NumVertsInFace = 0;
	uint TempIndices[4];
	Vector TempPositions[4];

	for( TiXmlElement* Vert = Face->FirstChildElement( "vert" ); Vert; Vert = Vert->NextSiblingElement( "vert" ) )
	{
		// Push back cached indices to handle 4-sided faces
		if( NumVertsInFace == 3 )
		{
			m_Indices.PushBack( TempIndices[0] );
			m_Indices.PushBack( TempIndices[2] );
		}

		m_TempPos = Vector( 0.f, 0.f, 0.f );
		m_TempUV = Vector2( 0.f, 0.f );
		m_TempNorm = Vector( 0.f, 0.f, 0.f );
		m_TempBoneIndex = SBoneData();
		m_TempBoneWeight = SBoneWeights();

		TiXmlElement* Pos = Vert->FirstChildElement( "pos" );
		if( Pos )
		{
			TiXmlAttribute* PosAttr = Pos->FirstAttribute();
			m_TempPos.x = (float)PosAttr->DoubleValue();
			PosAttr = PosAttr->Next();
			m_TempPos.y = (float)PosAttr->DoubleValue();
			PosAttr = PosAttr->Next();
			m_TempPos.z = (float)PosAttr->DoubleValue();
		}

		TempPositions[ NumVertsInFace ] = m_TempPos;

		TiXmlElement* UVEl = Vert->FirstChildElement( "uv" );
		if( UVEl )
		{
			m_Header.m_HasUVs = true;
			TiXmlAttribute* UVAttr = UVEl->FirstAttribute();
			m_TempUV.x = (float)UVAttr->DoubleValue();
			UVAttr = UVAttr->Next();
			m_TempUV.y = (float)UVAttr->DoubleValue();

			// Blender uses OpenGL-style (bottom-to-top) texture coordinates;
			// For now, at least, always convert to Direct3D-style.
			m_TempUV.y = 1.0f - m_TempUV.y;
		}

		TiXmlElement* Norm = Vert->FirstChildElement( "norm" );
		if( Norm )
		{
			m_Header.m_HasNormals = true;
			TiXmlAttribute* NormAttr = Norm->FirstAttribute();
			m_TempNorm.x = (float)NormAttr->DoubleValue();
			NormAttr = NormAttr->Next();
			m_TempNorm.y = (float)NormAttr->DoubleValue();
			NormAttr = NormAttr->Next();
			m_TempNorm.z = (float)NormAttr->DoubleValue();
		}

		int bIdx = 0;
		for( TiXmlElement* Bone = Vert->FirstChildElement( "bone" ); Bone; Bone = Bone->NextSiblingElement( "bone" ) )
		{
			TiXmlAttribute* BoneAttr = Bone->FirstAttribute();
			m_TempBoneIndex.m_Data[ bIdx ] = GetIndexForBone( HashedString( BoneAttr->Value() ) );
			BoneAttr = BoneAttr->Next();
			m_TempBoneWeight.m_Data[ bIdx ] = static_cast<float>( BoneAttr->DoubleValue() );
			++bIdx;
		}

		uint32 Index = GetIndexForTempVertex();
		if( Index == 65536 && !m_Header.m_LongIndices )
		{
			PRINTF( "Warning: Exceeded 65536 indices.\n" );
			PRINTF( "\tUse -l to compile with long indices.\n" );
		}
		m_Indices.PushBack( Index );

		// Cache indices to handle 4-side faces
		TempIndices[ NumVertsInFace++ ] = Index;
	}

	if( NumVertsInFace == 4 )
	{
		m_RawTris.PushBack( Triangle( TempPositions[0], TempPositions[1], TempPositions[2] ) );
		m_RawTris.PushBack( Triangle( TempPositions[0], TempPositions[2], TempPositions[3] ) );
	}
	else
	{
		m_RawTris.PushBack( Triangle( TempPositions[0], TempPositions[1], TempPositions[2] ) );
	}
}

void MeshCompiler::CompileMaterial( TiXmlElement* Material )
{
	SMaterial NewMaterial = { 0 };

	TiXmlAttribute* MatAttr = Material->FirstAttribute();
	const char* pTypeName = MatAttr->Value();
	if( strcmp( pTypeName, "Diffuse" ) == 0 )
	{
		NewMaterial.m_Type = EMT_DIFFUSE;
	}
	else if( strcmp( pTypeName, "Normal" ) == 0 )
	{
		NewMaterial.m_Type = EMT_NORMAL;
	}

	MatAttr = MatAttr->Next();
	const char* pFilename = MatAttr->Value();
	CleanStrncpy( NewMaterial.m_Filename, FileUtil::StripLeadingUpFolders( pFilename ), MAT_FILENAME_LENGTH );

	SimpleString FilenameAsString( NewMaterial.m_Filename );
	FilenameAsString.Replace( '\\', '/' );

	CleanStrncpy( NewMaterial.m_Filename, FilenameAsString.CStr(), MAT_FILENAME_LENGTH );

	if( FilenameAsString.Contains( "NODXT" ) )
	{
		// Leave the extension alone
	}
	else
	{
		ReplaceExtension( NewMaterial.m_Filename, ".tga", ".dds" );
	}

	m_Materials.PushBack( NewMaterial );
	++m_Header.m_NumMaterials;
}

int MeshCompiler::Compile( const char* InFilename, const char* OutFilename, bool LongIndices )
{
	m_StrippedFilename = FileUtil::StripExtensions( FileUtil::StripLeadingFolders( InFilename ) );

	TiXmlDocument XMLDoc;
	XMLDoc.LoadFile( InFilename );
	TiXmlElement* RootElement = XMLDoc.FirstChildElement();	// "mesh"

	// Sanity check
	if( _stricmp( RootElement->Value(), "mesh" ) )
	{
		PRINTF( "Input file is not a valid XML mesh file.\n" );
		return -1;
	}

	STATICHASH( BakeAOForDynamicMeshes );
	STATICHASH( BakeAOForAnimatedMeshes );
	STATICHASH( TraceTriangleBacks );
	STATICHASH( DynamicAORadius );
	STATICHASH( DynamicAOPushOut );
	MAKEHASH( m_StrippedFilename );

	ConfigManager::Load( FileStream( "tools.cfg", FileStream::EFM_Read ) );
	m_BakeAOForDynamicMeshes = ConfigManager::GetArchetypeBool( sBakeAOForDynamicMeshes, ConfigManager::EmptyContext, false, sm_StrippedFilename );
	m_BakeAOForAnimatedMeshes = ConfigManager::GetArchetypeBool( sBakeAOForAnimatedMeshes, ConfigManager::EmptyContext, false, sm_StrippedFilename );
	m_TraceTriangleBacks = ConfigManager::GetArchetypeBool( sTraceTriangleBacks, ConfigManager::EmptyContext, false, sm_StrippedFilename );
	m_AORadius = ConfigManager::GetArchetypeFloat( sDynamicAORadius, ConfigManager::EmptyContext, 0.1f, sm_StrippedFilename );
	m_AOPushOut = ConfigManager::GetArchetypeFloat( sDynamicAOPushOut, ConfigManager::EmptyContext, 0.01f, sm_StrippedFilename );

	m_Header.m_LongIndices = LongIndices;

	// Get armature first, which will make it easier to handle bone references in verts
	TiXmlElement* Arm = RootElement->FirstChildElement( "armature" );
	CompileArmature( Arm );

	int NumFaces = 0;
	for( TiXmlElement* Face = RootElement->FirstChildElement( "face" ); Face; Face = Face->NextSiblingElement( "face" ) )
	{
		CompileFace( Face );
		NumFaces++;
	}

	for( TiXmlElement* Mat = RootElement->FirstChildElement( "material" ); Mat; Mat = Mat->NextSiblingElement( "material" ) )
	{
		CompileMaterial( Mat );
	}

	m_Header.m_NumVertices = m_Positions.Size();
	m_Header.m_NumIndices = m_Indices.Size();

	NormalizeWeights();

	CalculateAABB();

	if( m_Header.m_HasUVs && m_Header.m_HasNormals )
	{
		m_Header.m_HasTangents = true;
	}
	PRINTF( "Calculating tangents...\n" );
	CalculateTangents();

	CalculateAmbientOcclusion();

	PRINTF( "Compile successful!\n" );
	PRINTF( "Imported %d faces.\n", NumFaces );

	Write( FileStream( OutFilename, FileStream::EFM_Write ) );

	PRINTF( "Exported %d vertices.\n", m_Header.m_NumVertices );
	PRINTF( "Exported %d indices (%d triangles).\n", m_Header.m_NumIndices, m_Header.m_NumIndices / 3 );
	if( m_Header.m_HasSkeleton )
	{
		PRINTF( "Exported %d bones.\n", m_Header.m_NumBones );
		PRINTF( "Exported %d frames.\n", m_Header.m_NumFrames );
		PRINTF( "Exported %d animations.\n", m_Header.m_NumAnims );
	}

	return 0;
}

// Finds the vertex and returns its index, adding it to the vectors if necessary (SIDE EFFECT!)
// Don't bother confirming tangent vector; it's a product of pos and norm
uint32 MeshCompiler::GetIndexForTempVertex()
{
	uint32 NumVerts = (uint32)m_Positions.Size();

	for( uint32 VertexIndex = 0; VertexIndex < NumVerts; ++VertexIndex )
	{
		if( m_TempPos	== m_Positions[ VertexIndex ]	&&
			m_TempUV	== m_UVs[ VertexIndex ]			&&
			m_TempNorm	== m_Normals[ VertexIndex ] )
		{
			for( int BoneIndex = 0; BoneIndex < MAX_BONES; ++BoneIndex )
			{
				if( m_TempBoneIndex.m_Data[ BoneIndex ] != m_BoneIndices[ VertexIndex ].m_Data[ BoneIndex ] ||
					m_TempBoneWeight.m_Data[ BoneIndex ] != m_FloatBoneWeights[ VertexIndex ].m_Data[ BoneIndex ] )
				{
					break;
				}
			}

			// We found a match, return its index
			return VertexIndex;
		}
	}

	// Create a new vertex and return its index

	m_Positions.PushBack( m_TempPos );
	m_Colors.PushBack( 0xffffffff );
#if USE_HDR
	m_FloatColors1.PushBack( Vector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
	m_FloatColors2.PushBack( Vector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
	m_FloatColors3.PushBack( Vector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
#endif
	m_UVs.PushBack( m_TempUV );
	m_Normals.PushBack( m_TempNorm );
	m_Tangents.PushBack( Vector4() );	// The tangent will actually be computed later, if UVs and normals are used
	m_BoneIndices.PushBack( m_TempBoneIndex );
	m_FloatBoneWeights.PushBack( m_TempBoneWeight );
	m_ByteBoneWeights.PushBack( SBoneData() );

	return NumVerts;
}

void MeshCompiler::CalculateAABB()
{
	m_AABB = CalculateAABBForBasePose();

	if( m_Header.m_HasSkeleton )
	{
		for( uint Frame = 0; Frame < m_Header.m_NumFrames; ++Frame )
		{
			m_AABB.ExpandTo( CalculateAABBForFrame( Frame ) );
		}
	}
}

AABB MeshCompiler::CalculateAABBForBasePose()
{
	ASSERT( m_Positions.Size() > 0 );

	AABB RetVal = AABB( m_Positions[0] );

	for( uint VertexIndex = 1; VertexIndex < m_Positions.Size(); ++VertexIndex )
	{
		RetVal.ExpandTo( m_Positions[ VertexIndex ] );
	}

	return RetVal;
}

AABB MeshCompiler::CalculateAABBForFrame( const uint Frame )
{
	ASSERT( m_Positions.Size() > 0 );

	AABB RetVal = AABB( ApplyBonesToPosition( Frame, 0 ) );

	for( uint VertexIndex = 1; VertexIndex < m_Positions.Size(); ++VertexIndex )
	{
		RetVal.ExpandTo( ApplyBonesToPosition( Frame, VertexIndex ) );
	}

	return RetVal;
}

Vector MeshCompiler::ApplyBonesToPosition( const uint Frame, const uint VertexIndex ) const
{
	const Vector&		Position			= m_Positions[ VertexIndex ];
	const SBoneData&	Indices				= m_BoneIndices[ VertexIndex ];
	const SBoneWeights&	Weights				= m_FloatBoneWeights[ VertexIndex ];

	Vector				RetVal;

	for( uint Bone = 0; Bone < MAX_BONES; ++Bone )
	{
		const uint8			BoneIndex			= Indices.m_Data[ Bone ];
		const float			BoneWeight			= Weights.m_Data[ Bone ];

		const SNamedBone&	NamedBone			= m_Bones[ BoneIndex ];
		const Vector&		BoneTranslation		= NamedBone.m_FrameTransVecs[ Frame ];
		const Quat&			BoneOrientation		= NamedBone.m_FrameQuats[ Frame ];

		Matrix				BoneMatrix			= BoneOrientation.ToMatrix();
		BoneMatrix.SetTranslationElements( BoneTranslation );

		const Vector		TransformedPoint	= Position * BoneMatrix;

		RetVal += TransformedPoint * BoneWeight;
	}

	return RetVal;
}

// Based on the example at http://www.terathon.com/code/tangent.php
void MeshCompiler::CalculateTangents()
{
	// Notes:
	// Bitangent = (Normal x Tangent) * Bitangent orientation
	// Tangent is the tangent-space vector along the +U axis (right)
	// Bitangent is the tangent-space vector along the +V axis (up)

	// Use Gram-Schmidt to orthonormalize the vector

	// Write the handedness (direction of bitangent) in w

	Vector* tan1 = new Vector[ m_Header.m_NumVertices * 2];
	Vector* tan2 = tan1 + m_Header.m_NumVertices;
	memset( tan1, 0, m_Header.m_NumVertices * 2 * sizeof( Vector ) );

	for( uint32 i = 0; i < m_Header.m_NumIndices; i += 3 )
	{
		uint i1 = m_Indices[ i ];
		uint i2 = m_Indices[ i + 1 ];
		uint i3 = m_Indices[ i + 2 ];

		const Vector& v1 = m_Positions[ i1 ];
		const Vector& v2 = m_Positions[ i2 ];
		const Vector& v3 = m_Positions[ i3 ];

		const Vector2& w1 = m_UVs[ i1 ];
		const Vector2& w2 = m_UVs[ i2 ];
		const Vector2& w3 = m_UVs[ i3 ];

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.f / ( s1 * t2 - s2 * t1 );
		Vector sdir( ( t2 * x1 - t1 * x2 ) * r, ( t2 * y1 - t1 * y2 ) * r, ( t2 * z1 - t1 * z2 ) * r );
		Vector tdir( ( s1 * x2 - s2 * x1 ) * r, ( s1 * y2 - s2 * y1 ) * r, ( s1 * z2 - s2 * z1 ) * r );

		tan1[ i1 ] += sdir;
		tan1[ i2 ] += sdir;
		tan1[ i3 ] += sdir;

		tan2[ i1 ] += tdir;
		tan2[ i2 ] += tdir;
		tan2[ i3 ] += tdir;
	}

	for( uint32 i = 0; i < m_Header.m_NumVertices; ++i )
	{
		const Vector& n = m_Normals[i];
		const Vector& t = tan1[i];

		// Gram-Schmidt orthogonalize
		m_Tangents[i] = ( t - n * n.Dot(t) ).GetNormalized();

		// Calculate handedness
		m_Tangents[i].w = ( n.Cross(t).Dot( tan2[i] ) < 0.f ) ? -1.f : 1.f;
	}

	SafeDeleteArray( tan1 );
}

uint8 MeshCompiler::GetIndexForBone( const HashedString& Name )
{
	for( uint8 i = 0; i < m_Header.m_NumBones; ++i )
	{
		if( Name == m_Bones[i].m_Name )
		{
			return i;
		}
	}
	PRINTF( "Warning: Name refers to non-existent bone.\n" );
	return 0;
}

void MeshCompiler::NormalizeWeights()
{
	// Make weights sum to 1.0 (or 255, actually)
	for( uint32 VertexIndex = 0; VertexIndex < m_Header.m_NumVertices; ++VertexIndex )
	{
		SBoneWeights&	FloatBoneWeights	= m_FloatBoneWeights[ VertexIndex ];
		SBoneData&		ByteBoneWeights		= m_ByteBoneWeights[ VertexIndex ];

		float WeightSum = 0.0f;
		for( uint BoneIndex = 0; BoneIndex < MAX_BONES; ++BoneIndex )
		{
			WeightSum += FloatBoneWeights.m_Data[ BoneIndex ];
		}

		for( uint BoneIndex = 0; BoneIndex < MAX_BONES; ++BoneIndex )
		{
			float&	FloatBoneWeight	= FloatBoneWeights.m_Data[ BoneIndex ];
			uint8&	ByteBoneWeight	= ByteBoneWeights.m_Data[ BoneIndex ];

			FloatBoneWeight	/= WeightSum;
			ByteBoneWeight	= static_cast<uint8>( 255.0f * FloatBoneWeight );
		}
	}
}

void MeshCompiler::Write( const IDataStream& Stream )
{
	PRINTF( "Writing mesh...\n" );

	// Write header
	Stream.Write( sizeof( m_Header ), &m_Header );

	// Write positions
	Stream.Write( sizeof( Vector ) * m_Header.m_NumVertices, m_Positions.GetData() );

	// Write colors
	if( m_Header.m_HasColors )
	{
		Stream.Write( sizeof( uint ) * m_Header.m_NumVertices, m_Colors.GetData() );

#if USE_HDR
		Stream.Write( sizeof( Vector4 ) * m_Header.m_NumVertices, m_FloatColors1.GetData() );
		Stream.Write( sizeof( Vector4 ) * m_Header.m_NumVertices, m_FloatColors2.GetData() );
		Stream.Write( sizeof( Vector4 ) * m_Header.m_NumVertices, m_FloatColors3.GetData() );
#endif
	}

	// Write UVs
	if( m_Header.m_HasUVs )
	{
		Stream.Write( sizeof( Vector2 ) * m_Header.m_NumVertices, m_UVs.GetData() );
	}

	// Write normals
	if( m_Header.m_HasNormals )
	{
		Stream.Write( sizeof( Vector ) * m_Header.m_NumVertices, m_Normals.GetData() );
	}

	// Write tangents
	if( m_Header.m_HasTangents )
	{
		Stream.Write( sizeof( Vector4 ) * m_Header.m_NumVertices, m_Tangents.GetData() );
	}

	// Normalize and write weights
	if( m_Header.m_HasSkeleton )
	{
		Stream.Write( sizeof( SBoneData ) * m_Header.m_NumVertices, m_BoneIndices.GetData() );
		Stream.Write( sizeof( SBoneData ) * m_Header.m_NumVertices, m_ByteBoneWeights.GetData() );
	}

	// Write indices
	if( m_Header.m_LongIndices )
	{
		Stream.Write( sizeof( uint32 ) * m_Header.m_NumIndices, m_Indices.GetData() );
	}
	else
	{
		for( uint i = 0; i < m_Header.m_NumIndices; ++i )
		{
			Stream.WriteUInt16( ( uint16 )m_Indices[i] );
		}
	}

	if( m_Header.m_HasSkeleton )
	{
		ASSERT( m_Header.m_NumBones );
		for( uint i = 0; i < m_Header.m_NumBones; ++i )
		{
			Stream.Write( sizeof( HashedString ), &m_Bones[i].m_Name );
		}

		for( uint i = 0; i < m_Header.m_NumFrames; ++i )
		{
			for( uint j = 0; j < m_Header.m_NumBones; ++j )
			{
				Stream.Write( sizeof( Quat ), &m_Bones[j].m_FrameQuats[i] );
				Stream.Write( sizeof( Vector ), &m_Bones[j].m_FrameTransVecs[i] );
			}
		}

		if( m_Header.m_NumAnims )
		{
			Stream.Write( sizeof( Animation ) * m_Header.m_NumAnims, m_Animations.GetData() );
		}
	}

	// Materials are a bit different than other data, in that they
	// need to be interpreted (for material type and to load the
	// specified file) on the engine instead of simply loaded.
	if( m_Header.m_NumMaterials )
	{
		Stream.Write( sizeof( SMaterial ) * m_Header.m_NumMaterials, m_Materials.GetData() );
	}

	Stream.Write( sizeof( AABB ), &m_AABB );

	PRINTF( "%d bytes written.\n", Stream.GetPos() );
}

void MeshCompiler::CalculateAmbientOcclusion()
{
	// Always using a color channel even when not computing AO;
	// there were weird artifacts in the game when using meshes without
	// vertex colors now, and it's just easier to fix it here.
	m_Header.m_HasColors = true;

	if( !m_BakeAOForDynamicMeshes ||
		( m_Header.m_NumAnims && !m_BakeAOForAnimatedMeshes ) )
	{
		return;
	}

	if( m_Header.m_HasTangents )
	{
		Vector IrradianceDirections[6];
		IrradianceDirections[0] = Vector( 1.0f, 0.0f, 0.0f );
		IrradianceDirections[1] = Vector( -1.0f, 0.0f, 0.0f );
		IrradianceDirections[2] = Vector( 0.0f, 1.0f, 0.0f );
		IrradianceDirections[3] = Vector( 0.0f, -1.0f, 0.0f );
		IrradianceDirections[4] = Vector( 0.0f, 0.0f, 1.0f );
		IrradianceDirections[5] = Vector( 0.0f, 0.0f, -1.0f );

		for( uint j = 0; j < m_Header.m_NumVertices; ++j )
		{
			const Vector& Position = m_Positions[j];
			const Vector& Normal = m_Normals[j];
			const Vector4& Tangent = m_Tangents[j];
			uint& Colors = m_Colors[j];
#if USE_HDR
			Vector4& FloatColors1 = m_FloatColors1[j];
			Vector4& FloatColors2 = m_FloatColors2[j];
			Vector4& FloatColors3 = m_FloatColors3[j];
#endif

			Vector4 NonFloatColor( Colors );

			Vector Bitangent = Normal.Cross( Tangent ) * Tangent.w;
			Matrix TangentSpace(
				Tangent.x, Tangent.y, Tangent.z, 0.0f,
				Bitangent.x, Bitangent.y, Bitangent.z, 0.0f,
				Normal.x, Normal.y, Normal.z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f );

			// Do ambient occlusion for bounce lighting
			Vector Occlusion;
			ComputeAmbientOcclusion( Occlusion, Position, Normal, TangentSpace );
			float NonFloatAO = ( Occlusion.x + Occlusion.y + Occlusion.z ) / 3.0f;

			NonFloatColor = Vector4( NonFloatAO, NonFloatAO, NonFloatAO, 1.0f );
#if USE_HDR
			FloatColors1 = Vector4( Occlusion.x, Occlusion.x, Occlusion.x, 1.0f );
			FloatColors2 = Vector4( Occlusion.y, Occlusion.y, Occlusion.y, 1.0f );
			FloatColors3 = Vector4( Occlusion.z, Occlusion.z, Occlusion.z, 1.0f );
#endif

			Colors = NonFloatColor.ToColor();
		}
	}
}

bool MeshCompiler::TraceMesh( const Segment& s )
{
	if( m_RawTris.Size() )
	{
		if( s.Intersects( m_AABB ) )
		{
			for( uint j = 0; j < m_RawTris.Size(); ++j )
			{
				if(	s.Intersects( m_RawTris[j] ) ||
					( m_TraceTriangleBacks && s.Intersects( m_RawTris[j].GetFlipped() ) ) )
				{
					return true;
				}
			}
		}
	}
	return false;
}

// Get fake Poisson taps in one-third sections of
// the unit hemisphere for doing directional AO.
void MeshCompiler::GetAmbientOcclusionTaps( Array< Vector >& Taps, const Matrix& TangentSpace )
{
	Taps.Clear();
	Taps.Resize( 51 );
	Taps[0] = Vector( 0.0f, 0.81649700000000003f, 0.57735000000000003f );
	Taps[1] = Vector( -0.74401973459062531f, 0.57783472685691717f, 0.33547229837631554f );
	Taps[2] = Vector( 0.25901018724643815f, 0.68792466879887804f, 0.67799216290494024f );
	Taps[3] = Vector( -0.58988856970793446f, 0.80685558587873407f, 0.031867520521874151f );
	Taps[4] = Vector( 0.26882545565790789f, 0.93007621426021569f, 0.25038192837683715f );
	Taps[5] = Vector( 0.0041285378312679675f, 0.52339192756451713f, 0.85208206490670535f );
	Taps[6] = Vector( 0.52300845383744843f, 0.83589430002845266f, 0.16656192960729299f );
	Taps[7] = Vector( 0.67164601369536103f, 0.61759181590387924f, 0.40923340676889991f );
	Taps[8] = Vector( -0.38725619624135338f, 0.76349261896781795f, 0.51682846211711353f );
	Taps[9] = Vector( 0.55273004695713479f, 0.56155706754364609f, 0.61574601507646354f );
	Taps[10] = Vector( 0.72493806462362065f, 0.68525434936331431f, 0.069937680390617263f );
	Taps[11] = Vector( -0.57003909323433155f, 0.74767366048824102f, 0.34064575382161405f );
	Taps[12] = Vector( -0.4883240745402157f, 0.56430943593345706f, 0.66565340736820811f );
	Taps[13] = Vector( 0.24691230716222037f, 0.50036658190736905f, 0.82985998595074195f );
	Taps[14] = Vector( -0.26737561786638481f, 0.53293155768514677f, 0.80280398217363869f );
	Taps[15] = Vector( -0.33103794538084164f, 0.93594428249487172f, 0.12009237604107406f );
	Taps[16] = Vector( 0.076679712920537185f, 0.99461859155774424f, 0.069671220414983512f );

	Matrix FirstSection = Matrix::CreateRotationAboutZ( -TWOPI / 3.0f );
	Matrix ThirdSection = Matrix::CreateRotationAboutZ( TWOPI / 3.0f );

	for( uint i = 0; i < 17; ++i )
	{
		Vector Tap = Taps[ i ];
		Taps[ i ] = ( Tap * FirstSection ) * TangentSpace;
		Taps[ i + 17 ] = Tap * TangentSpace;
		Taps[ i + 34 ] = ( Tap * ThirdSection ) * TangentSpace;
	}
}

void MeshCompiler::ComputeAmbientOcclusion( Vector& OutOcclusion, const Vector& Position, const Vector& Normal, const Matrix& TangentSpace )
{
	Vector BasePosition = Position + ( Normal * m_AOPushOut );
	Array< Vector > AOTaps;
	GetAmbientOcclusionTaps( AOTaps, TangentSpace );
	uint NumOccluded1 = 0;
	uint NumOccluded2 = 0;
	uint NumOccluded3 = 0;
	for( uint k = 0; k < 17; ++k )
	{
		if( TraceMesh( Segment( BasePosition, BasePosition + ( AOTaps[ k ] * m_AORadius ) ) ) )
		{
			++NumOccluded1;
		}
		if( TraceMesh( Segment( BasePosition, BasePosition + ( AOTaps[ k + 17 ] * m_AORadius ) ) ) )
		{
			++NumOccluded2;
		}
		if( TraceMesh( Segment( BasePosition, BasePosition + ( AOTaps[ k + 34 ] * m_AORadius ) ) ) )
		{
			++NumOccluded3;
		}
	}
	OutOcclusion.x = 1.0f - ( (float)NumOccluded1 / 17.0f );
	OutOcclusion.y = 1.0f - ( (float)NumOccluded2 / 17.0f );
	OutOcclusion.z = 1.0f - ( (float)NumOccluded3 / 17.0f );
}