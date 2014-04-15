#ifndef ELDRITCHWORLDGEN_H
#define ELDRITCHWORLDGEN_H

#include "simplestring.h"
#include "array.h"
#include "set.h"
#include "eldritchworld.h"	// For typedefs
#include "angles.h"

class EldritchWorld;
class EldritchRoom;

enum ESpawnerOrientation
{
	ESO_North,
	ESO_West,
	ESO_South,
	ESO_East,
};

class EldritchWorldGen
{
public:
	EldritchWorldGen();
	~EldritchWorldGen();

	enum ERoomExits
	{
		ERE_None		= 0x00,
		ERE_North		= 0x01,
		ERE_South		= 0x02,
		ERE_East		= 0x04,
		ERE_West		= 0x08,
		ERE_Up			= 0x10,
		ERE_Down		= 0x20,
	};

	// All 8 permutations can be obtained through the ordered application of these three transforms
	enum ERoomXform
	{
		ERT_None		= 0x0,
		ERT_MirrorX		= 0x1,
		ERT_MirrorY		= 0x2,
		ERT_Rotate90	= 0x4,
	};

	void		Initialize( const SimpleString& WorldGenDefinitionName );

	// If SimGeneration is true, the rooms are generated but not pushed to world. Stats will be gathered instead.
	void		Generate( const bool SimGeneration = false );
	void		GatherStats();

private:
	struct SRoomDef
	{
		SRoomDef()
		:	m_Filename()
		,	m_Theme()
		,	m_AllowTransforms( false )
		,	m_Exits( ERE_None )
		{
		}

		SimpleString	m_Filename;
		HashedString	m_Theme;
		bool			m_AllowTransforms;
		ERoomExits		m_Exits;
	};

	// Tells us where and how we can fit a room
	struct SFittingRoom	// lol
	{
		int		m_RoomIndex;
		int		m_Transform;
	};

	struct SNeighborIndices
	{
		int		m_NorthIndex;
		int		m_SouthIndex;
		int		m_EastIndex;
		int		m_WestIndex;
		int		m_UpIndex;
		int		m_DownIndex;
	};

	struct SGenRoom
	{
		SGenRoom()
		:	m_Exits( ERE_None )
		,	m_UsePrescribedRoom( false )
		,	m_PrescribedFilename()
		,	m_PrescribedTransform( ERT_None )
		{
		}

		ERoomExits		m_Exits;
		bool			m_UsePrescribedRoom;
		SimpleString	m_PrescribedFilename;
		ERoomXform		m_PrescribedTransform;
	};

	struct SRoomDefLookup
	{
		uint		m_RoomIndex;	// Index into m_RoomDefs
		ERoomXform	m_Transform;	// Transform to be applied to make this room fit the exit configuration we need
	};

	struct SOpenLoopOpportunity
	{
		int					m_RoomIndex;
		ERoomExits			m_Exit;
		SNeighborIndices	m_Neighbors;
	};

	struct SFeatureRoomDef
	{
		bool		m_Required;			// Is this feature room required for world gen to succeed
		float		m_Chance;			// Chance that this feature will be added to a world; if chance < 1.0, m_Required cannot be true
		int			m_SliceMin;			// Min room index of slice of world in which feature may be placed
		int			m_SliceMax;			// Max room index of slice of world in which feature may be placed
		bool		m_OpenNeighbors;	// Should the neighbors of this room be added to open room set? (Only the start room should do this.)
		bool		m_AllowTransforms;	// Can the feature room be rotated or mirrored? (Defaults to true; disable this for hand-placed layouts.)
		bool		m_AllowUnavailable;	// Can the feature room connect to "unavailable" (closed/locked) rooms? (For hand-placed layouts.)
		bool		m_IgnoreFit;		// Add the room without checking how it fits with the world boundaries or neighbors. (For hand-placed layouts.)
		Array<uint>	m_RoomDefs;			// Which rooms satisfy this feature? Contains indices into EldritchWorldGen::m_RoomDefs
	};

	struct SPreparedSpawn
	{
		SPreparedSpawn()
		:	m_EntityDef()
		,	m_SpawnLocation()
		,	m_SpawnOrientation()
#if BUILD_DEV
		,	m_RoomFilename()
#endif
		{
		}

		SimpleString	m_EntityDef;
		Vector			m_SpawnLocation;
		Angles			m_SpawnOrientation;
#if BUILD_DEV
		SimpleString	m_RoomFilename;
#endif
	};

	// Group of potential spawns to be resolved according to some metric
	struct SSpawnResolveGroup
	{
		SSpawnResolveGroup()
		:	m_ResolveLimit( 0 )
		,	m_ResolveChance( 1.0f )
		,	m_MinRadiusSqFromPlayer( 0.0f )
		,	m_ScaleFromPlayerZ( 1.0f )
		,	m_PreparedSpawns()
		{
		}

		uint					m_ResolveLimit;				// How many prepared spawns are actually instantiated? 0 (default) means use ResolveChance instead.
		float					m_ResolveChance;			// What fraction of prepared spawns are actually instantiated? Defaults to 100%.
		float					m_MinRadiusSqFromPlayer;	// How close to the player members of this group may spawn
		float					m_ScaleFromPlayerZ;			// Scalar on Z axis so we don't filter a lot of things below the player
		Array<SPreparedSpawn>	m_PreparedSpawns;
	};

	typedef Map<ERoomExits, Array<SRoomDefLookup> > TRoomMap;

	// Room def initialization stuff
	ERoomExits	GetExit( const HashedString& ExitName ) const;
	ERoomExits	GetExit( const char ExitChar ) const;
	void		AddExit( ERoomExits& Exits, const ERoomExits NewExit );
	void		GetExitsFromFilename( const SimpleString& RoomFilename, ERoomExits& Exits );

	HashedString	GetThemeFromFilename( const SimpleString& RoomFilename );

	void		InitializeFeatureRoomDef( const SimpleString& DefinitionName );
	void		InitializeRoomDefs( Array<uint>* pRoomDefs, const uint NumRoomDefs, TRoomMap* pRoomMap, const SimpleString& DefinitionName );

	void		ComputeExitTransforms();
	int			GetPrecomputedTransformedExits( const int Exits, const int Transform );
	int			GetTransformedExits( const int Exits, const int Transform );
	int			GetTransformedRoomVoxelIndex( const int RoomVoxelIndex, const int Transform, const uint RoomSize );
	int			GetUntransformedRoomVoxelIndex( const int RoomVoxelIndex, const int Transform, const uint RoomSize );

	// Generation stuff
	void		InitializeMazeGen();
	void		AddFeatureRooms();
	void		InsertAndLockRoom( const SRoomDef& RoomDef, const SFittingRoom& FittingRoom, const SFeatureRoomDef& FeatureRoomDef );
	void		MazeExpansion();
	void		MazeOpenLoops();
	void		FindOpenLoopOpportunities( Array<SOpenLoopOpportunity>& OutOpportunities );
	void		TryExpandMaze( const ERoomExits Exits, const int OpenRoomIndex, const SNeighborIndices& Neighbors, const bool OpenRoom );
	void		ExpandMaze( const int OpenRoomIndex, const int UnopenRoomIndex, const ERoomExits Direction, const ERoomExits OppositeDirection, const bool OpenRoom );
	void		PrintMaze();
	bool		FindFittingRooms( const ERoomExits ExitConfig, const SFeatureRoomDef& FeatureRoomDef, Array<SFittingRoom>& OutFittingRooms );
	bool		Fits( const ERoomExits ExitConfig, const int Transform, const int RoomIndex, const bool AllowUnavailable );
	bool		IsAvailableNode( const int RoomIndex );
	bool		IsUnopenNode( const int RoomIndex );
	bool		IsOpenNode( const int RoomIndex );
	bool		IsClosedNode( const int RoomIndex );
	bool		IsLockedNode( const int RoomIndex );
	void		GetNeighborIndices( const int RoomIndex, SNeighborIndices& OutIndices );

	void		GetRoomFilenameAndTransform( const SGenRoom& GenRoom, SimpleString& OutRoomFilename, ERoomXform& OutTransform, const HashedString& ElectedTheme );
	void		LookupRandomRoom( const SGenRoom& GenRoom, SimpleString& OutRoomFilename, ERoomXform& OutTransform );
	void		LookupRandomThemeRoom( const SGenRoom& GenRoom, SimpleString& OutRoomFilename, ERoomXform& OutTransform, const HashedString& ElectedTheme );

	void			PopulateWorld();
	void			CopyMazeToWorld();
	void			SetVoxelsFromRoom( const EldritchRoom& Room, const ERoomXform Transform, const uint RoomX, const uint RoomY, const uint RoomZ );
	void			PrepareSpawnEntitiesFromRoom( const EldritchRoom& Room, const ERoomXform Transform, const uint RoomX, const uint RoomY, const uint RoomZ );
	void			PrepareSpawnEntity( const SimpleString& SpawnerDef, const HashedString& ResolveGroupName, const vidx_t VoxelLocation, const ERoomExits Direction, const ERoomXform Transform, const uint RoomX, const uint RoomY, const uint RoomZ );
	SimpleString	TryGetHardSpawner( const SimpleString& BaseSpawnerDef );
	SimpleString	GetSubSpawnerDef( const SimpleString& SuperSpawnerDef );
	float			GetYaw( const int Direction );
	float			GetTransformedYaw( const int Direction, const int Transform );
	Vector			GetTransformedOffset( const Vector& Offset, const int Transform );
	ERoomExits		GetDirection( const uint8 Orientation ) const;
	void			FilterPreparedSpawns();
	void			ResolveEntitySpawns();
	void			SpawnPreparedEntity( const SPreparedSpawn& PreparedSpawn );
	void			ValidateVoxels() const;
	void			FindPlayerStart();

#if BUILD_DEV
	void			ValidateSpawners();
	void			ValidateSpawnersInRoom( const EldritchRoom& Room, const SimpleString& RoomFilename );
#endif

	// Coordinate conversion
	int			GetRoomIndex( const int RoomX, const int RoomY, const int RoomZ );
	void		GetRoomCoords( const int RoomIndex, int& RoomX, int& RoomY, int& RoomZ );

	vidx_t		GetWorldVoxelIndex( const vidx_t RoomVoxelIndex, const uint RoomX, const uint RoomY, const uint RoomZ );

	void		ResetStats();
	void		PrintStats( const IDataStream& Stream );

	EldritchWorld*	m_World;

	Array<SRoomDef>			m_RoomDefs;			// *All* room defs, even those also tracked as start rooms, etc.
	Array<SFeatureRoomDef>	m_FeatureRoomDefs;
	Array<HashedString>		m_Themes;
	float					m_ThemeBias;

	// This maps from exit configurations to an array of all defs which fit (including with transforms.)
	// This does *not* include "special feature" rooms, or those comprising multiple rooms.
	// TODO: Track those somewhere else as needed.
	TRoomMap		m_RoomMap;

	// Precomputed array of exit configurations by each transform.
	// Ordered by exits then transforms.
	Array<int>		m_ExitTransforms;

	// Mirror of world's properties
	uint			m_MapSizeX;
	uint			m_MapSizeY;
	uint			m_MapSizeZ;
	uint			m_NumRooms;

	Array<SGenRoom>	m_GeneratedRooms;

	float			m_Expansion_HorizontalRatio;
	float			m_Expansion_FollowLastNodeRatio;
	float			m_Expansion_OpenLoopsRatio;

	// TODO: Move maze expansion containers in here.
	// Nodes can be in four states: unopened, opened, closed, and locked.
	// Closed means closed for maze expansion but can get new edges for loop creation.
	// Locked means nothing can touch that node ever again (for feature rooms, etc.).
	// I was using an array for open rooms so I could bias toward the last one, and because I didn't need to search on it.
	Set<int>		m_Maze_UnopenRoomSet;
	Set<int>		m_Maze_OpenRoomSet;
	Array<int>		m_Maze_OpenRoomStack;	// Mirrors OpenRoomSet, but allows random access.
	Set<int>		m_Maze_ClosedRoomSet;
	Set<int>		m_Maze_LockedRoomSet;

	Map<HashedString, SSpawnResolveGroup>	m_SpawnResolveGroups;

	Vector					m_PlayerStart;

	bool					m_IsHardMode;	// Shadow the traveling player state, because we can't query player during generation obviously
	bool					m_SimmingGeneration;
#if BUILD_DEV
	Map<HashedString, uint>	m_RoomStats;
	SimpleString			m_CurrentRoomFilename;
#endif
};

#endif // ELDRITCHWORLDGEN_H