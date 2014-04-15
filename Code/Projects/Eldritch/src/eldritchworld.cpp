#include "core.h"
#include "eldritchworld.h"
#include "eldritchframework.h"
#include "eldritchnav.h"
#include "wbworld.h"
#include "wbscene.h"
#include "wbentity.h"
#include "idatastream.h"
#include "configmanager.h"
#include "Components/wbcompeldtransform.h"
#include "Components/wbcompeldcollision.h"
#include "mesh.h"
#include "ivertexbuffer.h"
#include "iindexbuffer.h"
#include "ivertexdeclaration.h"
#include "mathfunc.h"
#include "vector.h"
#include "vector4.h"
#include "mathcore.h"
#include "ray.h"
#include "segment.h"
#include "collisioninfo.h"
#include "aabb.h"
#include "eldritchirradiance.h"
#include "wbeventmanager.h"
#include "wbcomponentarrays.h"
#include "irenderer.h"
#include "texturemanager.h"
#include "shadermanager.h"
#include "zlib.h"
#include "eldritchworldgen.h"
#include "hsv.h"
#include "eldritchgame.h"
#include "iaudiosystem.h"
#include "noise.h"

#define DO_VOXEL_NOISE_DISPLACEMENT				0
#define DO_VOXEL_MIDPOINT_NOISE_DISPLACEMENT	1
#define DO_VOXEL_MIDPOINT_FIXED_DISPLACEMENT	0

// Voxel dimensions are hard-coded (here and in other places not based on this variable).
// This simplifies a bunch of stuff and should never need to change for Eldritch.
static const Vector	kVoxelHalfExtents	= Vector( 0.5f, 0.5f, 0.5f );
static const Vector	kVoxelExtents		= Vector( 1.0f, 1.0f, 1.0f );

static const vidx_t	kInvalidVoxelIndex		= 0xffffffff;
static const vval_t	kEmptyVoxelValue		= 0x00;
static const vval_t	kInvalidVoxelValue		= 0xff;
static const vval_t kImpenetrableVoxelFlag	= 0x80;

static const Vector	kIrradianceDirs[6]	=
{
	Vector( -1.0f, 0.0f, 0.0f ),
	Vector( 1.0f, 0.0f, 0.0f ),
	Vector( 0.0f, -1.0f, 0.0f ),
	Vector( 0.0f, 1.0f, 0.0f ),
	Vector( 0.0f, 0.0f, -1.0f ),
	Vector( 0.0f, 0.0f, 1.0f )
};

enum EVoxelNeighbor
{
	ENeighborNone	= 0x00,

	ENeighborRight	= 0x01,
	ENeighborLeft	= 0x02,
	ENeighborFront	= 0x04,
	ENeighborBack	= 0x08,
	ENeighborUp		= 0x10,
	ENeighborDown	= 0x20,

	ENeighborRFU	= 0x00000100,
	ENeighborRFC	= 0x00000200,
	ENeighborRFD	= 0x00000400,
	ENeighborRCU	= 0x00000800,
	ENeighborRCC	= ENeighborRight,
	ENeighborRCD	= 0x00001000,
	ENeighborRBU	= 0x00002000,
	ENeighborRBC	= 0x00004000,
	ENeighborRBD	= 0x00008000,
	ENeighborCFU	= 0x00010000,
	ENeighborCFC	= ENeighborFront,
	ENeighborCFD	= 0x00020000,
	ENeighborCCU	= ENeighborUp,
	ENeighborCCC	= ENeighborNone,
	ENeighborCCD	= ENeighborDown,
	ENeighborCBU	= 0x00040000,
	ENeighborCBC	= ENeighborBack,
	ENeighborCBD	= 0x00080000,
	ENeighborLFU	= 0x00100000,
	ENeighborLFC	= 0x00200000,
	ENeighborLFD	= 0x00400000,
	ENeighborLCU	= 0x00800000,
	ENeighborLCC	= ENeighborLeft,
	ENeighborLCD	= 0x01000000,
	ENeighborLBU	= 0x02000000,
	ENeighborLBC	= 0x04000000,
	ENeighborLBD	= 0x08000000,
};

enum EVoxelDirection
{
	EDirectionNone	= 0x00,

	EDirectionRight	= 0x01,
	EDirectionLeft	= 0x02,
	EDirectionFront	= 0x04,
	EDirectionBack	= 0x08,
	EDirectionUp	= 0x10,
	EDirectionDown	= 0x20,

	EDirectionRFU	= EDirectionRight | EDirectionFront | EDirectionUp,
	EDirectionRFC	= EDirectionRight | EDirectionFront,
	EDirectionRFD	= EDirectionRight | EDirectionFront | EDirectionDown,
	EDirectionRCU	= EDirectionRight | EDirectionUp,
	EDirectionRCC	= EDirectionRight,
	EDirectionRCD	= EDirectionRight | EDirectionDown,
	EDirectionRBU	= EDirectionRight | EDirectionBack | EDirectionUp,
	EDirectionRBC	= EDirectionRight | EDirectionBack,
	EDirectionRBD	= EDirectionRight | EDirectionBack | EDirectionDown,
	EDirectionCFU	= EDirectionFront | EDirectionUp,
	EDirectionCFC	= EDirectionFront,
	EDirectionCFD	= EDirectionFront | EDirectionDown,
	EDirectionCCU	= EDirectionUp,
	EDirectionCCC	= EDirectionRight,
	EDirectionCCD	= EDirectionDown,
	EDirectionCBU	= EDirectionBack | EDirectionUp,
	EDirectionCBC	= EDirectionBack,
	EDirectionCBD	= EDirectionBack | EDirectionDown,
	EDirectionLFU	= EDirectionLeft | EDirectionFront | EDirectionUp,
	EDirectionLFC	= EDirectionLeft | EDirectionFront,
	EDirectionLFD	= EDirectionLeft | EDirectionFront | EDirectionDown,
	EDirectionLCU	= EDirectionLeft | EDirectionUp,
	EDirectionLCC	= EDirectionLeft,
	EDirectionLCD	= EDirectionLeft | EDirectionDown,
	EDirectionLBU	= EDirectionLeft | EDirectionBack | EDirectionUp,
	EDirectionLBC	= EDirectionLeft | EDirectionBack,
	EDirectionLBD	= EDirectionLeft | EDirectionBack | EDirectionDown,
};

EldritchWorld::SVoxelLight::SVoxelLight()
:	m_Color()
,	m_Radius( 0.0f )
{
}

struct SVoxelMeshBuffers
{
	Array<Vector>	Positions;
	Array<Vector4>	Colors;
	Array<Vector2>	UVs;
	Array<index_t>	Indices;
};

EldritchWorld::EldritchWorld()
:	m_Framework( NULL )
,	m_Nav( NULL )
,	m_VoxelMap()
,	m_PackedVoxels()
,	m_WorldMeshes()
,	m_TileUVsMap()
,	m_RayTraceLength( 0.0f )
,	m_LightMap()
,	m_IrradianceMap()
,	m_LightInfluenceMap()
,	m_WorldDefs()
,	m_CurrentWorldDef()
,	m_Maze()
,	m_DisableCompute( false )
,	m_RoomSizeX( 0 )
,	m_RoomSizeY( 0 )
,	m_RoomSizeZ( 0 )
,	m_MapSizeX( 0 )
,	m_MapSizeY( 0 )
,	m_MapSizeZ( 0 )
,	m_NumVoxelsX( 0 )
,	m_NumVoxelsY( 0 )
,	m_NumVoxelsZ( 0 )
,	m_NumVoxels( 0 )
,	m_MeshSizeX( 0 )
,	m_MeshSizeY( 0 )
,	m_MeshSizeZ( 0 )
,	m_MeshSize( 0 )
,	m_NumMeshesX( 0 )
,	m_NumMeshesY( 0 )
,	m_NumMeshesZ( 0 )
,	m_NumMeshes( 0 )
,	m_NumTilesX( 0 )
,	m_NumTilesY( 0 )
,	m_NumTiles( 0 )
,	m_TileU( 0.0f )
,	m_TileV( 0.0f )
,	m_TileUh( 0.0f )
,	m_TileVh( 0.0f )
,	m_TileUAdjustment( 0.0f )
,	m_TileVAdjustment( 0.0f )
,	m_MaxVertices( 0 )
,	m_MaxIndices( 0 )
,	m_GlobalLight()
,	m_AOColor()
#if BUILD_DEV
,	m_TestLight( false )
#endif
{
}

EldritchWorld::~EldritchWorld()
{
	SafeDelete( m_Nav );

	for( uint WorldMeshIndex = 0; WorldMeshIndex < m_WorldMeshes.Size(); ++WorldMeshIndex )
	{
		SafeDelete( m_WorldMeshes[ WorldMeshIndex ] );
	}
	m_WorldMeshes.Clear();

	FOR_EACH_MAP( WorldDefIter, m_WorldDefs, HashedString, SWorldDef )
	{
		SWorldDef& WorldDef = WorldDefIter.GetValue();
		SafeDelete( WorldDef.m_WorldGen );
	}
}

void EldritchWorld::InitializeVoxelPacking()
{
	const uint UnpackedVoxelSize = m_NumVoxels * sizeof( vval_t );
	const uint PackedVoxelSize = compressBound( UnpackedVoxelSize );
	m_PackedVoxels.SetDeflate( false );
	m_PackedVoxels.Reserve( PackedVoxelSize );
	m_PackedVoxels.Resize( PackedVoxelSize );
}

void EldritchWorld::PackVoxels()
{
	const uint UnpackedVoxelSize = m_NumVoxels * sizeof( vval_t );
	uint32 PackedVoxelSize = compressBound( UnpackedVoxelSize );
	ASSERT( m_PackedVoxels.CheckCapacity() >= PackedVoxelSize );
	compress( m_PackedVoxels.GetData(), &PackedVoxelSize, m_VoxelMap.GetData(), UnpackedVoxelSize );
	m_PackedVoxels.Resize( PackedVoxelSize );
}

void EldritchWorld::UnpackVoxels()
{
	const uint UnpackedVoxelSize = m_NumVoxels * sizeof( vval_t );
	uint32 DestinationSize = UnpackedVoxelSize;	// zlib writes the size of the compressed buffer (m_PackedVoxels.Size()?) to this for some reason.
	uncompress( m_VoxelMap.GetData(), &DestinationSize, m_PackedVoxels.GetData(), m_PackedVoxels.Size() );
}

void EldritchWorld::InitializeConfig( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( RoomSizeX );
	m_RoomSizeX	= ConfigManager::GetInt( sRoomSizeX, 0, sDefinitionName );

	STATICHASH( RoomSizeY );
	m_RoomSizeY	= ConfigManager::GetInt( sRoomSizeY, 0, sDefinitionName );

	STATICHASH( RoomSizeZ );
	m_RoomSizeZ	= ConfigManager::GetInt( sRoomSizeZ, 0, sDefinitionName );

	STATICHASH( MapSizeX );
	m_MapSizeX	= ConfigManager::GetInt( sMapSizeX, 0, sDefinitionName );

	STATICHASH( MapSizeY );
	m_MapSizeY	= ConfigManager::GetInt( sMapSizeY, 0, sDefinitionName );

	STATICHASH( MapSizeZ );
	m_MapSizeZ	= ConfigManager::GetInt( sMapSizeZ, 0, sDefinitionName );

	STATICHASH( MeshSizeX );
	m_MeshSizeX	= ConfigManager::GetInt( sMeshSizeX, 0, sDefinitionName );

	STATICHASH( MeshSizeY );
	m_MeshSizeY	= ConfigManager::GetInt( sMeshSizeY, 0, sDefinitionName );

	STATICHASH( MeshSizeZ );
	m_MeshSizeZ	= ConfigManager::GetInt( sMeshSizeZ, 0, sDefinitionName );

	// Add 2 to each dimension to provide a cushion on every side of the world.
	// These will be unbreakable walls around the generated rooms.
	m_NumVoxelsX	= m_RoomSizeX * m_MapSizeX + 2;
	m_NumVoxelsY	= m_RoomSizeY * m_MapSizeY + 2;
	m_NumVoxelsZ	= m_RoomSizeZ * m_MapSizeZ + 2;
	m_NumVoxels		= m_NumVoxelsX * m_NumVoxelsY * m_NumVoxelsZ;

	m_RayTraceLength = SqRt( static_cast<float>( Square( m_NumVoxelsX ) + Square( m_NumVoxelsY ) + Square( m_NumVoxelsZ ) ) );

	m_MeshSize		= m_MeshSizeX * m_MeshSizeY * m_MeshSizeZ;

	m_NumMeshesX	= ( m_NumVoxelsX + ( m_MeshSizeX - 1 ) ) / m_MeshSizeX;
	m_NumMeshesY	= ( m_NumVoxelsY + ( m_MeshSizeY - 1 ) ) / m_MeshSizeY;
	m_NumMeshesZ	= ( m_NumVoxelsZ + ( m_MeshSizeZ - 1 ) ) / m_MeshSizeZ;
	m_NumMeshes		= m_NumMeshesX * m_NumMeshesY * m_NumMeshesZ;

	m_MaxVertices	= m_MeshSize * 4 * 6 / 2;		// Theoretical max: enough for every other block to have 4 verts on all 6 faces
	m_MaxIndices	= m_MeshSize * 3 * 2 * 6 / 2;	// Theoretical max: enough for every other block to have 2 tris on all 6 faces

	ASSERT( m_MaxVertices < 65536 );

	STATICHASH( TileSetSizeX );
	STATICHASH( TileSetSizeY );
	m_NumTilesX		= ConfigManager::GetInt( sTileSetSizeX, 0, sDefinitionName );
	m_NumTilesY		= ConfigManager::GetInt( sTileSetSizeY, 0, sDefinitionName );
	m_NumTiles		= m_NumTilesX * m_NumTilesY;
	m_TileU			= 1.0f / m_NumTilesX;
	m_TileV			= 1.0f / m_NumTilesY;
	m_TileUh		= 0.5f * m_TileU;
	m_TileVh		= 0.5f * m_TileV;

	STATICHASH( UVAdjustment );
	const float UVAdjustment	= ConfigManager::GetFloat( sUVAdjustment, 0.0f, sDefinitionName );
	m_TileUAdjustment			= m_TileU * UVAdjustment;
	m_TileVAdjustment			= m_TileV * UVAdjustment;

#if BUILD_DEV
	STATICHASH( TestLight );
	m_TestLight = ConfigManager::GetBool( sTestLight, false, sDefinitionName );
#endif

	// Initialize world defs
	STATICHASH( NumWorldDefs );
	const uint NumWorldDefs = ConfigManager::GetInt( sNumWorldDefs, 0, sDefinitionName );

	for( uint WorldDefIndex = 0; WorldDefIndex < NumWorldDefs; ++WorldDefIndex )
	{
		const SimpleString WorldDefName = ConfigManager::GetSequenceString( "WorldDef%d", WorldDefIndex, "", sDefinitionName );
		ASSERT( WorldDefName != "" );

		InitializeWorldDefConfig( WorldDefName );
	}
}

void EldritchWorld::InitializeWorldDefConfig( const SimpleString& WorldDefinitionName )
{
	MAKEHASH( WorldDefinitionName );

	SWorldDef& NewWorldDef = m_WorldDefs[ WorldDefinitionName ];

	STATICHASH( Tileset );
	NewWorldDef.m_Tileset = ConfigManager::GetInheritedString( sTileset, "", sWorldDefinitionName );

	STATICHASH( ColorGrading );
	NewWorldDef.m_ColorGrading = ConfigManager::GetInheritedString( sColorGrading, "", sWorldDefinitionName );

	STATICHASH( LightDef );
	const SimpleString LightDefinitionName = ConfigManager::GetInheritedString( sLightDef, "", sWorldDefinitionName );
	InitializeLightConfig( LightDefinitionName, NewWorldDef );

	STATICHASH( FogDef );
	const SimpleString FogDefinitionName = ConfigManager::GetInheritedString( sFogDef, "", sWorldDefinitionName );
	InitializeFogConfig( FogDefinitionName, NewWorldDef );

	STATICHASH( Music );
	NewWorldDef.m_Music = ConfigManager::GetInheritedString( sMusic, "", sWorldDefinitionName );

	// I only need to do this with Audiere, because it doesn't support streams.
	// Doing it with FMOD causes memory leaks because the sounds are never instanced
	// and nothing owns them.
	// "Warm" music files so they don't hitch when first played
	//if( NewWorldDef.m_Music != "" )
	//{
	//	IAudioSystem* const pAudioSystem = EldritchFramework::GetInstance()->GetAudioSystem();
	//	pAudioSystem->GetSound( NewWorldDef.m_Music );
	//}

	STATICHASH( DisplacementNoiseOctaves );
	NewWorldDef.m_DisplacementNoiseOctaves = ConfigManager::GetInheritedInt( sDisplacementNoiseOctaves, 0, sWorldDefinitionName );

	if( NewWorldDef.m_DisplacementNoiseOctaves > 0 )
	{
		STATICHASH( DisplacementNoiseScale );
		const float DisplacementNoiseScaleXY	= ConfigManager::GetInheritedFloat( sDisplacementNoiseScale, 0.0f, sWorldDefinitionName );
		NewWorldDef.m_DisplacementNoiseScale.x	= DisplacementNoiseScaleXY;
		NewWorldDef.m_DisplacementNoiseScale.y	= DisplacementNoiseScaleXY;

		STATICHASH( DisplacementNoiseScaleZ );
		NewWorldDef.m_DisplacementNoiseScale.z	= ConfigManager::GetInheritedFloat( sDisplacementNoiseScaleZ, DisplacementNoiseScaleXY, sWorldDefinitionName );

		const float OctaveScale					= 1.0f / static_cast<float>( Noise::GetOctaveScale( NewWorldDef.m_DisplacementNoiseOctaves ) );
		NewWorldDef.m_DisplacementNoiseScale	*= OctaveScale;

		STATICHASH( DisplacementNoiseLookupScale );
		NewWorldDef.m_DisplacementNoiseLookupScale = ConfigManager::GetInheritedFloat( sDisplacementNoiseLookupScale, 0.0f, sWorldDefinitionName );
	}

	STATICHASH( NumSpawnerOverrides );
	const uint NumSpawnerOverrides = ConfigManager::GetInheritedInt( sNumSpawnerOverrides, 0, sWorldDefinitionName );
	for( uint SpawnerOverrideIndex = 0; SpawnerOverrideIndex < NumSpawnerOverrides; ++SpawnerOverrideIndex )
	{
		const HashedString OldSpawner = ConfigManager::GetInheritedSequenceHash( "SpawnerOverride%dOld", SpawnerOverrideIndex, "", sWorldDefinitionName );
		const SimpleString NewSpawner = ConfigManager::GetInheritedSequenceString( "SpawnerOverride%dNew", SpawnerOverrideIndex, "", sWorldDefinitionName );

		NewWorldDef.m_SpawnerOverrides[ OldSpawner ] = NewSpawner;
	}

	// Initialize voxel defs for world
	STATICHASH( NumVoxelDefs );
	const uint NumVoxelDefs = ConfigManager::GetInheritedInt( sNumVoxelDefs, 0, sWorldDefinitionName );

	for( uint VoxelDefIndex = 0; VoxelDefIndex < NumVoxelDefs; ++VoxelDefIndex )
	{
		const SimpleString VoxelDefName = ConfigManager::GetInheritedSequenceString( "VoxelDef%d", VoxelDefIndex, "", sWorldDefinitionName );

		STATICHASH( VoxelValue );
		STATICHASH( SideTile );
		STATICHASH( TopTile );
		STATICHASH( BottomTile );
		MAKEHASH( VoxelDefName );

		const vval_t VoxelValue = static_cast<vval_t>( ConfigManager::GetInt( sVoxelValue, 0, sVoxelDefName ) );
		ASSERT( VoxelValue != kEmptyVoxelValue );
		ASSERT( VoxelValue != kInvalidVoxelValue );

		SVoxelDef& NewVoxelDef = NewWorldDef.m_VoxelDefs[ VoxelValue ];

		NewVoxelDef.m_SideTile		= ConfigManager::GetInt( sSideTile, 0, sVoxelDefName );
		NewVoxelDef.m_TopTile		= ConfigManager::GetInt( sTopTile, 0, sVoxelDefName );
		NewVoxelDef.m_BottomTile	= ConfigManager::GetInt( sBottomTile, 0, sVoxelDefName );
	}

	// Each world gets its own world gen objects, which seems a little weird, but
	// it's a good place to store room defs and per-world generation properties.
	STATICHASH( WorldGenDef );
	const SimpleString WorldGenDefName = ConfigManager::GetInheritedString( sWorldGenDef, "", sWorldDefinitionName );
	ASSERT( WorldGenDefName != "" );

	NewWorldDef.m_WorldGen = new EldritchWorldGen;
	NewWorldDef.m_WorldGen->Initialize( WorldGenDefName );
}

void EldritchWorld::InitializeLightConfig( const SimpleString& LightDefinitionName, SWorldDef& WorldDef )
{
	MAKEHASH( LightDefinitionName );

	STATICHASH( HiColorH );
	WorldDef.m_HiColorMin.x = ConfigManager::GetFloat( sHiColorH, 0.0f, sLightDefinitionName );

	STATICHASH( HiColorHRange );
	WorldDef.m_HiColorMax.x = WorldDef.m_HiColorMin.x + ConfigManager::GetFloat( sHiColorHRange, 0.0f, sLightDefinitionName );

	STATICHASH( HiColorS );
	WorldDef.m_HiColorMin.y = ConfigManager::GetFloat( sHiColorS, 0.0f, sLightDefinitionName );

	STATICHASH( HiColorSRange );
	WorldDef.m_HiColorMax.y = WorldDef.m_HiColorMin.y + ConfigManager::GetFloat( sHiColorSRange, 0.0f, sLightDefinitionName );

	STATICHASH( HiColorV );
	WorldDef.m_HiColorMin.z = ConfigManager::GetFloat( sHiColorV, 0.0f, sLightDefinitionName );

	STATICHASH( HiColorVRange );
	WorldDef.m_HiColorMax.z = WorldDef.m_HiColorMin.z + ConfigManager::GetFloat( sHiColorVRange, 0.0f, sLightDefinitionName );

	Vector LoColorMin;
	Vector LoColorMax;

	STATICHASH( LoColorH );
	LoColorMin.x = ConfigManager::GetFloat( sLoColorH, WorldDef.m_HiColorMin.x, sLightDefinitionName );

	STATICHASH( LoColorHRange );
	LoColorMax.x = LoColorMin.x + ConfigManager::GetFloat( sLoColorHRange, WorldDef.m_HiColorMax.x - WorldDef.m_HiColorMin.x, sLightDefinitionName );

	STATICHASH( LoColorS );
	LoColorMin.y = ConfigManager::GetFloat( sLoColorS, WorldDef.m_HiColorMin.y, sLightDefinitionName );

	STATICHASH( LoColorSRange );
	LoColorMax.y = LoColorMin.y + ConfigManager::GetFloat( sLoColorSRange, WorldDef.m_HiColorMax.y - WorldDef.m_HiColorMin.y, sLightDefinitionName );

	STATICHASH( LoColorV );
	LoColorMin.z = ConfigManager::GetFloat( sLoColorV, WorldDef.m_HiColorMin.z, sLightDefinitionName );

	STATICHASH( LoColorVRange );
	LoColorMax.z = LoColorMin.z + ConfigManager::GetFloat( sLoColorVRange, WorldDef.m_HiColorMax.z - WorldDef.m_HiColorMin.z, sLightDefinitionName );

	WorldDef.m_LoColorDeltaMin = LoColorMin - WorldDef.m_HiColorMin;
	WorldDef.m_LoColorDeltaMax = LoColorMax - WorldDef.m_HiColorMax;
	Vector::MinMax( WorldDef.m_LoColorDeltaMin, WorldDef.m_LoColorDeltaMax );

	STATICHASH( LoColorDeltaH );
	WorldDef.m_LoColorDeltaMin.x = ConfigManager::GetFloat( sLoColorDeltaH, WorldDef.m_LoColorDeltaMin.x, sLightDefinitionName );

	STATICHASH( LoColorDeltaHRange );
	WorldDef.m_LoColorDeltaMax.x = WorldDef.m_LoColorDeltaMin.x + ConfigManager::GetFloat( sLoColorDeltaHRange, WorldDef.m_LoColorDeltaMax.x - WorldDef.m_LoColorDeltaMin.x, sLightDefinitionName );

	STATICHASH( LoColorDeltaS );
	WorldDef.m_LoColorDeltaMin.y = ConfigManager::GetFloat( sLoColorDeltaS, WorldDef.m_LoColorDeltaMin.y, sLightDefinitionName );

	STATICHASH( LoColorDeltaSRange );
	WorldDef.m_LoColorDeltaMax.y = WorldDef.m_LoColorDeltaMin.y + ConfigManager::GetFloat( sLoColorDeltaSRange, WorldDef.m_LoColorDeltaMax.y - WorldDef.m_LoColorDeltaMin.y, sLightDefinitionName );

	STATICHASH( LoColorDeltaV );
	WorldDef.m_LoColorDeltaMin.z = ConfigManager::GetFloat( sLoColorDeltaV, WorldDef.m_LoColorDeltaMin.z, sLightDefinitionName );

	STATICHASH( LoColorDeltaVRange );
	WorldDef.m_LoColorDeltaMax.z = WorldDef.m_LoColorDeltaMin.z + ConfigManager::GetFloat( sLoColorDeltaVRange, WorldDef.m_LoColorDeltaMax.z - WorldDef.m_LoColorDeltaMin.z, sLightDefinitionName );

	static const float kMidColorAlphaX = 1.0f / 3.0f;

	STATICHASH( MidColorAlphaXH );
	WorldDef.m_MidColorAlphaX.x = ConfigManager::GetFloat( sMidColorAlphaXH, kMidColorAlphaX, sLightDefinitionName );

	STATICHASH( MidColorAlphaXS );
	WorldDef.m_MidColorAlphaX.y = ConfigManager::GetFloat( sMidColorAlphaXS, kMidColorAlphaX, sLightDefinitionName );

	STATICHASH( MidColorAlphaXV );
	WorldDef.m_MidColorAlphaX.z = ConfigManager::GetFloat( sMidColorAlphaXV, kMidColorAlphaX, sLightDefinitionName );

	static const float kMidColorAlphaY = 2.0f / 3.0f;

	STATICHASH( MidColorAlphaYH );
	WorldDef.m_MidColorAlphaY.x = ConfigManager::GetFloat( sMidColorAlphaYH, kMidColorAlphaY, sLightDefinitionName );

	STATICHASH( MidColorAlphaYS );
	WorldDef.m_MidColorAlphaY.y = ConfigManager::GetFloat( sMidColorAlphaYS, kMidColorAlphaY, sLightDefinitionName );

	STATICHASH( MidColorAlphaYV );
	WorldDef.m_MidColorAlphaY.z = ConfigManager::GetFloat( sMidColorAlphaYV, kMidColorAlphaY, sLightDefinitionName );

	STATICHASH( AOColorV );
	WorldDef.m_AOColorVMin = ConfigManager::GetFloat( sAOColorV, 0.0f, sLightDefinitionName );

	STATICHASH( AOColorVRange );
	WorldDef.m_AOColorVMax = WorldDef.m_AOColorVMin + ConfigManager::GetFloat( sAOColorVRange, 0.0f, sLightDefinitionName );
}

void EldritchWorld::InitializeFogConfig( const SimpleString& FogDefinitionName, SWorldDef& WorldDef )
{
	MAKEHASH( FogDefinitionName );

	STATICHASH( Near );
	WorldDef.m_FogNear = ConfigManager::GetFloat( sNear, 0.0f, sFogDefinitionName );

	STATICHASH( Far );
	WorldDef.m_FogFar = ConfigManager::GetFloat( sFar, 0.0f, sFogDefinitionName );

	STATICHASH( Texture );
	WorldDef.m_FogTexture = ConfigManager::GetString( sTexture, "", sFogDefinitionName );
}

void EldritchWorld::Initialize()
{
	m_Framework = EldritchFramework::GetInstance();

	const SimpleString DefinitionName( "EldritchWorld" );
	InitializeConfig( DefinitionName );

	m_Nav = new EldritchNav;

	m_TileUVsMap.Reserve( m_NumTiles );
	for( uint TileIndex = 0; TileIndex < m_NumTiles; ++TileIndex )
	{
		const float		TileU	= ( TileIndex % m_NumTilesX ) * m_TileU;
		const float		TileV	= ( TileIndex / m_NumTilesY ) * m_TileV;
		const Vector2	TileUV	= Vector2( TileU, TileV );
		m_TileUVsMap.PushBack( TileUV );
	}

	InitializeVoxelPacking();

	m_WorldMeshes.Resize( m_NumMeshes );
	m_WorldMeshes.MemoryZero();

	m_VoxelMap.Resize( m_NumVoxels );

	m_IrradianceMap.Resize( m_NumVoxels );
	m_IrradianceMap.MemoryZero();

	m_LightInfluenceMap.Resize( m_NumVoxels );
	m_LightInfluenceMap.MemoryZero();			// This could have side effects on inner maps.

	PRINTF( "Voxel mem.: %d Irradiance mem.: %d\n", m_VoxelMap.MemorySize(), m_IrradianceMap.MemorySize() );
}

void EldritchWorld::Create()
{
	PROFILE_FUNCTION;

	ASSERT( m_CurrentWorldDef != HashedString::NullString );

	EldritchGame* const	pGame			= EldritchFramework::GetInstance()->GetGame();
	const SWorldDef&	CurrentWorldDef	= GetWorldDef();
	pGame->SetColorGradingTexture( CurrentWorldDef.m_ColorGrading );
	pGame->SetFogParams( CurrentWorldDef.m_FogNear, CurrentWorldDef.m_FogFar, CurrentWorldDef.m_FogTexture );
	pGame->SetCurrentMusic( CurrentWorldDef.m_Music );

	m_DisableCompute = true;
	CreateImpenetrableOuterWalls();
	RollLightColors();
	GetWorldDef().m_WorldGen->Generate();
	m_DisableCompute = false;

	AddAllLightInfluences();
	ComputeAllIrradiance();
	BuildAllMeshes();
	m_Nav->UpdateWorldFromAllVoxels();
}

void EldritchWorld::RollLightColors()
{
	const SWorldDef&	WorldDef		= GetWorldDef();
	const Vector		HiColor			= Math::Random( WorldDef.m_HiColorMin,		WorldDef.m_HiColorMax );
	const Vector		LoColorDelta	= Math::Random( WorldDef.m_LoColorDeltaMin,	WorldDef.m_LoColorDeltaMax );
	const Vector		LoColor			= HiColor + LoColorDelta;
	const float			AOColorV		= Math::Random( WorldDef.m_AOColorVMin,		WorldDef.m_AOColorVMax );
	const Vector		XColor			= HiColor.LERP( WorldDef.m_MidColorAlphaX, LoColor );
	const Vector		YColor			= HiColor.LERP( WorldDef.m_MidColorAlphaY, LoColor );
	const Vector		AOColor			= Vector( LoColor.x, LoColor.y, AOColorV );

	m_GlobalLight.m_Light[ IRRDIR_Up ]		= HSV::HSVToRGB( HiColor );
	m_GlobalLight.m_Light[ IRRDIR_Down ]	= HSV::HSVToRGB( LoColor );
	m_GlobalLight.m_Light[ IRRDIR_Left ]	= HSV::HSVToRGB( XColor );
	m_GlobalLight.m_Light[ IRRDIR_Right ]	= m_GlobalLight.m_Light[ IRRDIR_Left ];
	m_GlobalLight.m_Light[ IRRDIR_Back ]	= HSV::HSVToRGB( YColor );
	m_GlobalLight.m_Light[ IRRDIR_Front ]	= m_GlobalLight.m_Light[ IRRDIR_Back ];
	m_AOColor								= HSV::HSVToRGB( AOColor );
}

void EldritchWorld::CreateImpenetrableOuterWalls()
{
	static const vval_t kWallValue = 1;

	const vpos_t HiX = m_NumVoxelsX - 1;
	const vpos_t HiY = m_NumVoxelsY - 1;
	const vpos_t HiZ = m_NumVoxelsZ - 1;

	for( vpos_t Y = 0; Y < m_NumVoxelsY; ++Y )
	{
		for( vpos_t X = 0; X < m_NumVoxelsX; ++X )
		{
			SetVoxel( GetIndex( X, Y, 0 ),		kWallValue | kImpenetrableVoxelFlag );
			SetVoxel( GetIndex( X, Y, HiZ ),	kWallValue | kImpenetrableVoxelFlag );
		}
	}

	for( vpos_t Z = 0; Z < m_NumVoxelsZ; ++Z )
	{
		for( vpos_t X = 0; X < m_NumVoxelsX; ++X )
		{
			SetVoxel( GetIndex( X, 0, Z ),		kWallValue | kImpenetrableVoxelFlag );
			SetVoxel( GetIndex( X, HiY, Z ),	kWallValue | kImpenetrableVoxelFlag );
		}
	}

	for( vpos_t Z = 0; Z < m_NumVoxelsZ; ++Z )
	{
		for( vpos_t Y = 0; Y < m_NumVoxelsY; ++Y )
		{
			SetVoxel( GetIndex( 0, Y, Z ),		kWallValue | kImpenetrableVoxelFlag );
			SetVoxel( GetIndex( HiX, Y, Z ),	kWallValue | kImpenetrableVoxelFlag );
		}
	}
}

void EldritchWorld::GetMeshCoords( const uint MeshIndex, uint& OutX, uint& OutY, uint& OutZ ) const
{
	OutX = MeshIndex % m_NumMeshesX;
	OutY = ( MeshIndex / m_NumMeshesX ) % m_NumMeshesY;
	OutZ = ( ( MeshIndex / m_NumMeshesX ) / m_NumMeshesY ) % m_NumMeshesZ;
}

uint EldritchWorld::GetMeshIndex( const uint MeshX, const uint MeshY, const uint MeshZ ) const
{
	return MeshX + ( MeshY * m_NumMeshesX ) + ( MeshZ * m_NumMeshesX * m_NumMeshesY );
}

void EldritchWorld::BuildMeshesForChangedVoxels( const Set<vidx_t>& ChangedVoxels )
{
	PROFILE_FUNCTION;

	if( m_DisableCompute )
	{
		return;
	}

	Set<uint> MeshesToBuild;

	FOR_EACH_SET( VoxelIter, ChangedVoxels, vidx_t )
	{
		vidx_t ChangedVoxel = VoxelIter.GetValue();

		if( !IsValidIndex( ChangedVoxel ) )
		{
			WARNDESC( "Invalid index in BuildMeshesForChangedVoxels" );
			continue;
		}

		vpos_t VoxelX, VoxelY, VoxelZ;
		GetCoords( ChangedVoxel, VoxelX, VoxelY, VoxelZ );

		const uint MeshX		= VoxelX / m_MeshSizeX;
		const uint MeshY		= VoxelY / m_MeshSizeY;
		const uint MeshZ		= VoxelZ / m_MeshSizeZ;
		const uint LocalMeshX	= VoxelX % m_MeshSizeX;
		const uint LocalMeshY	= VoxelY % m_MeshSizeY;
		const uint LocalMeshZ	= VoxelZ % m_MeshSizeZ;

		const uint MeshIndex = GetMeshIndex( MeshX, MeshY, MeshZ );

		MeshesToBuild.Insert( MeshIndex );

		// If the voxel is on the edge of a mesh, gather adjacent meshes which may need updated faces or AO.
		uint AddAdjacentDirections = EDirectionNone;
		if( LocalMeshX == 0					&& MeshX > 0				) AddAdjacentDirections |= EDirectionLeft;
		if( LocalMeshX == m_MeshSizeX - 1	&& MeshX < m_NumMeshesX - 1	) AddAdjacentDirections |= EDirectionRight;
		if( LocalMeshY == 0					&& MeshY > 0				) AddAdjacentDirections |= EDirectionBack;
		if( LocalMeshY == m_MeshSizeY - 1	&& MeshY < m_NumMeshesY - 1	) AddAdjacentDirections |= EDirectionFront;
		if( LocalMeshZ == 0					&& MeshZ > 0				) AddAdjacentDirections |= EDirectionDown;
		if( LocalMeshZ == m_MeshSizeZ - 1	&& MeshZ < m_NumMeshesZ - 1	) AddAdjacentDirections |= EDirectionUp;

		const uint dX = 1;
		const uint dY = m_NumMeshesX;
		const uint dZ = m_NumMeshesX * m_NumMeshesY;

#define TEST_DIR_FLAGS( dirs ) ( ( AddAdjacentDirections & ( dirs ) ) == ( dirs ) )
		// Faces
		if( TEST_DIR_FLAGS( EDirectionLeft ) )	MeshesToBuild.Insert( MeshIndex - dX );
		if( TEST_DIR_FLAGS( EDirectionRight ) )	MeshesToBuild.Insert( MeshIndex + dX );
		if( TEST_DIR_FLAGS( EDirectionBack ) )	MeshesToBuild.Insert( MeshIndex - dY );
		if( TEST_DIR_FLAGS( EDirectionFront ) )	MeshesToBuild.Insert( MeshIndex + dY );
		if( TEST_DIR_FLAGS( EDirectionDown ) )	MeshesToBuild.Insert( MeshIndex - dZ );
		if( TEST_DIR_FLAGS( EDirectionUp ) )	MeshesToBuild.Insert( MeshIndex + dZ );

		// Edges
		if( TEST_DIR_FLAGS( EDirectionLBC ) )	MeshesToBuild.Insert( MeshIndex - dX - dY );
		if( TEST_DIR_FLAGS( EDirectionLFC ) )	MeshesToBuild.Insert( MeshIndex - dX + dY );
		if( TEST_DIR_FLAGS( EDirectionLCD ) )	MeshesToBuild.Insert( MeshIndex - dX - dZ );
		if( TEST_DIR_FLAGS( EDirectionLCU ) )	MeshesToBuild.Insert( MeshIndex - dX + dZ );
		if( TEST_DIR_FLAGS( EDirectionRBC ) )	MeshesToBuild.Insert( MeshIndex + dX - dY );
		if( TEST_DIR_FLAGS( EDirectionRFC ) )	MeshesToBuild.Insert( MeshIndex + dX + dY );
		if( TEST_DIR_FLAGS( EDirectionRCD ) )	MeshesToBuild.Insert( MeshIndex + dX - dZ );
		if( TEST_DIR_FLAGS( EDirectionRCU ) )	MeshesToBuild.Insert( MeshIndex + dX + dZ );
		if( TEST_DIR_FLAGS( EDirectionCBD ) )	MeshesToBuild.Insert( MeshIndex - dY - dZ );
		if( TEST_DIR_FLAGS( EDirectionCBU ) )	MeshesToBuild.Insert( MeshIndex - dY + dZ );
		if( TEST_DIR_FLAGS( EDirectionCFD ) )	MeshesToBuild.Insert( MeshIndex + dY - dZ );
		if( TEST_DIR_FLAGS( EDirectionCFU ) )	MeshesToBuild.Insert( MeshIndex + dY + dZ );

		// Vertices
		if( TEST_DIR_FLAGS( EDirectionLBD ) )	MeshesToBuild.Insert( MeshIndex - dX - dY - dZ );
		if( TEST_DIR_FLAGS( EDirectionLBU ) )	MeshesToBuild.Insert( MeshIndex - dX - dY + dZ );
		if( TEST_DIR_FLAGS( EDirectionLFD ) )	MeshesToBuild.Insert( MeshIndex - dX + dY - dZ );
		if( TEST_DIR_FLAGS( EDirectionLFU ) )	MeshesToBuild.Insert( MeshIndex - dX + dY + dZ );
		if( TEST_DIR_FLAGS( EDirectionRBD ) )	MeshesToBuild.Insert( MeshIndex + dX - dY - dZ );
		if( TEST_DIR_FLAGS( EDirectionRBU ) )	MeshesToBuild.Insert( MeshIndex + dX - dY + dZ );
		if( TEST_DIR_FLAGS( EDirectionRFD ) )	MeshesToBuild.Insert( MeshIndex + dX + dY - dZ );
		if( TEST_DIR_FLAGS( EDirectionRFU ) )	MeshesToBuild.Insert( MeshIndex + dX + dY + dZ );
#undef TEST_DIR_FLAGS
	}

	FOR_EACH_SET( MeshIter, MeshesToBuild, uint )
	{
		BuildMesh( MeshIter.GetValue() );
	}

#if BUILD_DEV
	if( MeshesToBuild.Size() )
	{
		PRINTF( "BuildMeshesForChangedVoxels built %d meshes.\n", MeshesToBuild.Size() );
	}
#endif
}

// Build meshes around voxels
void EldritchWorld::BuildAllMeshes()
{
	PROFILE_FUNCTION;

	if( m_DisableCompute )
	{
		return;
	}

	for( uint MeshIndex = 0; MeshIndex < m_NumMeshes; ++MeshIndex )
	{
		BuildMesh( MeshIndex );
	}

#if BUILD_DEV
	if( m_NumMeshes )
	{
		PRINTF( "BuildAllMeshes built %d meshes.\n", m_NumMeshes );
	}
#endif
}

// MeshIndex is in the 1D array representing a 3D array of meshes ordered XYZ
void EldritchWorld::BuildMesh( const uint MeshIndex )
{
	PROFILE_FUNCTION;

	if( m_DisableCompute )
	{
		return;
	}

	// Destroy the current mesh, if any.
	SafeDelete( m_WorldMeshes[ MeshIndex ] );

	ASSERT( MeshIndex < m_NumMeshes );
	ASSERT( MeshIndex <  m_WorldMeshes.Size() );

	uint MeshX, MeshY, MeshZ;
	GetMeshCoords( MeshIndex, MeshX, MeshY, MeshZ );

	const uint BaseVoxelX = MeshX * m_MeshSizeX;
	const uint BaseVoxelY = MeshY * m_MeshSizeY;
	const uint BaseVoxelZ = MeshZ * m_MeshSizeZ;

	const uint EndVoxelX = BaseVoxelX + m_MeshSizeX;
	const uint EndVoxelY = BaseVoxelY + m_MeshSizeY;
	const uint EndVoxelZ = BaseVoxelZ + m_MeshSizeZ;

	SVoxelMeshBuffers Buffers;

	Buffers.Positions.Reserve( m_MaxVertices );
	Buffers.UVs.Reserve( m_MaxVertices );
	Buffers.Colors.Reserve( m_MaxVertices );
	Buffers.Indices.Reserve( m_MaxIndices );

	for( uint Z = BaseVoxelZ; Z < EndVoxelZ; ++Z )
	{
		for( uint Y = BaseVoxelY; Y < EndVoxelY; ++Y )
		{
			for( uint X = BaseVoxelX; X < EndVoxelX; ++X )
			{
				BuildVertsForVoxel( X, Y, Z, Buffers );
			}
		}
	}

	if( Buffers.Positions.Size() == 0 )
	{
		// No voxels to draw in this mesh space.
		ASSERT( m_WorldMeshes[ MeshIndex ] == NULL );
		return;
	}

	ASSERT( Buffers.Positions.Size() == Buffers.Colors.Size() );
	ASSERT( Buffers.Positions.Size() == Buffers.UVs.Size() );

	IVertexBuffer*			VertexBuffer		= m_Framework->GetRenderer()->CreateVertexBuffer();
	IVertexDeclaration*		VertexDeclaration	= m_Framework->GetRenderer()->GetVertexDeclaration( VD_POSITIONS | VD_FLOATCOLORS_SM2 | VD_UVS );
	IIndexBuffer*			IndexBuffer			= m_Framework->GetRenderer()->CreateIndexBuffer();

	IVertexBuffer::SInit	InitStruct;
	InitStruct.NumVertices	= Buffers.Positions.Size();
	InitStruct.Positions	= Buffers.Positions.GetData();
	InitStruct.FloatColors1	= Buffers.Colors.GetData();
	InitStruct.UVs			= Buffers.UVs.GetData();

	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( Buffers.Indices.Size(), Buffers.Indices.GetData() );

	const Vector AABBMin( static_cast<float>( BaseVoxelX ), static_cast<float>( BaseVoxelY ), static_cast<float>( BaseVoxelZ ) );
	const Vector AABBMax( static_cast<float>( EndVoxelX ), static_cast<float>( EndVoxelY ), static_cast<float>( EndVoxelZ ) );

	Mesh* const NewWorldMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );
	NewWorldMesh->m_AABB = AABB( AABBMin, AABBMax );
#if BUILD_DEV
	if( m_TestLight )
	{
		NewWorldMesh->SetTexture( 0, m_Framework->GetRenderer()->GetTextureManager()->GetTexture( DEFAULT_TEXTURE ) );
	}
	else
#endif
	{
		const SimpleString& WorldTileset = GetWorldDef().m_Tileset;
		NewWorldMesh->SetTexture( 0, m_Framework->GetRenderer()->GetTextureManager()->GetTexture( WorldTileset.CStr() ) );
	}
	NewWorldMesh->SetTexture( 1, m_Framework->GetGame()->GetFogTexture() );
	NewWorldMesh->SetMaterialDefinition( "Material_World", m_Framework->GetRenderer() );
	NewWorldMesh->SetMaterialFlags( MAT_WORLD );

	m_WorldMeshes[ MeshIndex ] = NewWorldMesh;
}

// NOTE: Intentionally never shuffling the noise hasher because I don't want to serialize it for each world.
Vector EldritchWorld::ApplyDisplacementNoise( const Vector& Location, const SWorldDef& WorldDef ) const
{
	if( WorldDef.m_DisplacementNoiseOctaves == 0 )
	{
		return Location;
	}

	const Vector ScaledVector = Location * WorldDef.m_DisplacementNoiseLookupScale;

	// Use transformed locations for lookups so each component has a different value.
	const float NoiseX = Noise::SumNoise3( ScaledVector.x, ScaledVector.y, ScaledVector.z, WorldDef.m_DisplacementNoiseOctaves, Noise::CubicNoise3 );
	const float NoiseY = Noise::SumNoise3( ScaledVector.y, ScaledVector.z, ScaledVector.x, WorldDef.m_DisplacementNoiseOctaves, Noise::CubicNoise3 );
	const float NoiseZ = Noise::SumNoise3( ScaledVector.z, ScaledVector.x, ScaledVector.y, WorldDef.m_DisplacementNoiseOctaves, Noise::CubicNoise3 );
	const Vector NoiseVector = Vector( NoiseX, NoiseY, NoiseZ );

	return Location + WorldDef.m_DisplacementNoiseScale * NoiseVector;
}

void EldritchWorld::BuildVertsForVoxel( const uint X, const uint Y, const uint Z, SVoxelMeshBuffers& Buffers ) const
{
	// This gets called enough that the profiler hook is a cost. >_<
	//PROFILE_FUNCTION;

	const vidx_t VoxelIndex = GetIndex( X, Y, Z );
	if( VoxelIndex == kInvalidVoxelIndex )
	{
		return;
	}

	const vval_t Voxel = GetVoxel( VoxelIndex );
	ASSERT( Voxel != kInvalidVoxelValue );

	if( Voxel > kEmptyVoxelValue )
	{
		const EldritchWorld::SVoxelDef& VoxelDef = GetVoxelDef( Voxel );

		// This is a solid voxel, it might have exposed faces
		const vxnb_t Neighbors = GetExtendedNeighbors( VoxelIndex );

		// For ease, voxels are hard-coded to 1m dimensions.
		const float fX1 = static_cast<float>( X );
		const float fXh = fX1 + 0.5f;
		const float fX2 = fX1 + 1.0f;
		const float fY1 = static_cast<float>( Y );
		const float fYh = fY1 + 0.5f;
		const float fY2 = fY1 + 1.0f;
		const float fZ1 = static_cast<float>( Z );
		const float fZh = fZ1 + 0.5f;
		const float fZ2 = fZ1 + 1.0f;

		// Verts:
		//   6---7
		//  /|  /|
		// 4-2-5-3
		// |/  |/
		// 0---1

#if DO_VOXEL_NOISE_DISPLACEMENT
#define APPLY_DISPLACEMENT( x, y )	ApplyDisplacementNoise( x, y )
#else
#define APPLY_DISPLACEMENT( x, y )	x
#endif

#if DO_VOXEL_MIDPOINT_NOISE_DISPLACEMENT || DO_VOXEL_NOISE_DISPLACEMENT
#define APPLY_MIDPOINT_DISPLACEMENT( x, y )	ApplyDisplacementNoise( x, y )
#else
#define APPLY_MIDPOINT_DISPLACEMENT( x, y )	x
#endif

#if DO_VOXEL_MIDPOINT_FIXED_DISPLACEMENT
#define ADD_MIDPOINT_FIXED_DISPLACEMENT( x )	+ x
#else
#define ADD_MIDPOINT_FIXED_DISPLACEMENT( x )
#endif

		const SWorldDef& CurrentWorldDef = GetWorldDef();
		Unused( CurrentWorldDef );

		const Vector V0		= APPLY_DISPLACEMENT( Vector( fX1, fY1, fZ1 ), CurrentWorldDef );
		const Vector V1		= APPLY_DISPLACEMENT( Vector( fX2, fY1, fZ1 ), CurrentWorldDef );
		const Vector V2		= APPLY_DISPLACEMENT( Vector( fX1, fY2, fZ1 ), CurrentWorldDef );
		const Vector V3		= APPLY_DISPLACEMENT( Vector( fX2, fY2, fZ1 ), CurrentWorldDef );
		const Vector V4		= APPLY_DISPLACEMENT( Vector( fX1, fY1, fZ2 ), CurrentWorldDef );
		const Vector V5		= APPLY_DISPLACEMENT( Vector( fX2, fY1, fZ2 ), CurrentWorldDef );
		const Vector V6		= APPLY_DISPLACEMENT( Vector( fX1, fY2, fZ2 ), CurrentWorldDef );
		const Vector V7		= APPLY_DISPLACEMENT( Vector( fX2, fY2, fZ2 ), CurrentWorldDef );
		const Vector VXp	= APPLY_MIDPOINT_DISPLACEMENT( Vector( fX2, fYh, fZh ), CurrentWorldDef ) ADD_MIDPOINT_FIXED_DISPLACEMENT( Vector( CurrentWorldDef.m_DisplacementNoiseScale.x, 0.0f, 0.0f ) );
		const Vector VXn	= APPLY_MIDPOINT_DISPLACEMENT( Vector( fX1, fYh, fZh ), CurrentWorldDef ) ADD_MIDPOINT_FIXED_DISPLACEMENT( Vector( -CurrentWorldDef.m_DisplacementNoiseScale.x, 0.0f, 0.0f ) );
		const Vector VYp	= APPLY_MIDPOINT_DISPLACEMENT( Vector( fXh, fY2, fZh ), CurrentWorldDef ) ADD_MIDPOINT_FIXED_DISPLACEMENT( Vector( 0.0f, CurrentWorldDef.m_DisplacementNoiseScale.y, 0.0f ) );
		const Vector VYn	= APPLY_MIDPOINT_DISPLACEMENT( Vector( fXh, fY1, fZh ), CurrentWorldDef ) ADD_MIDPOINT_FIXED_DISPLACEMENT( Vector( 0.0f, -CurrentWorldDef.m_DisplacementNoiseScale.y, 0.0f ) );
		const Vector VZp	= APPLY_MIDPOINT_DISPLACEMENT( Vector( fXh, fYh, fZ2 ), CurrentWorldDef ) ADD_MIDPOINT_FIXED_DISPLACEMENT( Vector( 0.0f, 0.0f, CurrentWorldDef.m_DisplacementNoiseScale.z ) );
		const Vector VZn	= APPLY_MIDPOINT_DISPLACEMENT( Vector( fXh, fYh, fZ1 ), CurrentWorldDef ) ADD_MIDPOINT_FIXED_DISPLACEMENT( Vector( 0.0f, 0.0f, -CurrentWorldDef.m_DisplacementNoiseScale.z ) );

		float AO[4];

		index_t VertIndex = static_cast<index_t>( Buffers.Positions.Size() );

		if( !HasAnyNeighbors( Neighbors, ENeighborRight ) )
		{
			Buffers.Positions.PushBack( V1 );
			Buffers.Positions.PushBack( V3 );
			Buffers.Positions.PushBack( V5 );
			Buffers.Positions.PushBack( V7 );
			Buffers.Positions.PushBack( VXp );

			AO[0] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborRBC | ENeighborRBD | ENeighborRCD ) );
			AO[1] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborRFC | ENeighborRFD | ENeighborRCD ) );
			AO[2] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborRBC | ENeighborRBU | ENeighborRCU ) );
			AO[3] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborRFC | ENeighborRFU | ENeighborRCU ) );

			const SVoxelIrradiance& Irradiance = GetIrradiance( GetIndexNeighborRight( VoxelIndex ) );
			const Vector4 Color = m_GlobalLight.m_Light[ IRRDIR_Right ] + Irradiance.m_Light[ IRRDIR_Right ];

			PushColors( Color, m_AOColor, AO, Buffers );
			PushUVs( GetTileUV( VoxelDef.m_SideTile ), false, true, Buffers );
			PushIndices( VertIndex, true, Buffers );
		}

		if( !HasAnyNeighbors( Neighbors, ENeighborLeft ) )
		{
			Buffers.Positions.PushBack( V0 );
			Buffers.Positions.PushBack( V2 );
			Buffers.Positions.PushBack( V4 );
			Buffers.Positions.PushBack( V6 );
			Buffers.Positions.PushBack( VXn );

			AO[0] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborLBC | ENeighborLBD | ENeighborLCD ) );
			AO[1] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborLFC | ENeighborLFD | ENeighborLCD ) );
			AO[2] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborLBC | ENeighborLBU | ENeighborLCU ) );
			AO[3] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborLFC | ENeighborLFU | ENeighborLCU ) );

			const SVoxelIrradiance& Irradiance = GetIrradiance( GetIndexNeighborLeft( VoxelIndex ) );
			const Vector4 Color = m_GlobalLight.m_Light[ IRRDIR_Left ] + Irradiance.m_Light[ IRRDIR_Left ];

			PushColors( Color, m_AOColor, AO, Buffers );
			PushUVs( GetTileUV( VoxelDef.m_SideTile ), true, true, Buffers );
			PushIndices( VertIndex, false, Buffers );
		}

		if( !HasAnyNeighbors( Neighbors, ENeighborFront ) )
		{
			Buffers.Positions.PushBack( V2 );
			Buffers.Positions.PushBack( V3 );
			Buffers.Positions.PushBack( V6 );
			Buffers.Positions.PushBack( V7 );
			Buffers.Positions.PushBack( VYp );

			AO[0] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborLFC | ENeighborLFD | ENeighborCFD ) );
			AO[1] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborRFC | ENeighborRFD | ENeighborCFD ) );
			AO[2] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborLFC | ENeighborLFU | ENeighborCFU ) );
			AO[3] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborRFC | ENeighborRFU | ENeighborCFU ) );

			const SVoxelIrradiance& Irradiance = GetIrradiance( GetIndexNeighborFront( VoxelIndex ) );
			const Vector4 Color = m_GlobalLight.m_Light[ IRRDIR_Front ] + Irradiance.m_Light[ IRRDIR_Front ];

			PushColors( Color, m_AOColor, AO, Buffers );
			PushUVs( GetTileUV( VoxelDef.m_SideTile ), true, true, Buffers );
			PushIndices( VertIndex, false, Buffers );
		}

		if( !HasAnyNeighbors( Neighbors, ENeighborBack ) )
		{
			Buffers.Positions.PushBack( V0 );
			Buffers.Positions.PushBack( V1 );
			Buffers.Positions.PushBack( V4 );
			Buffers.Positions.PushBack( V5 );
			Buffers.Positions.PushBack( VYn );

			AO[0] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborLBC | ENeighborLBD | ENeighborCBD ) );
			AO[1] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborRBC | ENeighborRBD | ENeighborCBD ) );
			AO[2] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborLBC | ENeighborLBU | ENeighborCBU ) );
			AO[3] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborRBC | ENeighborRBU | ENeighborCBU ) );

			const SVoxelIrradiance& Irradiance = GetIrradiance( GetIndexNeighborBack( VoxelIndex ) );
			const Vector4 Color = m_GlobalLight.m_Light[ IRRDIR_Back ] + Irradiance.m_Light[ IRRDIR_Back ];

			PushColors( Color, m_AOColor, AO, Buffers );
			PushUVs( GetTileUV( VoxelDef.m_SideTile ), false, true, Buffers );
			PushIndices( VertIndex, true, Buffers );
		}

		// Top surface
		if( !HasAnyNeighbors( Neighbors, ENeighborUp ) )
		{
			Buffers.Positions.PushBack( V4 );
			Buffers.Positions.PushBack( V5 );
			Buffers.Positions.PushBack( V6 );
			Buffers.Positions.PushBack( V7 );
			Buffers.Positions.PushBack( VZp );

			AO[0] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborLCU | ENeighborLBU | ENeighborCBU ) );
			AO[1] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborRCU | ENeighborRBU | ENeighborCBU ) );
			AO[2] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborLCU | ENeighborLFU | ENeighborCFU ) );
			AO[3] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborRCU | ENeighborRFU | ENeighborCFU ) );

			const SVoxelIrradiance& Irradiance = GetIrradiance( GetIndexNeighborUp( VoxelIndex ) );
			const Vector4 Color = m_GlobalLight.m_Light[ IRRDIR_Up ] + Irradiance.m_Light[ IRRDIR_Up ];

			PushColors( Color, m_AOColor, AO, Buffers );
			PushUVs( GetTileUV( VoxelDef.m_TopTile ), false, true, Buffers );
			PushIndices( VertIndex, true, Buffers );
		}

		// Bottom surface
		if( !HasAnyNeighbors( Neighbors, ENeighborDown ) )
		{
			Buffers.Positions.PushBack( V0 );
			Buffers.Positions.PushBack( V1 );
			Buffers.Positions.PushBack( V2 );
			Buffers.Positions.PushBack( V3 );
			Buffers.Positions.PushBack( VZn );

			AO[0] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborLCD | ENeighborLBD | ENeighborCBD ) );
			AO[1] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborRCD | ENeighborRBD | ENeighborCBD ) );
			AO[2] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborLCD | ENeighborLFD | ENeighborCFD ) );
			AO[3] = GetAOForNeighbors( CountNeighbors( Neighbors, ENeighborRCD | ENeighborRFD | ENeighborCFD ) );

			const SVoxelIrradiance& Irradiance = GetIrradiance( GetIndexNeighborDown( VoxelIndex ) );
			const Vector4 Color = m_GlobalLight.m_Light[ IRRDIR_Down ] + Irradiance.m_Light[ IRRDIR_Down ];

			PushColors( Color, m_AOColor, AO, Buffers );
			PushUVs( GetTileUV( VoxelDef.m_BottomTile ), false, false, Buffers );
			PushIndices( VertIndex, false, Buffers );
		}
	}
}

inline void EldritchWorld::PushColors( const Vector4& Color, const Vector4& AOColor, const float AOValues[], SVoxelMeshBuffers& Buffers ) const
{
	const float AverageAO = 0.25f * ( AOValues[0] + AOValues[1] + AOValues[2] + AOValues[3] );

	const Vector4 Color0 = AOColor.LERP( AOValues[0], Color );
	const Vector4 Color1 = AOColor.LERP( AOValues[1], Color );
	const Vector4 Color2 = AOColor.LERP( AOValues[2], Color );
	const Vector4 Color3 = AOColor.LERP( AOValues[3], Color );
	const Vector4 Average = AOColor.LERP( AverageAO, Color );

	Buffers.Colors.PushBack( Color0 );
	Buffers.Colors.PushBack( Color1 );
	Buffers.Colors.PushBack( Color2 );
	Buffers.Colors.PushBack( Color3 );
	Buffers.Colors.PushBack( Average );
}

void EldritchWorld::PushUVs( const Vector2& BaseUV, const bool FlipU, const bool FlipV, struct SVoxelMeshBuffers& Buffers ) const
{
	float U1 = BaseUV.uv_u + m_TileUAdjustment;
	float U2 = BaseUV.uv_u + m_TileU - m_TileUAdjustment;
	float V1 = BaseUV.uv_v + m_TileVAdjustment;
	float V2 = BaseUV.uv_v + m_TileV - m_TileVAdjustment;
	const float Uh = BaseUV.uv_u + m_TileUh;
	const float Vh = BaseUV.uv_v + m_TileVh;

	if( FlipU ) { Swap( U1, U2 ); };
	if( FlipV ) { Swap( V1, V2 ); };

	Buffers.UVs.PushBack( Vector2( U1, V1 ) );
	Buffers.UVs.PushBack( Vector2( U2, V1 ) );
	Buffers.UVs.PushBack( Vector2( U1, V2 ) );
	Buffers.UVs.PushBack( Vector2( U2, V2 ) );
	Buffers.UVs.PushBack( Vector2( Uh, Vh ) );
}

inline void EldritchWorld::PushIndices( index_t& BaseIndex, const bool CCW, struct SVoxelMeshBuffers& Buffers ) const
{
	if( CCW )
	{
		Buffers.Indices.PushBack( BaseIndex + 0 );
		Buffers.Indices.PushBack( BaseIndex + 1 );
		Buffers.Indices.PushBack( BaseIndex + 4 );
		Buffers.Indices.PushBack( BaseIndex + 1 );
		Buffers.Indices.PushBack( BaseIndex + 3 );
		Buffers.Indices.PushBack( BaseIndex + 4 );
		Buffers.Indices.PushBack( BaseIndex + 3 );
		Buffers.Indices.PushBack( BaseIndex + 2 );
		Buffers.Indices.PushBack( BaseIndex + 4 );
		Buffers.Indices.PushBack( BaseIndex + 2 );
		Buffers.Indices.PushBack( BaseIndex + 0 );
		Buffers.Indices.PushBack( BaseIndex + 4 );
	}
	else
	{
		Buffers.Indices.PushBack( BaseIndex + 0 );
		Buffers.Indices.PushBack( BaseIndex + 4 );
		Buffers.Indices.PushBack( BaseIndex + 1 );
		Buffers.Indices.PushBack( BaseIndex + 1 );
		Buffers.Indices.PushBack( BaseIndex + 4 );
		Buffers.Indices.PushBack( BaseIndex + 3 );
		Buffers.Indices.PushBack( BaseIndex + 3 );
		Buffers.Indices.PushBack( BaseIndex + 4 );
		Buffers.Indices.PushBack( BaseIndex + 2 );
		Buffers.Indices.PushBack( BaseIndex + 2 );
		Buffers.Indices.PushBack( BaseIndex + 4 );
		Buffers.Indices.PushBack( BaseIndex + 0 );
	}

	// HACK: Advance the index pointer. Just trying to minimize code in the calling function.
	BaseIndex += 5;
}

void EldritchWorld::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	WBWorld::GetInstance()->Tick( DeltaTime );
}

void EldritchWorld::Render() const
{
	XTRACE_FUNCTION;

	IRenderer* const pRenderer = m_Framework->GetRenderer();

	for( uint WorldMeshIndex = 0; WorldMeshIndex < m_NumMeshes; ++WorldMeshIndex )
	{
		Mesh* const pWorldMesh = m_WorldMeshes[ WorldMeshIndex ];
		if( pWorldMesh )
		{
			pRenderer->AddMesh( pWorldMesh );
		}
	}
}

#if BUILD_DEV
void EldritchWorld::DebugRender() const
{
	IRenderer* const pRenderer = m_Framework->GetRenderer();

	STATICHASH( EldritchWorld );

	STATICHASH( DebugRenderLightCrosses );
	static const bool RenderCrosses = ConfigManager::GetBool( sDebugRenderLightCrosses, false, sEldritchWorld );

	STATICHASH( DebugRenderLightSpheres );
	static const bool RenderSpheres = ConfigManager::GetBool( sDebugRenderLightSpheres, false, sEldritchWorld );

	if( RenderCrosses || RenderSpheres )
	{
		FOR_EACH_MAP( LightIter, m_LightMap, vidx_t, SVoxelLight )
		{
			const vidx_t		LightVoxel		= LightIter.GetKey();
			const SVoxelLight&	Light			= LightIter.GetValue();
			const Vector		LightLocation	= GetVoxelCenter( LightVoxel );

			if( RenderCrosses ) { pRenderer->DEBUGDrawCross(	LightLocation, Light.m_Radius, Light.m_Color.ToColor() ); }
			if( RenderSpheres ) { pRenderer->DEBUGDrawSphere(	LightLocation, Light.m_Radius, Light.m_Color.ToColor() ); }
		}
	}
}
#endif

bool EldritchWorld::Sweep( const Segment& SweepSegment, const Vector& HalfExtents, CollisionInfo& Info ) const
{
	CollisionInfo MinInfo;

	ASSERT( Info.m_CollideWorld || Info.m_CollideEntities );

	if( Info.m_CollideWorld )
	{
		CollisionInfo VoxelInfo;
		VoxelInfo.CopyInParametersFrom( Info );
		if( SweepVoxels( SweepSegment, HalfExtents, VoxelInfo ) )
		{
			MinInfo.CopyOutParametersFrom( VoxelInfo );
		}
	}

	if( Info.m_CollideEntities )
	{
		CollisionInfo EntitiesInfo;
		EntitiesInfo.CopyInParametersFrom( Info );
		if( SweepEntities( SweepSegment, HalfExtents, EntitiesInfo ) )
		{
			if( EntitiesInfo.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
			{
				MinInfo.CopyOutParametersFrom( EntitiesInfo );
			}
		}
	}

	Info.CopyOutParametersFrom( MinInfo );
	return Info.m_Collision;
}

bool EldritchWorld::SweepVoxels( const Segment& SweepSegment, const Vector& HalfExtents, CollisionInfo& Info ) const
{
	CollisionInfo MinInfo;
	CollisionInfo CheckInfo;

	Vector MinCorner;
	Vector MaxCorner;
	Vector::MinMax( SweepSegment.m_Point1, SweepSegment.m_Point2, MinCorner, MaxCorner );

	const AABB MovementBox( MinCorner - HalfExtents, MaxCorner + HalfExtents );
	vpos_t MinVoxelX = Clamp( static_cast<vpos_t>( MovementBox.m_Min.x ), 0, m_NumVoxelsX - 1 );
	vpos_t MinVoxelY = Clamp( static_cast<vpos_t>( MovementBox.m_Min.y ), 0, m_NumVoxelsY - 1 );
	vpos_t MinVoxelZ = Clamp( static_cast<vpos_t>( MovementBox.m_Min.z ), 0, m_NumVoxelsZ - 1 );
	vpos_t MaxVoxelX = Clamp( static_cast<vpos_t>( MovementBox.m_Max.x ), 0, m_NumVoxelsX - 1 );
	vpos_t MaxVoxelY = Clamp( static_cast<vpos_t>( MovementBox.m_Max.y ), 0, m_NumVoxelsY - 1 );
	vpos_t MaxVoxelZ = Clamp( static_cast<vpos_t>( MovementBox.m_Max.z ), 0, m_NumVoxelsZ - 1 );

	const Vector ExpandedExtents = HalfExtents + kVoxelHalfExtents;

	const uint	DeltaX			= 1 + MaxVoxelX - MinVoxelX;
	const uint	DeltaY			= 1 + MaxVoxelY - MinVoxelY;
	const uint	SkipVoxelsY		= m_NumVoxelsX - DeltaX;
	const uint	SkipVoxelsZ		= ( m_NumVoxelsY - DeltaY ) * m_NumVoxelsX;

	bool		ContinueSweep	= true;
	vidx_t		VoxelIndex		= GetIndex( MinVoxelX, MinVoxelY, MinVoxelZ );
	DEVASSERT( IsValidIndex( VoxelIndex ) );

	for( vpos_t VoxelZ = MinVoxelZ; VoxelZ <= MaxVoxelZ && ContinueSweep; ++VoxelZ, VoxelIndex += SkipVoxelsZ )
	{
		for( vpos_t VoxelY = MinVoxelY; VoxelY <= MaxVoxelY && ContinueSweep; ++VoxelY, VoxelIndex += SkipVoxelsY )
		{
			for( vpos_t VoxelX = MinVoxelX; VoxelX <= MaxVoxelX && ContinueSweep; ++VoxelX, ++VoxelIndex )
			{
				// I can turn these back on if I get paranoid, but this seems to work fine.
				//ASSERT( IsValidIndex( VoxelIndex ) );
				//DEBUGASSERT( VoxelIndex == GetIndex( VoxelX, VoxelY, VoxelZ ) );

				// I used to do a SafeGetVoxel here and treat null space as collidable,
				// but now I continue if the index is invalid. This should be fine.
				// Nothing is ever meant to sweep outside the bounds of the world.
				const vval_t VoxelValue = GetVoxel( VoxelIndex );
				if( VoxelValue == kEmptyVoxelValue )
				{
					continue;
				}

				// There's collision on this voxel.
				// Sweep moving box against voxel box by testing segment against expanded voxel box.

				const float VoxelCenterX = 0.5f + static_cast<float>( VoxelX );
				const float VoxelCenterY = 0.5f + static_cast<float>( VoxelY );
				const float VoxelCenterZ = 0.5f + static_cast<float>( VoxelZ );
				const Vector VoxelCenter( VoxelCenterX, VoxelCenterY, VoxelCenterZ );
				AABB VoxelBox = AABB::CreateFromCenterAndExtents( VoxelCenter, ExpandedExtents );

				CheckInfo.m_Collision = false;
				if( SweepSegment.Intersects( VoxelBox, &CheckInfo ) )
				{
					if( CheckInfo.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
					{
						MinInfo = CheckInfo;
						ContinueSweep = !Info.m_StopAtAnyCollision;
					}
				}
			}
		}
	}

	Info.CopyOutParametersFrom( MinInfo );
	return Info.m_Collision;
}

bool EldritchWorld::SweepEntities( const Segment& SweepSegment, const Vector& HalfExtents, CollisionInfo& Info ) const
{
	// This gets called enough that the profiler hook is a cost. >_<
	//PROFILE_FUNCTION;

	const uint	CollideAsType	= Info.m_UserFlags & EECF_Mask_CollideAs;
	const uint	EntityType		= Info.m_UserFlags & EECF_Mask_EntityTypes;
	const bool	CollideAll		= ( Info.m_UserFlags & EECF_CollideAllEntities ) > 0;

	ASSERT( 0 != ( CollideAsType ) );			// We need to be colliding as some type
	ASSERT( 1 == CountBits( CollideAsType ) );	// We should only be colliding as one type (optimization; can be changed, see WBCompEldCollision::AddToCollisionMap)
	ASSERT( 0 != ( EntityType ) );				// We need to be colliding against some type of entity

	const Array<WBCompEldCollision*>* const pCollisionComponents =
		CollideAll ?
		WBComponentArrays::GetComponents<WBCompEldCollision>() :
		WBCompEldCollision::GetCollisionArray( CollideAsType );

	if( !pCollisionComponents )
	{
		return false;
	}

	CollisionInfo MinInfo;
	CollisionInfo CheckInfo;

	Vector MinCorner;
	Vector MaxCorner;
	Vector::MinMax( SweepSegment.m_Point1, SweepSegment.m_Point2, MinCorner, MaxCorner );

	AABB SegmentBox( MinCorner, MaxCorner );

	bool ContinueSweep = true;
	const uint NumCollisionComponents = pCollisionComponents->Size();
	for( uint CollisionComponentIndex = 0; CollisionComponentIndex < NumCollisionComponents && ContinueSweep; ++CollisionComponentIndex )
	{
		WBCompEldCollision* const pCollision = ( *pCollisionComponents )[ CollisionComponentIndex ];
		DEVASSERT( pCollision );

		const uint CollisionFlags				= pCollision->GetCollisionFlags();
		const uint MatchedFlags					= ( CollisionFlags & Info.m_UserFlags );
		const uint MatchedEntityFlags			= MatchedFlags & EECF_Mask_EntityTypes;

#if BUILD_DEV
		if( !CollideAll )
		{
			const uint MatchedCollisionFlags = MatchedFlags & EECF_Mask_CollideAs;
			ASSERT( MatchedCollisionFlags != 0 );
		}
#endif

		if( 0 == MatchedEntityFlags )
		{
			continue;
		}

		WBEntity* const pEntity = pCollision->GetEntity();
		DEVASSERT( pEntity );

		if( pEntity == Info.m_CollidingEntity )
		{
			continue;
		}

		if( pEntity->IsDestroyed() )
		{
			continue;
		}

		AABB	CollisionBox	= pCollision->GetBounds();
		CollisionBox.ExpandBy( HalfExtents );

		if( !SegmentBox.Intersects( CollisionBox ) )
		{
			continue;
		}

		CheckInfo.m_Collision = false;
		if( SweepSegment.Intersects( CollisionBox, &CheckInfo ) )
		{
			CheckInfo.m_HitEntity = pEntity;
			if( CheckInfo.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
			{
				MinInfo = CheckInfo;
				ContinueSweep = !Info.m_StopAtAnyCollision;
			}
		}
	}

	Info.CopyOutParametersFrom( MinInfo );
	return Info.m_Collision;
}

bool EldritchWorld::Trace( const Ray& TraceRay, CollisionInfo& Info ) const
{
	// HACK: So that everything is just implemented via a segment sweep, use a fixed trace length.
	// This is set (in ::InitializeConfig) to a size big enough to cross the entire world at its
	// furthest corners, so it should be sufficient.
	const Segment TraceSegment( TraceRay.m_Point, TraceRay.m_Point + TraceRay.m_Direction * m_RayTraceLength );

	// Since a sweep is just a trace against expanded boxes, a trace is just a sweep with zero extents!
	return Sweep( TraceSegment, Vector(), Info );
}

bool EldritchWorld::Trace( const Segment& TraceSegment, CollisionInfo& Info ) const
{
	// Since a sweep is just a trace against expanded boxes, a trace is just a sweep with zero extents!
	return Sweep( TraceSegment, Vector(), Info );
}

bool EldritchWorld::CheckClearance( const Vector& Location, const Vector& HalfExtents, CollisionInfo& Info ) const
{
	// A zero length sweep should work fine for a clearance check
	const Segment TraceSegment( Location, Location );
	return Sweep( TraceSegment, HalfExtents, Info );
}

bool EldritchWorld::PointCheck( const Vector& Location, CollisionInfo& Info ) const
{
	// A zero length, zero extent sweep should work fine for a point check
	const Segment TraceSegment( Location, Location );
	return Sweep( TraceSegment, Vector(), Info );
}

bool EldritchWorld::LineCheck( const Vector& Start, const Vector& End, CollisionInfo& Info ) const
{
	const Segment TraceSegment( Start, End );
	return Sweep( TraceSegment, Vector(), Info );
}

bool EldritchWorld::SweepCheck( const Vector& Start, const Vector& End, const Vector& HalfExtents, CollisionInfo& Info ) const
{
	const Segment TraceSegment( Start, End );
	return Sweep( TraceSegment, HalfExtents, Info );
}

bool EldritchWorld::FindSpot( Vector& InOutSpot, const Vector& Extents, CollisionInfo& Info ) const
{
	PROFILE_FUNCTION;

	// HACKHACK: If we have a zero-extents entity, just use the given spot.
	// This fixes some behavior that got broken by the cardinal direction check below.
	if( Extents.IsZero() )
	{
		return true;
	}

	// First, see if the given spot is ok.
	if( !CheckClearance( InOutSpot, Extents, Info ) )
	{
		return true;
	}

	// Given spot is not ok; we need to find a spot.

	// Check if the zero extents spot is usable.
	if( PointCheck( InOutSpot, Info ) )
	{
		// If it's not, try moving by 1m in each cardinal direction.
		const Vector UnitX( 1.0f, 0.0f, 0.0f );
		const Vector UnitY( 0.0f, 1.0f, 0.0f );
		const Vector UnitZ( 0.0f, 0.0f, 1.0f );

		if( !PointCheck( InOutSpot + UnitX, Info ) )		{ InOutSpot += UnitX; }
		else if( !PointCheck( InOutSpot - UnitX, Info ) )	{ InOutSpot -= UnitX; }
		else if( !PointCheck( InOutSpot + UnitY, Info ) )	{ InOutSpot += UnitY; }
		else if( !PointCheck( InOutSpot - UnitY, Info ) )	{ InOutSpot -= UnitY; }
		else if( !PointCheck( InOutSpot + UnitZ, Info ) )	{ InOutSpot += UnitZ; }
		else if( !PointCheck( InOutSpot - UnitZ, Info ) )	{ InOutSpot -= UnitZ; }
		else
		{
			// Moving in cardinal directions didn't help. For now, just bail out and don't try anymore.
			return false;
		}
	}

	// Zero extents spot is good.
	// Push InOutSpot away from nearby surfaces along each axis.
	for( uint Dir = 0; Dir < 3; ++Dir )
	{
		static const float kSmallDistance = 0.1f;

		Vector TraceDir;
		TraceDir.v[ Dir ] = Extents.v[ Dir ] + kSmallDistance;

		if( LineCheck( InOutSpot, InOutSpot + TraceDir, Info ) )
		{
			InOutSpot = Info.m_Intersection - TraceDir;
		}
		// Using "else" here because there's no need to check the second direction
		// if the first collides; if there's collision there, we can't resolve it.
		else if( LineCheck( InOutSpot, InOutSpot - TraceDir, Info ) )
		{
			InOutSpot = Info.m_Intersection + TraceDir;
		}
	}

	// Now we know that each axis (like an "axial caltrop") fits or doesn't
	// We might be able to fit now, by pushing away from occlusions directly along each axis.
	if( !CheckClearance( InOutSpot, Extents, Info ) )
	{
		return true;
	}

	// If not, then we need to sweep a line segment along each axis to find more complex intersections.
	for( uint Dir = 0; Dir < 3; ++Dir )
	{
		const uint NextDir = ( Dir + 1 ) % 3;
		static const float kSmallDistance = 0.1f;

		Vector TraceDir;
		TraceDir.v[ Dir ] = Extents.v[ Dir ] + kSmallDistance;

		Vector LineExtents;
		LineExtents.v[ NextDir ] = Extents.v[ NextDir ];

		if( SweepCheck( InOutSpot, InOutSpot + TraceDir, LineExtents, Info ) )
		{
			InOutSpot = Info.m_Intersection - TraceDir;
		}
		// Using "else" here because there's no need to check the second direction
		// if the first collides; if there's collision there, we can't resolve it.
		else if( SweepCheck( InOutSpot, InOutSpot - TraceDir, LineExtents, Info ) )
		{
			InOutSpot = Info.m_Intersection + TraceDir;
		}
	}

	// Now we know that each planar section fits or doesn't.
	// Try again to check for final clearance before continuing.
	if( !CheckClearance( InOutSpot, Extents, Info ) )
	{
		return true;
	}

	// Finally, we need to sweep a rectangle along each axis to find more complex intersections.
	for( uint Dir = 0; Dir < 3; ++Dir )
	{
		static const float kSmallDistance = 0.1f;

		Vector TraceDir;
		TraceDir.v[ Dir ] = Extents.v[ Dir ] + kSmallDistance;

		Vector PlaneExtents = Extents;
		PlaneExtents.v[ Dir ] = 0.0f;

		if( SweepCheck( InOutSpot, InOutSpot + TraceDir, PlaneExtents, Info ) )
		{
			InOutSpot = Info.m_Intersection - TraceDir;
		}
		// Using "else" here because there's no need to check the second direction
		// if the first collides; if there's collision there, we can't resolve it.
		else if( SweepCheck( InOutSpot, InOutSpot - TraceDir, PlaneExtents, Info ) )
		{
			InOutSpot = Info.m_Intersection + TraceDir;
		}
	}

	// Now, the whole volume fits, or doesn't. Check final clearance.
	return !CheckClearance( InOutSpot, Extents, Info );
}

inline bool EldritchWorld::HasAnyNeighbors( const vnbr_t VoxelNeighbors, const vnbr_t NeighborFlags ) const
{
	return ( VoxelNeighbors & NeighborFlags ) > 0;
}

inline bool EldritchWorld::HasAnyNeighbors( const vxnb_t VoxelExtendedNeighbors, const vxnb_t ExtendedNeighborFlags ) const
{
	return ( VoxelExtendedNeighbors & ExtendedNeighborFlags ) > 0;
}

inline uint EldritchWorld::CountNeighbors( const vxnb_t VoxelExtendedNeighbors, const vxnb_t ExtendedNeighborFlags ) const
{
	const vxnb_t MatchedNeighbors = VoxelExtendedNeighbors & ExtendedNeighborFlags;
	return CountBits( MatchedNeighbors );
}

// We expect 0-3 neighbors
inline float EldritchWorld::GetAOForNeighbors( const uint NumNeighbors ) const
{
	DEBUGASSERT( NumNeighbors < 4 );
	static const float AO[] = { 1.0f, 0.667f, 0.333f, 0.0f };
	return AO[ NumNeighbors ];
}

// NOTE: This used to be inline, but not every compiler would link it properly.
/*inline*/ bool EldritchWorld::IsValidIndex( const vidx_t VoxelIndex ) const
{
	return VoxelIndex < m_NumVoxels;
}

inline bool EldritchWorld::IsValidX( const vpos_t X ) const
{
	return X >= 0 && X < m_NumVoxelsX;
}

inline bool EldritchWorld::IsValidY( const vpos_t Y ) const
{
	return Y >= 0 && Y < m_NumVoxelsY;
}

inline bool EldritchWorld::IsValidZ( const vpos_t Z ) const
{
	return Z >= 0 && Z < m_NumVoxelsZ;
}

// NOTE: This used to be inline, but not every compiler would link it properly.
/*inline*/ void EldritchWorld::GetCoords( const vidx_t VoxelIndex, vpos_t& OutX, vpos_t& OutY, vpos_t& OutZ ) const
{
	DEVASSERT( IsValidIndex( VoxelIndex ) );

	OutX = VoxelIndex % m_NumVoxelsX;
	OutY = ( VoxelIndex / m_NumVoxelsX ) % m_NumVoxelsY;
	OutZ = ( ( VoxelIndex / m_NumVoxelsX ) / m_NumVoxelsY ) % m_NumVoxelsZ;
}

// NOTE: This used to be inline, but not every compiler would link it properly.
/*inline*/ vidx_t EldritchWorld::GetIndex( const vpos_t X, const vpos_t Y, const vpos_t Z ) const
{
	if( IsValidX( X ) && IsValidY( Y ) && IsValidZ( Z ) )
	{
		return X + ( Y * m_NumVoxelsX ) + ( Z * m_NumVoxelsX * m_NumVoxelsY );
	}
	else
	{
		return kInvalidVoxelIndex;
	}
}

// NOTE: This used to be inline, but not every compiler would link it properly.
/*inline*/ vidx_t EldritchWorld::GetIndex( const Vector& Location ) const
{
	// Note we simply drop the fractional part instead of rounding down.
	const vpos_t VoxelX = static_cast<int>( Location.x );
	const vpos_t VoxelY = static_cast<int>( Location.y );
	const vpos_t VoxelZ = static_cast<int>( Location.z );

	return GetIndex( VoxelX, VoxelY, VoxelZ );
}

Vector EldritchWorld::GetVoxelCenter( const Vector& Location ) const
{
	return Vector( Floor( Location.x ) + 0.5f, Floor( Location.y ) + 0.5f, Floor( Location.z ) + 0.5f );
}

Vector EldritchWorld::GetVoxelBase( const Vector& Location ) const
{
	return Vector( Floor( Location.x ), Floor( Location.y ), Floor( Location.z ) );
}

// NOTE: This used to be inline, but not every compiler would link it properly.
/*inline*/ Vector EldritchWorld::GetVoxelCenter( const vidx_t VoxelIndex ) const
{
	return GetVoxelBase( VoxelIndex ) + kVoxelHalfExtents;
}

inline Vector EldritchWorld::GetVoxelBase( const vidx_t VoxelIndex ) const
{
	vpos_t X, Y, Z;
	GetCoords( VoxelIndex, X, Y, Z );
	return Vector( static_cast<float>( X ), static_cast<float>( Y ), static_cast<float>( Z ) );
}

inline vidx_t EldritchWorld::GetIndexNeighborRight( const vidx_t VoxelIndex ) const
{
	return VoxelIndex + 1;
}

inline vidx_t EldritchWorld::GetIndexNeighborLeft( const vidx_t VoxelIndex ) const
{
	return VoxelIndex - 1;
}

inline vidx_t EldritchWorld::GetIndexNeighborFront( const vidx_t VoxelIndex ) const
{
	return VoxelIndex + m_NumVoxelsX;
}

inline vidx_t EldritchWorld::GetIndexNeighborBack( const vidx_t VoxelIndex ) const
{
	return VoxelIndex - m_NumVoxelsX;
}

inline vidx_t EldritchWorld::GetIndexNeighborUp( const vidx_t VoxelIndex ) const
{
	return VoxelIndex + m_NumVoxelsX * m_NumVoxelsY;
}

inline vidx_t EldritchWorld::GetIndexNeighborDown( const vidx_t VoxelIndex ) const
{
	return VoxelIndex - m_NumVoxelsX * m_NumVoxelsY;
}

inline vidx_t EldritchWorld::GetIndexNeighbor( const vidx_t VoxelIndex, const vdir_t Direction ) const
{
	vidx_t RetVal = VoxelIndex;

	if( Direction & EDirectionRight )	{ RetVal += 1; }
	if( Direction & EDirectionLeft )	{ RetVal -= 1; }
	if( Direction & EDirectionFront )	{ RetVal += m_NumVoxelsX; }
	if( Direction & EDirectionBack )	{ RetVal -= m_NumVoxelsX; }
	if( Direction & EDirectionUp )		{ RetVal += m_NumVoxelsX * m_NumVoxelsY; }
	if( Direction & EDirectionDown )	{ RetVal -= m_NumVoxelsX * m_NumVoxelsY; }

	return RetVal;
}

// NOTE: This used to be inline, but not every compiler would link it properly.
/*inline*/ vval_t EldritchWorld::GetVoxel( const vidx_t VoxelIndex ) const
{
	DEBUGASSERT( IsValidIndex( VoxelIndex ) );
	return m_VoxelMap[ VoxelIndex ];
}

// NOTE: This used to be inline, but not every compiler would link it properly.
/*inline*/ void EldritchWorld::SetVoxel( const vidx_t VoxelIndex, const vval_t VoxelValue )
{
	DEBUGASSERT( IsValidIndex( VoxelIndex ) );
	m_VoxelMap[ VoxelIndex ] = VoxelValue;
}

inline vval_t EldritchWorld::SafeGetVoxel( const vidx_t VoxelIndex ) const
{
	return IsValidIndex( VoxelIndex ) ? GetVoxel( VoxelIndex ) : kInvalidVoxelValue;
}

inline void EldritchWorld::SafeSetVoxel( const vidx_t VoxelIndex, const vval_t VoxelValue )
{
	if( IsValidIndex( VoxelIndex ) )
	{
		SetVoxel( VoxelIndex, VoxelValue );
	}
}

inline const SVoxelIrradiance& EldritchWorld::GetIrradiance( const vidx_t VoxelIndex ) const
{
	DEBUGASSERT( IsValidIndex( VoxelIndex ) );
	return m_IrradianceMap[ VoxelIndex ];
}

inline SVoxelIrradiance& EldritchWorld::GetIrradiance( const vidx_t VoxelIndex )
{
	DEBUGASSERT( IsValidIndex( VoxelIndex ) );
	return m_IrradianceMap[ VoxelIndex ];
}

inline vnbr_t EldritchWorld::GetNeighbors( const vidx_t VoxelIndex ) const
{
	vnbr_t RetVal = 0;

	const vval_t NeighborRight	= SafeGetVoxel( GetIndexNeighborRight( VoxelIndex ) );
	const vval_t NeighborLeft	= SafeGetVoxel( GetIndexNeighborLeft( VoxelIndex ) );
	const vval_t NeighborFront	= SafeGetVoxel( GetIndexNeighborFront( VoxelIndex ) );
	const vval_t NeighborBack	= SafeGetVoxel( GetIndexNeighborBack( VoxelIndex ) );
	const vval_t NeighborUp		= SafeGetVoxel( GetIndexNeighborUp( VoxelIndex ) );
	const vval_t NeighborDown	= SafeGetVoxel( GetIndexNeighborDown( VoxelIndex ) );

	if( NeighborRight > 0 ) { RetVal |= ENeighborRight; }
	if( NeighborLeft > 0 ) { RetVal |= ENeighborLeft; }
	if( NeighborFront > 0 ) { RetVal |= ENeighborFront; }
	if( NeighborBack > 0 ) { RetVal |= ENeighborBack; }
	if( NeighborUp > 0 ) { RetVal |= ENeighborUp; }
	if( NeighborDown > 0 ) { RetVal |= ENeighborDown; }

	return RetVal;
}

inline vxnb_t EldritchWorld::GetExtendedNeighbors( const vidx_t VoxelIndex ) const
{
	vxnb_t RetVal = 0;

#define TRY_ADD_NEIGHBOR( tag ) if( SafeGetVoxel( GetIndexNeighbor( VoxelIndex, EDirection##tag ) ) > 0 ) { RetVal |= ENeighbor##tag; }
	TRY_ADD_NEIGHBOR( RFU );
	TRY_ADD_NEIGHBOR( RFC );
	TRY_ADD_NEIGHBOR( RFD );
	TRY_ADD_NEIGHBOR( RCU );
	TRY_ADD_NEIGHBOR( RCC );
	TRY_ADD_NEIGHBOR( RCD );
	TRY_ADD_NEIGHBOR( RBU );
	TRY_ADD_NEIGHBOR( RBC );
	TRY_ADD_NEIGHBOR( RBD );
	TRY_ADD_NEIGHBOR( CFU );
	TRY_ADD_NEIGHBOR( CFC );
	TRY_ADD_NEIGHBOR( CFD );
	TRY_ADD_NEIGHBOR( CCU );
	TRY_ADD_NEIGHBOR( CCC );
	TRY_ADD_NEIGHBOR( CCD );
	TRY_ADD_NEIGHBOR( CBU );
	TRY_ADD_NEIGHBOR( CBC );
	TRY_ADD_NEIGHBOR( CBD );
	TRY_ADD_NEIGHBOR( LFU );
	TRY_ADD_NEIGHBOR( LFC );
	TRY_ADD_NEIGHBOR( LFD );
	TRY_ADD_NEIGHBOR( LCU );
	TRY_ADD_NEIGHBOR( LCC );
	TRY_ADD_NEIGHBOR( LCD );
	TRY_ADD_NEIGHBOR( LBU );
	TRY_ADD_NEIGHBOR( LBC );
	TRY_ADD_NEIGHBOR( LBD );
#undef TRY_ADD_NEIGHBOR

	return RetVal;
}

void EldritchWorld::SetMaze( const Array<uint8>& Maze )
{
	m_Maze = Maze;
}

#define VERSION_EMPTY		0
#define VERSION_VOXELS		1
#define VERSION_LIGHTS		2
#define VERSION_GLOBALLIGHT	3
#define VERSION_WORLDDEF	4
#define VERSION_MAZE		5
#define VERSION_CURRENT		5

void EldritchWorld::Save( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	PROFILE_FUNCTION;

	Stream.WriteUInt32( VERSION_CURRENT );

	// Write voxels
	PackVoxels();
	Stream.WriteUInt32( m_PackedVoxels.Size() );
	Stream.Write( m_PackedVoxels.Size(), m_PackedVoxels.GetData() );

	// Write lights
	Stream.WriteUInt32( m_LightMap.Size() );
	FOR_EACH_MAP( LightIter, m_LightMap, vidx_t, SVoxelLight )
	{
		const SVoxelLight& Light = LightIter.GetValue();
		Stream.WriteUInt32( LightIter.GetKey() );
		Stream.Write( sizeof( SVoxelLight ), &Light );
	}

	// Write global light
	Stream.Write( sizeof( SVoxelIrradiance ), &m_GlobalLight );
	Stream.Write( sizeof( Vector4 ), &m_AOColor );

	// Write world def
	Stream.WriteHashedString( m_CurrentWorldDef );

	// Write maze configuration
	Stream.WriteUInt32( m_Maze.Size() );
	Stream.Write( m_Maze.MemorySize(), m_Maze.GetData() );

	WBWorld::GetInstance()->Save( Stream );
}

void EldritchWorld::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	PROFILE_FUNCTION;

	// Shut down in advance, because it may affect the world.
	m_DisableCompute = true;
	WBWorld::GetInstance()->ShutDown();
	m_DisableCompute = false;

	m_LightMap.Clear();
	m_IrradianceMap.MemoryZero();
	ClearLightInfluenceMap();
	m_Nav->InitializeWorldValues();

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_VOXELS )
	{
		const uint PackedVoxelsSize = Stream.ReadUInt32();
		m_PackedVoxels.Resize( PackedVoxelsSize );
		Stream.Read( PackedVoxelsSize, m_PackedVoxels.GetData() );
	}

	if( Version >= VERSION_LIGHTS )
	{
		const uint NumLights = Stream.ReadUInt32();
		for( uint LightIndex = 0; LightIndex < NumLights; ++LightIndex )
		{
			const vidx_t Key = Stream.ReadUInt32();
			SVoxelLight Light;
			Stream.Read( sizeof( SVoxelLight ), &Light );
			m_LightMap[ Key ] = Light;
		}
	}

	if( Version >= VERSION_GLOBALLIGHT )
	{
		Stream.Read( sizeof( SVoxelIrradiance ), &m_GlobalLight );
		Stream.Read( sizeof( Vector4 ), &m_AOColor );
	}

	if( Version >= VERSION_WORLDDEF )
	{
		m_CurrentWorldDef = Stream.ReadHashedString();
	}

	if( Version >= VERSION_MAZE )
	{
		// Write maze configuration
		const uint MazeSize = Stream.ReadUInt32();
		m_Maze.Resize( MazeSize );
		Stream.Read( m_Maze.MemorySize(), m_Maze.GetData() );
	}

	EldritchGame* const	pGame			= EldritchFramework::GetInstance()->GetGame();
	const SWorldDef&	CurrentWorldDef	= GetWorldDef();
	pGame->SetColorGradingTexture( CurrentWorldDef.m_ColorGrading );
	pGame->SetFogParams( CurrentWorldDef.m_FogNear, CurrentWorldDef.m_FogFar, CurrentWorldDef.m_FogTexture );
	pGame->SetCurrentMusic( CurrentWorldDef.m_Music );

	// Make sure voxels and lights are up to date before loading world.
	// I *think* this was important for some bug. Else I could just load WBWorld first.
	UnpackVoxels();
	AddAllLightInfluences();
	ComputeAllIrradiance();
	BuildAllMeshes();
	m_Nav->UpdateWorldFromAllVoxels();

	WBWorld::GetInstance()->Load( Stream );
}

void EldritchWorld::ClearLightInfluenceMap()
{
	PROFILE_FUNCTION;

	for( uint LightInfluenceIndex = 0; LightInfluenceIndex < m_LightInfluenceMap.Size(); ++LightInfluenceIndex )
	{
		LightInfluences& Influences = m_LightInfluenceMap[ LightInfluenceIndex ];
		Influences.Clear();
	}
}

AABB EldritchWorld::GetVoxelBox( const vidx_t VoxelIndex ) const
{
	const Vector VoxelMin = GetVoxelBase( VoxelIndex );
	const Vector VoxelMax = VoxelMin + kVoxelExtents;
	return AABB( VoxelMin, VoxelMax );
}

void EldritchWorld::ExpandBoxAroundVoxels( const Set<vidx_t>& Voxels, AABB& InOutBox ) const
{
	PROFILE_FUNCTION;

	FOR_EACH_SET( VoxelIter, Voxels, vidx_t )
	{
		const vidx_t VoxelIndex = VoxelIter.GetValue();
		const Vector VoxelMin = GetVoxelBase( VoxelIndex );
		const Vector VoxelMax = VoxelMin + kVoxelExtents;
		InOutBox.ExpandTo( VoxelMin );
		InOutBox.ExpandTo( VoxelMax );
	}
}

void EldritchWorld::SendOnWorldChangedEvent( const AABB& ChangedBox ) const
{
	WB_MAKE_EVENT( OnWorldChanged, NULL );
	WB_SET_AUTO( OnWorldChanged, Vector, BoxMin, ChangedBox.m_Min );
	WB_SET_AUTO( OnWorldChanged, Vector, BoxMax, ChangedBox.m_Max );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), OnWorldChanged, NULL );
}

bool EldritchWorld::AddVoxelAt( const Vector& Location, const vval_t VoxelValue )
{
	CollisionInfo Info;
	Info.m_CollideEntities = true;
	Info.m_UserFlags = EECF_CollideAsWorld | EECF_CollideStaticEntities | EECF_CollideDynamicEntities;

	if( CheckClearance( GetVoxelCenter( Location ), kVoxelHalfExtents, Info ) )
	{
		// Something is blocking this from being added.
		return false;
	}

	vidx_t VoxelIndex = GetIndex( Location );
	if( VoxelIndex == kInvalidVoxelIndex )
	{
		return false;
	}

	SetVoxel( VoxelIndex, VoxelValue );

	Set<vidx_t> ChangedVoxels;
	ChangedVoxels.Insert( VoxelIndex );
	GatherLightInfluenceVoxels( ChangedVoxels, VoxelIndex );
	ComputeIrradianceForVoxels( ChangedVoxels );
	BuildMeshesForChangedVoxels( ChangedVoxels );
	m_Nav->UpdateWorldFromChangedVoxels( ChangedVoxels );

	const AABB ChangedBox = GetVoxelBox( VoxelIndex );
	SendOnWorldChangedEvent( ChangedBox );

	return true;
}

void EldritchWorld::RemoveVoxelAt( const Vector& Location )
{
	vidx_t VoxelIndex = GetIndex( Location );
	if( VoxelIndex == kInvalidVoxelIndex )
	{
		return;
	}

	vval_t CurrentValue = GetVoxel( VoxelIndex );
	if( CurrentValue & kImpenetrableVoxelFlag )
	{
		return;
	}

	SetVoxel( VoxelIndex, kEmptyVoxelValue );

	Set<vidx_t> ChangedVoxels;
	ChangedVoxels.Insert( VoxelIndex );
	GatherLightInfluenceVoxels( ChangedVoxels, VoxelIndex );
	ComputeIrradianceForVoxels( ChangedVoxels );
	BuildMeshesForChangedVoxels( ChangedVoxels );
	m_Nav->UpdateWorldFromChangedVoxels( ChangedVoxels );

	const AABB ChangedBox = GetVoxelBox( VoxelIndex );
	SendOnWorldChangedEvent( ChangedBox );
}

void EldritchWorld::RemoveVoxelsAt( const Vector& Location, const float Radius )
{
	vidx_t VoxelIndex = GetIndex( Location );
	if( VoxelIndex == kInvalidVoxelIndex )
	{
		return;
	}

	// Gather all voxels in radius
	Set<vidx_t> AffectedVoxels;
	GatherVoxelsInRadius( AffectedVoxels, VoxelIndex, Radius );

	// Filter impenetrable voxels from that set
	Set<vidx_t> ChangedVoxels;
	FOR_EACH_SET( VoxelIter, AffectedVoxels, vidx_t )
	{
		const vidx_t	AffectedVoxelIndex	= VoxelIter.GetValue();
		vval_t			AffectedVoxelValue	= GetVoxel( AffectedVoxelIndex );

		if( AffectedVoxelValue & kImpenetrableVoxelFlag )
		{
			continue;
		}

		SetVoxel( AffectedVoxelIndex, kEmptyVoxelValue );
		ChangedVoxels.Insert( AffectedVoxelIndex );
	}

	const Vector	RadiusExtents	= Vector( Radius, Radius, Radius );
	const AABB		ChangedBox		= AABB::CreateFromCenterAndExtents( GetVoxelCenter( VoxelIndex ), RadiusExtents );

	// Updates light and meshes
	OnBoxChanged( ChangedBox );

	m_Nav->UpdateWorldFromChangedVoxels( ChangedVoxels );
	SendOnWorldChangedEvent( ChangedBox );
}

void EldritchWorld::OnBoxChanged( const AABB& Box )
{
	PROFILE_FUNCTION;

	Set<vidx_t> ChangedVoxels;
	GatherVoxelsInBox( ChangedVoxels, Box );

	Set<vidx_t> AffectedVoxels;
	FOR_EACH_SET( VoxelIter, ChangedVoxels, vidx_t )
	{
		const vidx_t ChangedVoxel = VoxelIter.GetValue();
		AffectedVoxels.Insert( ChangedVoxel );
		GatherLightInfluenceVoxels( AffectedVoxels, ChangedVoxel );
	}
	ComputeIrradianceForVoxels( AffectedVoxels );
	BuildMeshesForChangedVoxels( AffectedVoxels );
}

// For all the lights that influence a voxel, gather all the other voxels they influence.
void EldritchWorld::GatherLightInfluenceVoxels( Set<vidx_t>& Voxels, const vidx_t VoxelIndex ) const
{
	PROFILE_FUNCTION;

	if( m_DisableCompute )
	{
		return;
	}

	DEVASSERT( IsValidIndex( VoxelIndex ) );
	const LightInfluences& LocalLightInfluences = m_LightInfluenceMap[ VoxelIndex ];

	FOR_EACH_SET( InfluenceIter, LocalLightInfluences, vidx_t )
	{
		const vidx_t LightVoxel = InfluenceIter.GetValue();

		// If this fails, something is probably happening out of order. Remove light, compute irradiance, then do this.
		DEVASSERT( m_LightMap.Search( LightVoxel ).IsValid() );
		const SVoxelLight& Light = m_LightMap[ LightVoxel ];
		GatherVoxelsInRadius( Voxels, LightVoxel, Light.m_Radius );
	}
}

// NOTE: This intentionally does not clear Voxels.
void EldritchWorld::GatherVoxelsInRadius( Set<vidx_t>& Voxels, const vidx_t VoxelIndex, const float Radius ) const
{
	PROFILE_FUNCTION;

	if( m_DisableCompute )
	{
		return;
	}

	vpos_t VoxelX, VoxelY, VoxelZ;
	GetCoords( VoxelIndex, VoxelX, VoxelY, VoxelZ );

	const Vector Center = GetVoxelCenter( VoxelIndex );
	const float RadiusSq = Square( Radius );

	const int VoxelRadius = static_cast<int>( Ceiling( Radius ) );
	const vpos_t MinX = VoxelX - VoxelRadius;
	const vpos_t MaxX = VoxelX + VoxelRadius;
	const vpos_t MinY = VoxelY - VoxelRadius;
	const vpos_t MaxY = VoxelY + VoxelRadius;
	const vpos_t MinZ = VoxelZ - VoxelRadius;
	const vpos_t MaxZ = VoxelZ + VoxelRadius;

	for( vpos_t Z = MinZ; Z <= MaxZ; ++Z )
	{
		for( vpos_t Y = MinY; Y <= MaxY; ++Y )
		{
			for( vpos_t X = MinX; X <= MaxX; ++X )
			{
				vidx_t IterIndex = GetIndex( X, Y, Z );
				if( !IsValidIndex( IterIndex ) )
				{
					continue;
				}

				const Vector IterCenter = GetVoxelCenter( IterIndex );
				const float DistSq = ( IterCenter - Center ).LengthSquared();
				if( DistSq > RadiusSq )
				{
					continue;
				}

				Voxels.Insert( IterIndex );
			}
		}
	}
}

void EldritchWorld::GatherVoxelsInBox( Set<vidx_t>& Voxels, const AABB& Box ) const
{
	PROFILE_FUNCTION;

	if( m_DisableCompute )
	{
		return;
	}

	const vpos_t MinX = static_cast<vpos_t>( Box.m_Min.x );
	const vpos_t MaxX = static_cast<vpos_t>( Box.m_Max.x );
	const vpos_t MinY = static_cast<vpos_t>( Box.m_Min.y );
	const vpos_t MaxY = static_cast<vpos_t>( Box.m_Max.y );
	const vpos_t MinZ = static_cast<vpos_t>( Box.m_Min.z );
	const vpos_t MaxZ = static_cast<vpos_t>( Box.m_Max.z );

	for( vpos_t Z = MinZ; Z <= MaxZ; ++Z )
	{
		for( vpos_t Y = MinY; Y <= MaxY; ++Y )
		{
			for( vpos_t X = MinX; X <= MaxX; ++X )
			{
				vidx_t IterIndex = GetIndex( X, Y, Z );
				if( !IsValidIndex( IterIndex ) )
				{
					continue;
				}

				Voxels.Insert( IterIndex );
			}
		}
	}
}

void EldritchWorld::AddLightInfluence( const Set<vidx_t>& AffectedVoxels, const vidx_t LightVoxel )
{
	FOR_EACH_SET( VoxelIter, AffectedVoxels, vidx_t )
	{
		const vidx_t		VoxelIndex				= VoxelIter.GetValue();
		LightInfluences&	LocalLightInfluences	= m_LightInfluenceMap[ VoxelIndex ];
		
		DEVASSERT( LocalLightInfluences.Search( LightVoxel ).IsNull() );
		LocalLightInfluences.Insert( LightVoxel );
	}
}

void EldritchWorld::RemoveLightInfluence( const Set<vidx_t>& AffectedVoxels, const vidx_t LightVoxel )
{
	FOR_EACH_SET( VoxelIter, AffectedVoxels, vidx_t )
	{
		const vidx_t		VoxelIndex				= VoxelIter.GetValue();
		LightInfluences&	LocalLightInfluences	= m_LightInfluenceMap[ VoxelIndex ];

		DEVASSERT( LocalLightInfluences.Search( LightVoxel ).IsValid() );
		LocalLightInfluences.Remove( LightVoxel );
	}
}

bool EldritchWorld::AddLightAt( const Vector& Location, const float Radius, const Vector4& Color )
{
	vidx_t VoxelIndex = GetIndex( Location );
	if( VoxelIndex == kInvalidVoxelIndex )
	{
		return false;
	}

	// Don't add a light on top of existing light. First come, first served.
	if( m_LightMap.Search( VoxelIndex ).IsValid() )
	{
		return false;
	}

	SVoxelLight NewLight;
	NewLight.m_Color = Color;
	NewLight.m_Radius = Radius;

	m_LightMap[ VoxelIndex ] = NewLight;

	Set<vidx_t> AffectedVoxels;
	GatherVoxelsInRadius( AffectedVoxels, VoxelIndex, NewLight.m_Radius );
	AddLightInfluence( AffectedVoxels, VoxelIndex );
	ComputeIrradianceForVoxels( AffectedVoxels );
	BuildMeshesForChangedVoxels( AffectedVoxels );

	return true;
}

void EldritchWorld::RemoveLightAt( const Vector& Location )
{
	vidx_t VoxelIndex = GetIndex( Location );
	if( VoxelIndex == kInvalidVoxelIndex )
	{
		return;
	}

	LightMap::Iterator LightIter = m_LightMap.Search( VoxelIndex );
	if( LightIter.IsNull() )
	{
		return;
	}

	// Make a copy since we're about to remove the light.
	const float RemovedLightRadius = LightIter.GetValue().m_Radius;

	m_LightMap.Remove( LightIter );

	Set<vidx_t> AffectedVoxels;
	GatherVoxelsInRadius( AffectedVoxels, VoxelIndex, RemovedLightRadius );
	RemoveLightInfluence( AffectedVoxels, VoxelIndex );
	ComputeIrradianceForVoxels( AffectedVoxels );
	BuildMeshesForChangedVoxels( AffectedVoxels );
}

void EldritchWorld::ResetIrradiance( const vidx_t VoxelIndex )
{
	SVoxelIrradiance& Irradiance = GetIrradiance( VoxelIndex );
	memset( &Irradiance.m_Light, 0, sizeof( SVoxelIrradiance ) );
}

void EldritchWorld::AddAllLightInfluences()
{
#if BUILD_DEV
	// Make sure light influences are already cleared, else we need to call ClearLightInfluenceMap first.
	for( uint LightInfluenceIndex = 0; LightInfluenceIndex < m_LightInfluenceMap.Size(); ++LightInfluenceIndex )
	{
		const LightInfluences& Influences = m_LightInfluenceMap[ LightInfluenceIndex ];
		ASSERT( Influences.Empty() );
	}
#endif

	FOR_EACH_MAP( LightIter, m_LightMap, vidx_t, SVoxelLight )
	{
		const vidx_t LightVoxel = LightIter.GetKey();
		const SVoxelLight& Light = LightIter.GetValue();

		Set<vidx_t> InfluencedVoxels;
		GatherVoxelsInRadius( InfluencedVoxels, LightVoxel, Light.m_Radius );
		AddLightInfluence( InfluencedVoxels, LightVoxel );
	}
}

void EldritchWorld::ComputeAllIrradiance()
{
	PROFILE_FUNCTION;

	ASSERT( m_NumVoxels == m_IrradianceMap.Size() );

	for( vidx_t VoxelIndex = 0; VoxelIndex < m_NumVoxels; ++VoxelIndex )
	{
		ResetIrradiance( VoxelIndex );

		const LightInfluences&	LocalLightInfluences	= m_LightInfluenceMap[ VoxelIndex ];
		FOR_EACH_SET( InfluenceIter, LocalLightInfluences, vidx_t )
		{
			const vidx_t		LightVoxel	= InfluenceIter.GetValue();
			const SVoxelLight&	Light		= m_LightMap[ LightVoxel ];

			ComputePartialIrradiance( VoxelIndex, LightVoxel, Light );
		}
	}
}

void EldritchWorld::ComputeIrradianceForVoxels( const Set<vidx_t>& Voxels )
{
	PROFILE_FUNCTION;

	FOR_EACH_SET( VoxelIter, Voxels, vidx_t )
	{
		const vidx_t VoxelIndex = VoxelIter.GetValue();
		DEVASSERT( IsValidIndex( VoxelIndex ) );

		ResetIrradiance( VoxelIndex );

		const LightInfluences&	LocalLightInfluences	= m_LightInfluenceMap[ VoxelIndex ];
		FOR_EACH_SET( InfluenceIter, LocalLightInfluences, vidx_t )
		{
			const vidx_t		LightVoxel	= InfluenceIter.GetValue();
			const SVoxelLight&	Light		= m_LightMap[ LightVoxel ];

			ComputePartialIrradiance( VoxelIndex, LightVoxel, Light );
		}
	}
}

void EldritchWorld::ComputePartialIrradiance( vidx_t VoxelIndex, vidx_t LightVoxel, const SVoxelLight& Light )
{
	// This gets called enough that the profiler hook is a cost. >_<
	//PROFILE_FUNCTION;

	if( m_DisableCompute )
	{
		return;
	}

	SVoxelIrradiance& Irradiance = GetIrradiance( VoxelIndex );

	if( VoxelIndex == LightVoxel )
	{
		// This voxel is the light source's voxel. Just add the values.
		// 0.5f corresponds with tangential lighting from the half Lambert equation in GetIrradianceIncidence
		const Vector4 LocalColor = Light.m_Color * 0.5f;
		Irradiance.m_Light[0] += LocalColor;
		Irradiance.m_Light[1] += LocalColor;
		Irradiance.m_Light[2] += LocalColor;
		Irradiance.m_Light[3] += LocalColor;
		Irradiance.m_Light[4] += LocalColor;
		Irradiance.m_Light[5] += LocalColor;
	}
	else
	{
		const Vector	LightDest		= GetVoxelCenter( VoxelIndex );
		const Vector	LightSource		= GetVoxelCenter( LightVoxel );
		const Vector	LightOffset		= LightDest - LightSource;

#if BUILD_DEV
		const float		LightDistSq		= LightOffset.LengthSquared();
		const float		LightRadiusSq	= Square( Light.m_Radius );
		ASSERT( LightDistSq <= LightRadiusSq );
#endif

		const Vector	LightDir		= LightOffset.GetFastNormalized();
		const Segment	LightTrace		= Segment( LightSource, LightDest );

		CollisionInfo	Info;
		Info.m_CollideWorld			= true;
		Info.m_CollideEntities		= true;
		Info.m_UserFlags			= EECF_Occlusion;
		Info.m_StopAtAnyCollision	= true;

		if( Trace( LightTrace, Info ) )
		{
			// Light is occluded.
		}
		else
		{
			// Simple linear falloff.
			const float		Falloff			= 1.0f - ( LightOffset.Length() / Light.m_Radius );
			const Vector4	FalloffColor	= Light.m_Color * Falloff;

			Irradiance.m_Light[0] += FalloffColor * GetIrradianceIncidence( LightDir, kIrradianceDirs[0] );
			Irradiance.m_Light[1] += FalloffColor * GetIrradianceIncidence( LightDir, kIrradianceDirs[1] );
			Irradiance.m_Light[2] += FalloffColor * GetIrradianceIncidence( LightDir, kIrradianceDirs[2] );
			Irradiance.m_Light[3] += FalloffColor * GetIrradianceIncidence( LightDir, kIrradianceDirs[3] );
			Irradiance.m_Light[4] += FalloffColor * GetIrradianceIncidence( LightDir, kIrradianceDirs[4] );
			Irradiance.m_Light[5] += FalloffColor * GetIrradianceIncidence( LightDir, kIrradianceDirs[5] );
		}
	}
}

inline float EldritchWorld::GetIrradianceIncidence( const Vector& LightDir, const Vector& IrradianceDir ) const
{
	// In order to get a small pool around a light that's flush against a surface,
	// I bias the dot product a little bit. Might have other side effects.

	const float Incidence = LightDir.Dot( IrradianceDir );
	const float BentIncidence = ( Incidence * 0.5f ) + 0.5f;	// Wrap light around surfaces.
	return Saturate( BentIncidence );
}

const SVoxelIrradiance& EldritchWorld::GetIrradianceAt( const Vector& Location ) const
{
	vidx_t VoxelIndex = GetIndex( Location );
	if( VoxelIndex == kInvalidVoxelIndex )
	{
		WARN;
		static const SVoxelIrradiance NullLights;
		return NullLights;
	}

	return GetIrradiance( VoxelIndex );
}

SVoxelIrradiance EldritchWorld::BlendIrradiances( const Vector& LocationA, const Vector& LocationB ) const
{
	const SVoxelIrradiance& IrradianceA = GetIrradianceAt( LocationA );
	const SVoxelIrradiance& IrradianceB = GetIrradianceAt( LocationB );

	const Vector BlendNormal = ( LocationB - LocationA ).GetNormalized();

	SVoxelIrradiance BlendedIrradiance;

	for( uint DirectionIndex = 0; DirectionIndex < 6; ++DirectionIndex )
	{
		const float CosTheta = BlendNormal.Dot( kIrradianceDirs[ DirectionIndex ] );
		const float FactorA = ( CosTheta * 0.5f ) + 0.5f;
		const float FactorB = 1.0f - FactorA;
		BlendedIrradiance.m_Light[ DirectionIndex ] = FactorA * IrradianceA.m_Light[ DirectionIndex ] + FactorB * IrradianceB.m_Light[ DirectionIndex ];
	}

	return BlendedIrradiance;
}

void EldritchWorld::GetRoomDimensions( uint& SizeX, uint& SizeY, uint& SizeZ ) const
{
	SizeX = m_RoomSizeX;
	SizeY = m_RoomSizeY;
	SizeZ = m_RoomSizeZ;
}

const EldritchWorld::SWorldDef& EldritchWorld::GetWorldDef() const
{
	DEBUGASSERT( m_CurrentWorldDef != HashedString::NullString );
	DEBUGASSERT( m_WorldDefs.Search( m_CurrentWorldDef ).IsValid() );
	return m_WorldDefs[ m_CurrentWorldDef ];
}

const EldritchWorld::SVoxelDef& EldritchWorld::GetVoxelDef( const vval_t Voxel ) const
{
	const SWorldDef& CurrentWorldDef = GetWorldDef();

	DEBUGASSERT( CurrentWorldDef.m_VoxelDefs.Search( Voxel ).IsValid() );
	return CurrentWorldDef.m_VoxelDefs[ Voxel ];
}

bool EldritchWorld::IsCollidable( const vval_t Voxel ) const
{
	return Voxel > kEmptyVoxelValue;
}

bool EldritchWorld::IsVisible( const vval_t Voxel ) const
{
	return Voxel > kEmptyVoxelValue;
}

void EldritchWorld::GatherStats()
{
	const SWorldDef& WorldDef = GetWorldDef();
	ASSERT( WorldDef.m_WorldGen );

	WorldDef.m_WorldGen->GatherStats();
}

ITexture* EldritchWorld::GetTileTexture() const
{
	IRenderer* const		pRenderer		= m_Framework->GetRenderer();
	TextureManager* const	pTextureManager	= pRenderer->GetTextureManager();
	const SimpleString&		WorldTileset	= GetWorldDef().m_Tileset;

	return pTextureManager->GetTexture( WorldTileset.CStr() );
}

void EldritchWorld::GetTileUVs( const uint TileIndex, Vector2& OutUVMin, Vector2& OutUVMax ) const
{
	OutUVMin	= GetTileUV( TileIndex );
	OutUVMax.x	= OutUVMin.x + GetTileU();
	OutUVMax.y	= OutUVMin.y + GetTileV();
}

SimpleString EldritchWorld::GetSpawnerOverride( const SimpleString& OldSpawner ) const
{
	const SWorldDef& WorldDef = GetWorldDef();
	SWorldDef::TSpawnerMap::Iterator SpawnerIter = WorldDef.m_SpawnerOverrides.Search( OldSpawner );

	if( SpawnerIter.IsValid() )
	{
		return SpawnerIter.GetValue();
	}
	else
	{
		return OldSpawner;
	}
}

int EldritchWorld::GetRoomIndex( const Vector& Location ) const
{
	// Note we simply drop the fractional part instead of rounding down.
	// Subtract 1 for the world bounds.
	const vpos_t VoxelX = static_cast<int>( Location.x - 1.0f );
	const vpos_t VoxelY = static_cast<int>( Location.y - 1.0f  );
	const vpos_t VoxelZ = static_cast<int>( Location.z - 1.0f  );

	const uint RoomX = VoxelX / m_RoomSizeX;
	const uint RoomY = VoxelY / m_RoomSizeY;
	const uint RoomZ = VoxelZ / m_RoomSizeZ;

	return RoomX + ( RoomY * m_MapSizeX ) + ( RoomZ * m_MapSizeX * m_MapSizeY );
}

void EldritchWorld::GetRoomCoords( const int RoomIndex, int& OutX, int& OutY, int& OutZ ) const
{
	OutX = RoomIndex % m_MapSizeX;
	OutY = ( RoomIndex / m_MapSizeX ) % m_MapSizeY;
	OutZ = ( ( RoomIndex / m_MapSizeX ) / m_MapSizeY ) % m_MapSizeZ;
}

Vector EldritchWorld::GetWorldDimensions() const
{
	const float DimensionX = static_cast<float>( m_NumVoxelsX );
	const float DimensionY = static_cast<float>( m_NumVoxelsY );
	const float DimensionZ = static_cast<float>( m_NumVoxelsZ );

	return Vector( DimensionX, DimensionY, DimensionZ );
}