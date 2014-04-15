#ifndef MESHFACTORY_H
#define MESHFACTORY_H

#include "3d.h"
#include "iindexbuffer.h"

class Angles;

// For loading compiled meshes------------------
#define MAT_FILENAME_LENGTH 128
enum EMaterialType
{
	EMT_DIFFUSE,
	EMT_NORMAL,
};
struct SMaterial
{
	char			m_Filename[ MAT_FILENAME_LENGTH ];
	unsigned char	m_Type;
};
//----------------------------------------------

enum EPlane
{
	XY_PLANE,
	XZ_PLANE,
	YZ_PLANE,
};

struct SCompiledMeshHeader
{
	SCompiledMeshHeader();

	unsigned long	m_MagicID;			// 'SMCD'
	unsigned long	m_NumVertices;
	unsigned long	m_NumIndices;
	unsigned long	m_NumFrames;		// Ignored unless m_HasSkeleton
	unsigned long	m_NumCollisionTris;
	unsigned char	m_NumBones;			// Ignored unless m_HasSkeleton
	unsigned char	m_NumAnims;			// Ignored unless m_HasSkeleton
	unsigned char	m_NumMaterials;
	bool			m_LongIndices;
	bool			m_HasUVs;
	bool			m_HasColors;
	bool			m_HasNormals;
	bool			m_HasTangents;
	bool			m_HasSkeleton;
};

// For buffer callback
struct SReadMeshBuffers
{
	SCompiledMeshHeader			m_Header;
	class Vector*				m_Positions;
	uint*						m_Colors;
#if USE_HDR
	class Vector4*				m_FloatColors1;
	class Vector4*				m_FloatColors2;
	class Vector4*				m_FloatColors3;
#endif
	class Vector2*				m_UVs;
	class Vector*				m_Normals;
	class Vector4*				m_Tangents;
	struct SBoneData*			m_BoneIndices;
	struct SBoneData*			m_BoneWeights;
	index_t*					m_Indices;
	class HashedString*			m_BoneNames;
	struct SBone*				m_Bones;
	class Animation*			m_Animations;
	struct SMaterial*			m_Materials;
	class CollisionTriangle*	m_CollisionTris;
};

class Mesh;
class IRenderer;
class Vector;
class Vector2;
class View;
class Vector2;
class IDataStream;
class SimpleString;

class MeshFactory
{
public:
	MeshFactory( IRenderer* Renderer );

	// TODO: Could add a UV reps parameter to these...
	Mesh* CreateQuad( float Length, EPlane Plane, bool TwoSided );
	Mesh* CreatePlane( float Length, float Width, int LengthSegments, int WidthSegments, EPlane Plane, bool TwoSided );
	Mesh* CreateSprite();	// Creates simple plane mesh in the XZ plane
	Mesh* CreateGrid( float Length, float Width, int LengthSegments, int WidthSegments, EPlane Plane );
	Mesh* CreateCylinder( float Radius, float Height, int RadialSegments, int HeightSegments );
	Mesh* CreateCube( float Length );
	Mesh* CreateBox( float Length, float Width, float Height, int LengthSegments, int WidthSegments, int HeightSegments );
	Mesh* CreateSphere( float Radius, int LongitudinalSegments, int LatitudinalSegments );
	Mesh* CreateGeosphere( float Radius, int Refinement );
	Mesh* CreateTorus( float OuterRadius, float InnerRadius, int OuterSegments, int InnerSegments );
	Mesh* CreateCapsule( float Radius, float InnerLength, int CylinderSegments, int LongitudinalSegments, int LatitudinalSegments );
	Mesh* CreateCone( float Radius, float Height, int RadialSegments, int HeightSegments );

	// These functions will work in Release mode, but the meshes are inefficient,
	// so they're generally to be avoided. They could be used in tools, though.
	Mesh* CreateDebugLine( const Vector& Start, const Vector& End, unsigned int Color );
	Mesh* CreateDebugTriangle( const Vector& V1, const Vector& V2, const Vector& V3, unsigned int Color );
	Mesh* CreateDebugQuad( const Vector& V1, const Vector& V2, const Vector& V3, const Vector& V4, unsigned int Color );
	Mesh* CreateDebugBox( const Vector& Min, const Vector& Max, unsigned int Color );
	Mesh* CreateDebugFrustum( const View& rView, unsigned int Color );
	Mesh* CreateDebugChar( float Width, float Height, const Vector2& UV1, const Vector2& UV2, EPlane Plane, bool TwoSided );
	Mesh* CreateDebugSphere( const Vector& Center, float Radius, unsigned int Color );
	Mesh* CreateDebugEllipsoid( const Vector& Center, const Vector& Extents, unsigned int Color );
	Mesh* CreateDebugCross( const Vector& Center, const float Length, unsigned int Color );
	Mesh* CreateDebugArrow( const Vector& Root, const Angles& Direction, const float Length, unsigned int Color );

	// Callback stuff in case I need to copy the buffers for use in game
	typedef void ( *ReadMeshCallback )( void*, const SReadMeshBuffers& );
	struct SReadMeshCallback
	{
		SReadMeshCallback()
			:	m_Callback( NULL )
			,	m_Void( NULL )
		{
		}

		ReadMeshCallback	m_Callback;
		void*				m_Void;
	};

	// If given pMesh, will not new up another mesh
	// HACK: If given pDiffuseMaterial, will fill it with the filename in the base material
	Mesh* Read(
		const IDataStream& Stream,
		const char* Filename,								// Used for initializing bone arrays for a particular mesh
		Mesh* pMesh = NULL,
		SimpleString* pDiffuseMaterial = NULL,
		SReadMeshCallback Callback = SReadMeshCallback() );

	void GetDynamicMesh( const char* Filename, Mesh* pNewMesh );	// Read from the filename if not already in the DynamicMeshManager
	void WarmDynamicMesh( const char* Filename );					// Just request that the given mesh be loaded and cached if it is not already

private:
	IRenderer* m_Renderer;
};

#endif // MESHFACTORY_H