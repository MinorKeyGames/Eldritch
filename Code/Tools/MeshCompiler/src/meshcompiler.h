#ifndef MESHCOMPILER_H
#define MESHCOMPILER_H

#include "vector2.h"
#include "vector4.h"
#include "vector.h"
#include "matrix.h"
#include "quat.h"
#include "meshfactory.h"
#include "hashedstring.h"
#include "3d.h"
#include "array.h"
#include "aabb.h"
#include "triangle.h"
#include "animation.h"
#include "simplestring.h"

class Segment;
class Animation;
class IDataStream;

class TiXmlElement;

struct SNamedBone
{
	HashedString	m_Name;
	Array< Quat >	m_FrameQuats;		// For rotation
	Array< Vector >	m_FrameTransVecs;	// For translation
};
class MeshCompiler
{
public:
	MeshCompiler() {}
	~MeshCompiler();
	int Compile( const char* InFilename, const char* OutFilename, bool LongIndices );

private:
	uint32	GetIndexForTempVertex();
	void	CalculateTangents();
	uint8	GetIndexForBone( const HashedString& Name );
	void	NormalizeWeights();
	void	Write( const IDataStream& Stream );
	void	InitHeader( bool LongIndices );
	void	CompileArmature( TiXmlElement* Arm );
	void	CompileFace( TiXmlElement* Face );
	void	CompileAnimations( TiXmlElement* Arm );
	void	CompileMaterial( TiXmlElement* Material );
	void	CalculateAABB();
	AABB	CalculateAABBForBasePose();
	AABB	CalculateAABBForFrame( const uint Frame );
	void	CalculateAmbientOcclusion();
	void	GetAmbientOcclusionTaps( Array< Vector >& Taps, const Matrix& TangentSpace );
	void	ComputeAmbientOcclusion( Vector& OutOcclusion, const Vector& Position, const Vector& Normal, const Matrix& TangentSpace );
	bool	TraceMesh( const Segment& s );
	Vector	ApplyBonesToPosition( const uint Frame, const uint VertexIndex ) const;

	struct SBoneWeights
	{
		float m_Data[ MAX_BONES ];
	};

	SCompiledMeshHeader	m_Header;
	Array< Vector >		m_Positions;
	Array< uint >		m_Colors;
#if USE_HDR
	Array< Vector4 >	m_FloatColors1;
	Array< Vector4 >	m_FloatColors2;
	Array< Vector4 >	m_FloatColors3;
#endif
	Array< Vector2 >	m_UVs;
	Array< Vector >		m_Normals;
	Array< Vector4 >	m_Tangents;
	Array< uint32 >		m_Indices;
	Array< SBoneData >	m_BoneIndices;	// Into m_Bones array
	Array< SBoneData >	m_ByteBoneWeights;
	Array<SBoneWeights>	m_FloatBoneWeights;
	Array< SNamedBone >	m_Bones;
	Array< Animation >	m_Animations;
	Array< SMaterial >	m_Materials;
	Array< Triangle >	m_RawTris;

	Vector				m_TempPos;
	Vector2				m_TempUV;
	Vector				m_TempNorm;
	SBoneData			m_TempBoneIndex;
	SBoneWeights		m_TempBoneWeight;

	AABB				m_AABB;

	SimpleString		m_StrippedFilename;	// Stripped of leading folders and extension(s)

	// Cached config
	float				m_AORadius;
	float				m_AOPushOut;
	bool				m_BakeAOForDynamicMeshes;
	bool				m_BakeAOForAnimatedMeshes;
	bool				m_TraceTriangleBacks;
};

#endif // MESHCOMPILER_H