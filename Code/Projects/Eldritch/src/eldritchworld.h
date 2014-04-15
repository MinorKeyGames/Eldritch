#ifndef ELDRITCHWORLD_H
#define ELDRITCHWORLD_H

// Voxel world class.
// For ease, voxels are hard-coded to 1m dimensions.
// This means that the world space of an entity simply needs to be quantized
// (fractional part dropped, not rounded) to get its local voxel.

#include "eldritchirradiance.h"
#include "iindexbuffer.h"
#include "vector.h"
#include "vector2.h"
#include "vector4.h"
#include "array.h"
#include "set.h"
#include "map.h"
#include "hashedstring.h"
#include "simplestring.h"

class WBScene;
class EldritchFramework;
class EldritchNav;
class IRenderer;
class Mesh;
class Segment;
class Ray;
class AABB;
class CollisionInfo;
class IDataStream;
class EldritchWorldGen;
class ITexture;

struct SVoxelMeshBuffers;

typedef uint	vidx_t;	// Voxel index: index into linear array of voxels
typedef byte	vval_t;	// Voxel value: type of block contained in a voxel
typedef int		vpos_t;	// Voxel position: worldspace position of a voxel on an axis
typedef byte	vnbr_t;	// Voxel neighbor: bitfield of neighboring cells (see also EVoxelNeighbor)
typedef uint32	vxnb_t;	// Voxel extended neighbor: bitfield of neighboring cells including all diagonals (see also EVoxelNeighbor)
typedef byte	vdir_t;	// Voxel direction: bitfield of direction to neighbor (see also EVoxelDirection)

enum EEldritchCollisionFlags
{
	EECF_None					= 0x00,

	// Primary flags
	EECF_CollideAsWorld			= 0x01,
	EECF_CollideAsEntity		= 0x02,
	EECF_CollideAsOcclusion		= 0x04,	
	EECF_CollideAsTrace			= 0x08,
	EECF_CollideStaticEntities	= 0x10,
	EECF_CollideDynamicEntities	= 0x20,

	// Special case, overrides blocking flag optimization and tests all entities (which match the given EECF_Mask_EntityTypes type).
	// Should be used sparingly. I'm adding this as a hack for movable static entities (LizardWatson, specifically).
	EECF_CollideAllEntities		= 0x40,

	// Associative meanings
	EECF_BlocksWorld			= EECF_CollideAsWorld,
	EECF_BlocksEntities			= EECF_CollideAsEntity,
	EECF_BlocksOcclusion		= EECF_CollideAsOcclusion,
	EECF_BlocksTrace			= EECF_CollideAsTrace,
	EECF_IsStatic				= EECF_CollideStaticEntities,
	EECF_IsDynamic				= EECF_CollideDynamicEntities,

	// Usage masks
	EECF_Mask_CollideAs			= EECF_CollideAsWorld | EECF_CollideAsEntity | EECF_CollideAsOcclusion | EECF_CollideAsTrace,
	EECF_Mask_EntityTypes		= EECF_CollideStaticEntities | EECF_CollideDynamicEntities,

	// Common combinations
	EECF_Occlusion				= EECF_CollideAsOcclusion | EECF_CollideStaticEntities,
	EECF_EntityCollision		= EECF_CollideAsEntity | EECF_CollideStaticEntities | EECF_CollideDynamicEntities,
	EECF_Nav					= EECF_CollideAsEntity | EECF_CollideStaticEntities,
	EECF_Trace					= EECF_CollideAsTrace | EECF_CollideStaticEntities | EECF_CollideDynamicEntities,
};

class EldritchWorld
{
public:
	EldritchWorld();
	~EldritchWorld();

	void				Initialize();
	void				Create();
	void				GatherStats();

	void				SetCurrentWorld( const HashedString& WorldDef ) { m_CurrentWorldDef = WorldDef; }
	HashedString		GetCurrentWorld() const { return m_CurrentWorldDef; }

	void				Tick( float DeltaTime );
	void				Render() const;
#if BUILD_DEV
	void				DebugRender() const;
#endif

	EldritchFramework*	GetFramework() const { return m_Framework; }

	// Primary function for all collision tests: can be used for point, line, and sweep checks against voxels and entities.
	bool				Sweep( const Segment& SweepSegment, const Vector& HalfExtents, CollisionInfo& Info ) const;

	// More specific tests used by Sweep
	bool				SweepVoxels( const Segment& SweepSegment, const Vector& HalfExtents, CollisionInfo& Info ) const;
	bool				SweepEntities( const Segment& SweepSegment, const Vector& HalfExtents, CollisionInfo& Info ) const;

	// Helper functions implemented with Sweep
	bool				Trace( const Ray& TraceRay, CollisionInfo& Info ) const;
	bool				Trace( const Segment& TraceSegment, CollisionInfo& Info ) const;
	bool				CheckClearance( const Vector& Location, const Vector& HalfExtents, CollisionInfo& Info ) const;
	bool				PointCheck( const Vector& Location, CollisionInfo& Info ) const;
	bool				LineCheck( const Vector& Start, const Vector& End, CollisionInfo& Info ) const;
	bool				SweepCheck( const Vector& Start, const Vector& End, const Vector& HalfExtents, CollisionInfo& Info ) const;
	bool				FindSpot( Vector& InOutSpot, const Vector& Extents, CollisionInfo& Info ) const;

	void				BuildAllMeshes();

	void				PackVoxels();
	void				UnpackVoxels();

	void				Save( const IDataStream& Stream );
	void				Load( const IDataStream& Stream );

	bool				AddVoxelAt( const Vector& Location, const vval_t VoxelValue );
	void				RemoveVoxelAt( const Vector& Location );
	void				RemoveVoxelsAt( const Vector& Location, const float Radius );

	bool				AddLightAt( const Vector& Location, const float Radius, const Vector4& Color );	// Returns true if light was added
	void				RemoveLightAt( const Vector& Location );

	// Use this to update the world when something that affects it has changed.
	void				OnBoxChanged( const AABB& Box );

	const SVoxelIrradiance&	GetIrradianceAt( const Vector& Location ) const;
	const SVoxelIrradiance&	GetGlobalLight() const { return m_GlobalLight; }
	const Vector4&			GetShadowColor() const { return m_AOColor; }

	// Mainly intended for doors, but might serve some purpose elsewhere.
	SVoxelIrradiance		BlendIrradiances( const Vector& LocationA, const Vector& LocationB ) const;

	vidx_t				GetIndex( const Vector& Location ) const;
	Vector				GetVoxelCenter( const Vector& Location ) const;
	Vector				GetVoxelBase( const Vector& Location ) const;

	ITexture*			GetTileTexture() const;
	void				GetTileUVs( const uint TileIndex, Vector2& OutUVMin, Vector2& OutUVMax ) const;

	uint				GetRoomSizeX() const { return m_RoomSizeX; }
	uint				GetRoomSizeY() const { return m_RoomSizeY; }
	uint				GetRoomSizeZ() const { return m_RoomSizeZ; }
	uint				GetMapSizeX() const { return m_MapSizeX; }
	uint				GetMapSizeY() const { return m_MapSizeY; }
	uint				GetMapSizeZ() const { return m_MapSizeZ; }
	int					GetRoomIndex( const Vector& Location ) const;
	void				GetRoomCoords( const int RoomIndex, int& OutX, int& OutY, int& OutZ ) const;
	Vector				GetWorldDimensions() const;

	void				SetMaze( const Array<uint8>& Maze );
	uint8				GetMazeValue( const uint RoomIndex ) { return m_Maze[ RoomIndex ]; }

private:
	friend class EldritchTools;
	friend class EldritchWorldGen;
	friend class EldritchNav;

	struct SVoxelLight
	{
		SVoxelLight();

		Vector4	m_Color;
		float	m_Radius;
	};

	struct SVoxelDef
	{
		uint	m_SideTile;
		uint	m_TopTile;
		uint	m_BottomTile;
	};

	struct SWorldDef
	{
		SWorldDef()
		:	m_Tileset()
		,	m_ColorGrading()
		,	m_VoxelDefs()
		,	m_WorldGen( NULL )
		,	m_FogNear( 0.0f )
		,	m_FogFar( 0.0f )
		,	m_FogTexture()
		,	m_Music()
		,	m_SpawnerOverrides()
		,	m_DisplacementNoiseOctaves( 0 )
		,	m_DisplacementNoiseLookupScale( 0.0f )
		,	m_DisplacementNoiseScale()
		,	m_HiColorMin()
		,	m_HiColorMax()
		,	m_LoColorDeltaMin()
		,	m_LoColorDeltaMax()
		,	m_MidColorAlphaX()
		,	m_MidColorAlphaY()
		,	m_AOColorVMin( 0.0f )
		,	m_AOColorVMax( 0.0f )
		{
		}

		SimpleString			m_Tileset;
		SimpleString			m_ColorGrading;
		Map<vval_t, SVoxelDef>	m_VoxelDefs;	// Map from voxel value to its texture tiles
		EldritchWorldGen*		m_WorldGen;

		float					m_FogNear;
		float					m_FogFar;
		SimpleString			m_FogTexture;

		SimpleString			m_Music;

		// Override room spawner defs
		typedef Map<HashedString, SimpleString> TSpawnerMap;
		TSpawnerMap				m_SpawnerOverrides;

		uint					m_DisplacementNoiseOctaves;
		float					m_DisplacementNoiseLookupScale;
		Vector					m_DisplacementNoiseScale;

		// Global light settings for this world
		Vector					m_HiColorMin;
		Vector					m_HiColorMax;
		Vector					m_LoColorDeltaMin;
		Vector					m_LoColorDeltaMax;
		Vector					m_MidColorAlphaX;
		Vector					m_MidColorAlphaY;
		float					m_AOColorVMin;
		float					m_AOColorVMax;
	};

	void		InitializeConfig( const SimpleString& DefinitionName );
	void		InitializeWorldDefConfig( const SimpleString& WorldDefinitionName );
	void		InitializeLightConfig( const SimpleString& LightDefinitionName, SWorldDef& WorldDef );
	void		InitializeFogConfig( const SimpleString& FogDefinitionName, SWorldDef& WorldDef );

	void		CreateImpenetrableOuterWalls();
	void		RollLightColors();

	void		BuildMeshesForChangedVoxels( const Set<vidx_t>& ChangedVoxels );
	void		BuildMesh( const uint MeshIndex );
	void		BuildVertsForVoxel( const uint X, const uint Y, const uint Z, SVoxelMeshBuffers& Buffers ) const;
	Vector		ApplyDisplacementNoise( const Vector& Location, const SWorldDef& WorldDef ) const;
	void		PushColors( const class Vector4& Color, const class Vector4& AOColor, const float AOValues[], SVoxelMeshBuffers& Buffers ) const;
	void		PushUVs( const Vector2& BaseUV, const bool FlipU, const bool FlipV, SVoxelMeshBuffers& Buffers ) const;
	void		PushIndices( index_t& BaseIndex, const bool CCW, SVoxelMeshBuffers& Buffers ) const;

	void		AddAllLightInfluences();
	void		ComputeAllIrradiance();
	void		ComputeIrradianceForVoxels( const Set<vidx_t>& Voxels );
	void		ComputePartialIrradiance( vidx_t VoxelIndex, vidx_t LightVoxel, const SVoxelLight& Light );
	void		ResetIrradiance( const vidx_t VoxelIndex );
	float		GetIrradianceIncidence( const Vector& LightDir, const Vector& IrradianceDir ) const;

	// NOTE: The use of Sets here makes it easy to do multiple gathers over the same region without
	// getting duplicate voxels, but makes this more prone to allocator perf problems. Continue to watch.
	void		GatherVoxelsInRadius( Set<vidx_t>& Voxels, const vidx_t VoxelIndex, const float Radius ) const;
	void		GatherVoxelsInBox( Set<vidx_t>& Voxels, const AABB& Box ) const;
	void		GatherLightInfluenceVoxels( Set<vidx_t>& Voxels, const vidx_t VoxelIndex ) const;

	void		AddLightInfluence( const Set<vidx_t>& AffectedVoxels, const vidx_t LightVoxel );
	void		RemoveLightInfluence( const Set<vidx_t>& AffectedVoxels, const vidx_t LightVoxel );
	void		ClearLightInfluenceMap();

	bool		HasAnyNeighbors( const vnbr_t VoxelNeighbors, const vnbr_t NeighborFlags ) const;
	bool		HasAnyNeighbors( const vxnb_t VoxelExtendedNeighbors, const vxnb_t ExtendedNeighborFlags ) const;
	uint		CountNeighbors( const vxnb_t VoxelExtendedNeighbors, const vxnb_t ExtendedNeighborFlags ) const;
	float		GetAOForNeighbors( const uint Bits ) const;

	bool		IsValidIndex( const vidx_t VoxelIndex ) const;
	bool		IsValidX( const vpos_t X ) const;
	bool		IsValidY( const vpos_t Y ) const;
	bool		IsValidZ( const vpos_t Z ) const;

	void		GetCoords( const vidx_t VoxelIndex, vpos_t& OutX, vpos_t& OutY, vpos_t& OutZ ) const;
	vidx_t		GetIndex( const vpos_t X, const vpos_t Y, const vpos_t Z ) const;

	Vector		GetVoxelCenter( const vidx_t VoxelIndex ) const;
	Vector		GetVoxelBase( const vidx_t VoxelIndex ) const;

	vidx_t		GetIndexNeighborRight( const vidx_t VoxelIndex ) const;
	vidx_t		GetIndexNeighborLeft( const vidx_t VoxelIndex ) const;
	vidx_t		GetIndexNeighborFront( const vidx_t VoxelIndex ) const;
	vidx_t		GetIndexNeighborBack( const vidx_t VoxelIndex ) const;
	vidx_t		GetIndexNeighborUp( const vidx_t VoxelIndex ) const;
	vidx_t		GetIndexNeighborDown( const vidx_t VoxelIndex ) const;
	vidx_t		GetIndexNeighbor( const vidx_t VoxelIndex, const vdir_t Direction ) const;

	vval_t		GetVoxel( const vidx_t VoxelIndex ) const;
	void		SetVoxel( const vidx_t VoxelIndex, const vval_t VoxelValue );
	vval_t		SafeGetVoxel( const vidx_t VoxelIndex ) const;
	void		SafeSetVoxel( const vidx_t VoxelIndex, const vval_t VoxelValue );

	const SVoxelIrradiance&	GetIrradiance( const vidx_t VoxelIndex ) const;
	SVoxelIrradiance&		GetIrradiance( const vidx_t VoxelIndex );

	vnbr_t		GetNeighbors( const vidx_t VoxelIndex ) const;
	vxnb_t		GetExtendedNeighbors( const vidx_t VoxelIndex ) const;

	void		GetMeshCoords( const uint MeshIndex, uint& OutX, uint& OutY, uint& OutZ ) const;
	uint		GetMeshIndex( const uint MeshX, const uint MeshY, const uint MeshZ ) const;

	void		InitializeVoxelPacking();

	AABB		GetVoxelBox( const vidx_t VoxelIndex ) const;
	void		ExpandBoxAroundVoxels( const Set<vidx_t>& Voxels, AABB& InOutBox ) const;
	void		SendOnWorldChangedEvent( const AABB& ChangedBox ) const;

	void				GetRoomDimensions( uint& SizeX, uint& SizeY, uint& SizeZ ) const;

	float				GetTileU() const { return m_TileU; }
	float				GetTileV() const { return m_TileV; }

	const Vector2&		GetTileUV( const uint TilesheetIndex ) const { return m_TileUVsMap[ TilesheetIndex ]; }
	const SWorldDef&	GetWorldDef() const;
	const SVoxelDef&	GetVoxelDef( const vval_t Voxel ) const;

	bool				IsCollidable( const vval_t Voxel ) const;
	bool				IsVisible( const vval_t Voxel ) const;

	SimpleString		GetSpawnerOverride( const SimpleString& OldSpawner ) const;

	EldritchFramework*	m_Framework;

	EldritchNav*		m_Nav;

	Array<vval_t>		m_VoxelMap;				// Linear array representing a 3D array, ordered XYZ
	Array<byte>			m_PackedVoxels;			// Compressed form for sending over network
	Array<Mesh*>		m_WorldMeshes;			// Linear array representing a 3D array, ordered XYZ

	Array<Vector2>		m_TileUVsMap;			// Map from tilesheet index to base UV for that tile

	float				m_RayTraceLength;

	typedef Map<vidx_t, SVoxelLight> LightMap;
	LightMap			m_LightMap;
	Array<SVoxelIrradiance>	m_IrradianceMap;	// Note: This is a hefty chunk of data. 96 times bigger than voxel data! But it's transient.

	typedef Set<vidx_t> LightInfluences;
	typedef Array<LightInfluences> LightInfluenceMap;
	LightInfluenceMap	m_LightInfluenceMap;	// Map of the indices of lights influencing each voxel.

	Map<HashedString, SWorldDef>	m_WorldDefs;	// Map from world name to its tileset and voxel defs
	HashedString		m_CurrentWorldDef;		// Name of world theme

	Array<uint8>		m_Maze;					// Serialized copy of worldgen maze

	bool				m_DisableCompute;		// We can add and remove voxels and lights, but meshes and irradiance will not be computed while this is true

	// Voxel members
	// Voxels per room
	uint				m_RoomSizeX;			// Config
	uint				m_RoomSizeY;			// Config
	uint				m_RoomSizeZ;			// Config

	// Rooms per map
	uint				m_MapSizeX;				// Config
	uint				m_MapSizeY;				// Config
	uint				m_MapSizeZ;				// Config

	// Voxels per map
	int					m_NumVoxelsX;
	int					m_NumVoxelsY;
	int					m_NumVoxelsZ;
	uint				m_NumVoxels;

	// Voxels per mesh
	uint				m_MeshSizeX;			// Config
	uint				m_MeshSizeY;			// Config
	uint				m_MeshSizeZ;			// Config
	uint				m_MeshSize;

	// Meshes per map
	uint				m_NumMeshesX;
	uint				m_NumMeshesY;
	uint				m_NumMeshesZ;
	uint				m_NumMeshes;

	// Texture tiles per surface
	uint				m_NumTilesX;				// Config
	uint				m_NumTilesY;				// Config
	uint				m_NumTiles;
	float				m_TileU;
	float				m_TileV;
	float				m_TileUh;
	float				m_TileVh;
	float				m_TileUAdjustment;
	float				m_TileVAdjustment;

	// Vertices per mesh
	uint				m_MaxVertices;
	uint				m_MaxIndices;

	// Light colors
	SVoxelIrradiance	m_GlobalLight;
	Vector4				m_AOColor;

#if BUILD_DEV
	bool				m_TestLight;
#endif
};

#endif // ELDRITCHWORLD_H