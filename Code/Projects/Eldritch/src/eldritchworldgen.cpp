#include "core.h"
#include "eldritchworldgen.h"
#include "configmanager.h"
#include "eldritchframework.h"
#include "eldritchworld.h"
#include "eldritchroom.h"
#include "packstream.h"
#include "mathfunc.h"
#include "reversehash.h"
#include "filestream.h"
#include "wbworld.h"
#include "wbentity.h"
#include "Components/wbcompeldtransform.h"
#include "Components/wbcompeldcollision.h"
#include "mathcore.h"
#include "matrix.h"
#include "wbeventmanager.h"
#include "collisioninfo.h"
#include "file.h"
#include "eldritchgame.h"

#define DEBUG_WORLD_GEN 0

static const vidx_t	kInvalidVoxelIndex	= 0xffffffff;

static const int	kMaxExits		= 1 + ( EldritchWorldGen::ERE_North | EldritchWorldGen::ERE_South | EldritchWorldGen::ERE_East | EldritchWorldGen::ERE_West | EldritchWorldGen::ERE_Up | EldritchWorldGen::ERE_Down );
static const int	kMaxTransforms	= 1 + ( EldritchWorldGen::ERT_MirrorX | EldritchWorldGen::ERT_MirrorY | EldritchWorldGen::ERT_Rotate90 );

EldritchWorldGen::EldritchWorldGen()
:	m_World( NULL )
,	m_RoomDefs()
,	m_FeatureRoomDefs()
,	m_Themes()
,	m_ThemeBias( 0.0f )
,	m_RoomMap()
,	m_ExitTransforms()
,	m_MapSizeX( 0 )
,	m_MapSizeY( 0 )
,	m_MapSizeZ( 0 )
,	m_GeneratedRooms()
,	m_Expansion_HorizontalRatio( 0.0f )
,	m_Expansion_FollowLastNodeRatio( 0.0f )
,	m_Expansion_OpenLoopsRatio( 0.0f )
,	m_Maze_UnopenRoomSet()
,	m_Maze_OpenRoomSet()
,	m_Maze_OpenRoomStack()
,	m_Maze_ClosedRoomSet()
,	m_Maze_LockedRoomSet()
,	m_SpawnResolveGroups()
,	m_PlayerStart()
,	m_IsHardMode( false )
,	m_SimmingGeneration( false )
#if BUILD_DEV
,	m_RoomStats()
,	m_CurrentRoomFilename()
#endif
{
}

EldritchWorldGen::~EldritchWorldGen()
{
}

void EldritchWorldGen::Initialize( const SimpleString& WorldGenDefinitionName )
{
	m_World = EldritchFramework::GetInstance()->GetWorld();
	ASSERT( m_World );

	m_MapSizeX = m_World->m_MapSizeX;
	m_MapSizeY = m_World->m_MapSizeY;
	m_MapSizeZ = m_World->m_MapSizeZ;
	m_NumRooms = m_MapSizeX * m_MapSizeY * m_MapSizeZ;

	ComputeExitTransforms();

	MAKEHASH( WorldGenDefinitionName );

	STATICHASH( NumFeatureRoomDefs );
	const uint NumFeatureRoomDefs = ConfigManager::GetInheritedInt( sNumFeatureRoomDefs, 0, sWorldGenDefinitionName );
	for( uint FeatureRoomDefIndex = 0; FeatureRoomDefIndex < NumFeatureRoomDefs; ++FeatureRoomDefIndex )
	{
		const SimpleString FeatureRoomDefName = ConfigManager::GetInheritedSequenceString( "FeatureRoomDef%d", FeatureRoomDefIndex, "", sWorldGenDefinitionName );
		InitializeFeatureRoomDef( FeatureRoomDefName );
	}

	STATICHASH( NumRoomDefs );
	const uint NumRoomDefs = ConfigManager::GetInheritedInt( sNumRoomDefs, 0, sWorldGenDefinitionName );
	InitializeRoomDefs( NULL, NumRoomDefs, &m_RoomMap, WorldGenDefinitionName );

	STATICHASH( NumThemes );
	const uint NumThemes = ConfigManager::GetInheritedInt( sNumThemes, 0, sWorldGenDefinitionName );
	for( uint ThemeIndex = 0; ThemeIndex < NumThemes; ++ThemeIndex )
	{
		const HashedString Theme = ConfigManager::GetInheritedSequenceHash( "Theme%d", ThemeIndex, HashedString::NullString, sWorldGenDefinitionName );
		m_Themes.PushBack( Theme );
	}

	STATICHASH( ThemeBias );
	m_ThemeBias = ConfigManager::GetInheritedFloat( sThemeBias, 0.0f, sWorldGenDefinitionName );

	STATICHASH( HorizontalRatio );
	m_Expansion_HorizontalRatio = ConfigManager::GetInheritedFloat( sHorizontalRatio, 0.0f, sWorldGenDefinitionName );

	STATICHASH( FollowLastNodeRatio );
	m_Expansion_FollowLastNodeRatio = ConfigManager::GetInheritedFloat( sFollowLastNodeRatio, 0.0f, sWorldGenDefinitionName );

	STATICHASH( OpenLoopsRatio );
	m_Expansion_OpenLoopsRatio = ConfigManager::GetInheritedFloat( sOpenLoopsRatio, 0.0f, sWorldGenDefinitionName );

	// Any resolve groups not listed default to spawning 100% of entities.
	STATICHASH( NumResolveGroups );
	const uint NumResolveGroups = ConfigManager::GetInheritedInt( sNumResolveGroups, 0, sWorldGenDefinitionName );
	for( uint ResolveGroupIndex = 0; ResolveGroupIndex < NumResolveGroups; ++ResolveGroupIndex )
	{
		const HashedString	GroupName			= ConfigManager::GetInheritedSequenceHash( "ResolveGroup%dName", ResolveGroupIndex, HashedString::NullString, sWorldGenDefinitionName );

		SSpawnResolveGroup&	ResolveGroup		= m_SpawnResolveGroups[ GroupName ];

		ResolveGroup.m_ResolveLimit				= ConfigManager::GetInheritedSequenceInt( "ResolveGroup%dLimit", ResolveGroupIndex, 0, sWorldGenDefinitionName );
		ResolveGroup.m_ResolveChance			= ConfigManager::GetInheritedSequenceFloat( "ResolveGroup%dChance", ResolveGroupIndex, 1.0f, sWorldGenDefinitionName );

		const float MinRadiusFromPlayer			= ConfigManager::GetInheritedSequenceFloat( "ResolveGroup%dMinRadiusFromPlayer", ResolveGroupIndex, 0.0f, sWorldGenDefinitionName );
		const float MinRadiusFromPlayerZ		= ConfigManager::GetInheritedSequenceFloat( "ResolveGroup%dMinRadiusFromPlayerZ", ResolveGroupIndex, 0.0f, sWorldGenDefinitionName );
		ResolveGroup.m_MinRadiusSqFromPlayer	= Square( MinRadiusFromPlayer );
		ResolveGroup.m_ScaleFromPlayerZ			= ( MinRadiusFromPlayerZ > 0.0f ) ? ( MinRadiusFromPlayer / MinRadiusFromPlayerZ ) : 0.0f;
	}
}

void EldritchWorldGen::InitializeFeatureRoomDef( const SimpleString& FeatureRoomDefName )
{
	MAKEHASH( FeatureRoomDefName );

	SFeatureRoomDef& FeatureRoomDef = m_FeatureRoomDefs.PushBack();

	STATICHASH( Required );
	FeatureRoomDef.m_Required = ConfigManager::GetBool( sRequired, false, sFeatureRoomDefName );

	STATICHASH( Chance );
	FeatureRoomDef.m_Chance = FeatureRoomDef.m_Required ? 1.0f : ConfigManager::GetFloat( sChance, 1.0f, sFeatureRoomDefName );

	STATICHASH( SliceX );
	int SliceX = ConfigManager::GetInt( sSliceX, 0, sFeatureRoomDefName );
	SliceX = ( SliceX < 0 ) ? ( m_MapSizeX + SliceX ) : SliceX;

	STATICHASH( SliceY );
	int SliceY = ConfigManager::GetInt( sSliceY, 0, sFeatureRoomDefName );
	SliceY = ( SliceY < 0 ) ? ( m_MapSizeY + SliceY ) : SliceY;

	STATICHASH( SliceZ );
	int SliceZ = ConfigManager::GetInt( sSliceZ, 0, sFeatureRoomDefName );
	SliceZ = ( SliceZ < 0 ) ? ( m_MapSizeZ + SliceZ ) : SliceZ;

	STATICHASH( SliceMinX );
	int SliceMinX = ConfigManager::GetInt( sSliceMinX, SliceX, sFeatureRoomDefName );
	SliceMinX = ( SliceMinX < 0 ) ? ( m_MapSizeX + SliceMinX ) : SliceMinX;

	STATICHASH( SliceMinY );
	int SliceMinY = ConfigManager::GetInt( sSliceMinY, SliceY, sFeatureRoomDefName );
	SliceMinY = ( SliceMinY < 0 ) ? ( m_MapSizeY + SliceMinY ) : SliceMinY;

	STATICHASH( SliceMinZ );
	int SliceMinZ = ConfigManager::GetInt( sSliceMinZ, SliceZ, sFeatureRoomDefName );
	SliceMinZ = ( SliceMinZ < 0 ) ? ( m_MapSizeZ + SliceMinZ ) : SliceMinZ;

	STATICHASH( SliceMaxX );
	int SliceMaxX = ConfigManager::GetInt( sSliceMaxX, SliceX, sFeatureRoomDefName );
	SliceMaxX = ( SliceMaxX < 0 ) ? ( m_MapSizeX + SliceMaxX ) : SliceMaxX;

	STATICHASH( SliceMaxY );
	int SliceMaxY = ConfigManager::GetInt( sSliceMaxY, SliceY, sFeatureRoomDefName );
	SliceMaxY = ( SliceMaxY < 0 ) ? ( m_MapSizeY + SliceMaxY ) : SliceMaxY;

	STATICHASH( SliceMaxZ );
	int SliceMaxZ = ConfigManager::GetInt( sSliceMaxZ, SliceZ, sFeatureRoomDefName );
	SliceMaxZ = ( SliceMaxZ < 0 ) ? ( m_MapSizeZ + SliceMaxZ ) : SliceMaxZ;

	FeatureRoomDef.m_SliceMin = GetRoomIndex( SliceMinX, SliceMinY, SliceMinZ );
	FeatureRoomDef.m_SliceMax = GetRoomIndex( SliceMaxX, SliceMaxY, SliceMaxZ );

	STATICHASH( OpenNeighbors );
	FeatureRoomDef.m_OpenNeighbors = ConfigManager::GetBool( sOpenNeighbors, false, sFeatureRoomDefName );

	STATICHASH( AllowTransforms );
	FeatureRoomDef.m_AllowTransforms = ConfigManager::GetBool( sAllowTransforms, true, sFeatureRoomDefName );

	STATICHASH( AllowUnavailable );
	FeatureRoomDef.m_AllowUnavailable = ConfigManager::GetBool( sAllowUnavailable, false, sFeatureRoomDefName );

	STATICHASH( IgnoreFit );
	FeatureRoomDef.m_IgnoreFit = ConfigManager::GetBool( sIgnoreFit, false, sFeatureRoomDefName );

	STATICHASH( NumRoomDefs );
	const uint NumRoomDefs = ConfigManager::GetInt( sNumRoomDefs, 0, sFeatureRoomDefName );
	ASSERT( NumRoomDefs );
	InitializeRoomDefs( &FeatureRoomDef.m_RoomDefs, NumRoomDefs, NULL, FeatureRoomDefName );
}

// pRoomDefs is an out array of indices into the main m_RoomDefs array.
void EldritchWorldGen::InitializeRoomDefs( Array<uint>* pRoomDefs, const uint NumRoomDefs, TRoomMap* pRoomMap, const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	m_RoomDefs.Reserve( m_RoomDefs.Size() + NumRoomDefs );

	for( uint RoomDefIndex = 0; RoomDefIndex < NumRoomDefs; ++RoomDefIndex )
	{
		SRoomDef& RoomDef		= m_RoomDefs.PushBack();
		const uint RoomIndex	= m_RoomDefs.Size() - 1;

		if( pRoomDefs )
		{
			( *pRoomDefs ).PushBack( RoomIndex );
		}

		RoomDef.m_Filename = ConfigManager::GetInheritedSequenceString( "RoomDef%d", RoomDefIndex, "", sDefinitionName );
		RoomDef.m_Theme = GetThemeFromFilename( RoomDef.m_Filename );

		MAKEHASHFROM( RoomFileName, RoomDef.m_Filename );

		STATICHASH( AllowTransforms );
		RoomDef.m_AllowTransforms = ConfigManager::GetBool( sAllowTransforms, true, sRoomFileName );

		STATICHASH( PrescribedExits );
		const bool PrescribedExits = ConfigManager::GetBool( sPrescribedExits, false, sRoomFileName );

		if( PrescribedExits )
		{
			STATICHASH( NumExits );
			const uint NumExits = ConfigManager::GetInt( sNumExits, 0, sRoomFileName );
			for( uint ExitIndex = 0; ExitIndex < NumExits; ++ExitIndex )
			{
				const HashedString ExitName = ConfigManager::GetSequenceHash( "Exit%d", ExitIndex, HashedString::NullString, sRoomFileName );
				AddExit( RoomDef.m_Exits, GetExit( ExitName ) );
			}
		}
		else
		{
			// For the common case, simplify the config file by ignoring subroom fields
			GetExitsFromFilename( RoomDef.m_Filename, RoomDef.m_Exits );
		}

		if( pRoomMap )
		{
			// Add this room def's index to the exit config map.
			const ERoomExits	ActualExits	= RoomDef.m_Exits;

			const int MaxTransforms = RoomDef.m_AllowTransforms ? kMaxTransforms : 1;
			for( int Transform = ERT_None; Transform < MaxTransforms; ++Transform )
			{
				const ERoomExits		Exits				= static_cast<ERoomExits>( GetPrecomputedTransformedExits( ActualExits, Transform ) );
				Array<SRoomDefLookup>&	RoomsFittingExit	= ( *pRoomMap )[ Exits ];

				SRoomDefLookup Lookup;
				Lookup.m_RoomIndex = RoomIndex;
				Lookup.m_Transform = static_cast<ERoomXform>( Transform );
				RoomsFittingExit.PushBack( Lookup );
			}
		}
	}
}

void EldritchWorldGen::ComputeExitTransforms()
{
	PROFILE_FUNCTION;

	const int NumExitTransforms = kMaxExits * kMaxTransforms;
	m_ExitTransforms.Resize( NumExitTransforms );

	for( int ExitTransformIndex = 0; ExitTransformIndex < NumExitTransforms; ++ExitTransformIndex )
	{
		const int Exits		= ExitTransformIndex % kMaxExits;
		const int Transform	= ExitTransformIndex / kMaxExits;
		m_ExitTransforms[ ExitTransformIndex ] = GetTransformedExits( Exits, Transform );
	}
}

int EldritchWorldGen::GetPrecomputedTransformedExits( const int Exits, const int Transform )
{
	return m_ExitTransforms[ Exits + Transform * kMaxExits ];
}

int EldritchWorldGen::GetTransformedExits( const int Exits, const int Transform )
{
	int RetVal = Exits & ( ERE_Up | ERE_Down );

	if( Transform & ERT_MirrorX )
	{
		if( Exits & ERE_East )	{ RetVal |= ERE_West; }
		if( Exits & ERE_West )	{ RetVal |= ERE_East; }
	}
	else
	{
		RetVal |= Exits & ( ERE_East | ERE_West );
	}

	if( Transform & ERT_MirrorY )
	{
		if( Exits & ERE_North )	{ RetVal |= ERE_South; }
		if( Exits & ERE_South )	{ RetVal |= ERE_North; }
	}
	else
	{
		RetVal |= Exits & ( ERE_North | ERE_South );
	}

	if( Transform & ERT_Rotate90 )
	{
		const int Temp = RetVal;
		RetVal &= ( ERE_Up | ERE_Down );

		if( Temp & ERE_East )	{ RetVal |= ERE_North; }
		if( Temp & ERE_West )	{ RetVal |= ERE_South; }
		if( Temp & ERE_North )	{ RetVal |= ERE_West; }
		if( Temp & ERE_South )	{ RetVal |= ERE_East; }
	}

	return RetVal;
}

inline void EldritchWorldGen::AddExit( ERoomExits& Exits, const ERoomExits NewExit )
{
	// |= operator isn't defined for enum types
	Exits = static_cast<ERoomExits>( Exits | NewExit );
}

inline bool IsNumeric( const char c )
{
	return c >= '0' && c <= '9';
}

// This function is not very robust. :\
// It strips the leading folders and grabs whatever precedes the first dash.
HashedString EldritchWorldGen::GetThemeFromFilename( const SimpleString& RoomFilename )
{
	const char* const	TagStart	= FileUtil::StripLeadingFolders( RoomFilename.CStr() );
	const char*			StringIter	= TagStart;

	// Find the first -.
	for( ; *StringIter != '-'; ++StringIter );
	ASSERT( StringIter != TagStart );

	// Make a copy in a small static buffer
	static const uint	kMaxTagSize	= 32;
	const uint			TagSize		= static_cast<uint>( StringIter - TagStart );
	ASSERT( TagSize < 32 );
	char LocalTag[ kMaxTagSize ];
	memcpy( LocalTag, TagStart, TagSize );
	LocalTag[ TagSize ] = '\0';

	// Hash it and return
	return HashedString( LocalTag );
}

// This function is not very robust. :\
// It works with my current naming conventions for rooms:
// Rooms/[world]/[name]-[exits](-[version]).eldritchroom
// Where [name] is anything, [exits] is the NSWEUD tag I'm parsing, and version is an optional numeric tag.
void EldritchWorldGen::GetExitsFromFilename( const SimpleString& RoomFilename, ERoomExits& Exits )
{
	const char* StringIter = RoomFilename.CStr();
	ASSERT( StringIter );

	// Find the . preceding the file extension.
	for( ; *StringIter != '.'; ++StringIter );

	// Find the previous -.
	for( ; *StringIter != '-'; --StringIter );

	// Check the next character.
	const bool HasNumericTag = IsNumeric( *( StringIter + 1 ) );

	if( HasNumericTag )
	{
		// Find the - before the numeric tag.
		for( --StringIter; *StringIter != '-'; --StringIter );
	}

	// Now we should be at the start of the exits tag.
	for( ++StringIter; *StringIter != '-' && *StringIter != '.'; ++StringIter )
	{
		AddExit( Exits, GetExit( *StringIter ) );
	}
}

EldritchWorldGen::ERoomExits EldritchWorldGen::GetExit( const HashedString& ExitName ) const
{
	STATIC_HASHED_STRING( North );
	STATIC_HASHED_STRING( N );
	STATIC_HASHED_STRING( South );
	STATIC_HASHED_STRING( S );
	STATIC_HASHED_STRING( East );
	STATIC_HASHED_STRING( E );
	STATIC_HASHED_STRING( West );
	STATIC_HASHED_STRING( W );
	STATIC_HASHED_STRING( Up );
	STATIC_HASHED_STRING( U );
	STATIC_HASHED_STRING( Down );
	STATIC_HASHED_STRING( D );

	if( ExitName == sNorth	|| ExitName == sN ) { return ERE_North; }
	if( ExitName == sSouth	|| ExitName == sS ) { return ERE_South; }
	if( ExitName == sEast	|| ExitName == sE ) { return ERE_East; }
	if( ExitName == sWest	|| ExitName == sW ) { return ERE_West; }
	if( ExitName == sUp		|| ExitName == sU ) { return ERE_Up; }
	if( ExitName == sDown	|| ExitName == sD ) { return ERE_Down; }

	WARN;
	return ERE_None;
}

EldritchWorldGen::ERoomExits EldritchWorldGen::GetExit( const char ExitChar ) const
{
	if( ExitChar == 'n' ) { return ERE_North; }
	if( ExitChar == 's' ) { return ERE_South; }
	if( ExitChar == 'e' ) { return ERE_East; }
	if( ExitChar == 'w' ) { return ERE_West; }
	if( ExitChar == 'u' ) { return ERE_Up; }
	if( ExitChar == 'd' ) { return ERE_Down; }
	if( ExitChar == 'x' ) { return ERE_None; }

	WARN;
	return ERE_None;
}

inline int EldritchWorldGen::GetRoomIndex( const int RoomX, const int RoomY, const int RoomZ )
{
	return RoomX + ( RoomY * m_MapSizeX ) + ( RoomZ * m_MapSizeX * m_MapSizeY );
}

inline void EldritchWorldGen::GetRoomCoords( const int RoomIndex, int& RoomX, int& RoomY, int& RoomZ )
{
	RoomX = RoomIndex % m_MapSizeX;
	RoomY = ( RoomIndex / m_MapSizeX ) % m_MapSizeY;
	RoomZ = ( ( RoomIndex / m_MapSizeX ) / m_MapSizeY ) % m_MapSizeZ;
}

bool EldritchWorldGen::FindFittingRooms( const ERoomExits ExitConfig, const SFeatureRoomDef& FeatureRoomDef, Array<SFittingRoom>& OutFittingRooms )
{
	PROFILE_FUNCTION;

	const bool	AllowTransforms		= FeatureRoomDef.m_AllowTransforms;
	const bool	AllowUnavailable	= FeatureRoomDef.m_AllowUnavailable;
	const bool	IgnoreFit			= FeatureRoomDef.m_IgnoreFit;
	const int	SliceMinRoomIndex	= FeatureRoomDef.m_SliceMin;
	const int	SliceMaxRoomIndex	= FeatureRoomDef.m_SliceMax;

	int MinRoomX, MinRoomY, MinRoomZ;
	GetRoomCoords( SliceMinRoomIndex, MinRoomX, MinRoomY, MinRoomZ );

	int MaxRoomX, MaxRoomY, MaxRoomZ;
	GetRoomCoords( SliceMaxRoomIndex, MaxRoomX, MaxRoomY, MaxRoomZ );

	const int NumTransforms = AllowTransforms ? kMaxTransforms : 1;

	for( int Z = MinRoomZ; Z <= MaxRoomZ; ++Z )
	{
		for( int Y = MinRoomY; Y <= MaxRoomY; ++Y )
		{
			for( int X = MinRoomX; X <= MaxRoomX; ++X )
			{
				int RoomIndex = GetRoomIndex( X, Y, Z );
				if( IgnoreFit || IsUnopenNode( RoomIndex ) )
				{
					for( int Transform = ERT_None; Transform < NumTransforms; ++Transform )
					{
						if( IgnoreFit || Fits( ExitConfig, Transform, RoomIndex, AllowUnavailable ) )
						{
							SFittingRoom FittingRoom;
							FittingRoom.m_RoomIndex = RoomIndex;
							FittingRoom.m_Transform = Transform;
							OutFittingRooms.PushBack( FittingRoom );
						}
					}
				}
			}
		}
	}

	return OutFittingRooms.Size() > 0;
}

// Try fitting ExitConfig into graph, taking into account world boundaries and adjacent closed/locked nodes.
// (In other words, every neighbor via an exit must be a valid location and either unopen or open.)
// This is only intended to be used when seeding the graph with feature rooms;
// The room specified by RoomIndex is already validated to be unopen.
bool EldritchWorldGen::Fits( const ERoomExits ExitConfig, const int Transform, const int RoomIndex, const bool AllowUnavailable )
{
	int RoomX, RoomY, RoomZ;
	GetRoomCoords( RoomIndex, RoomX, RoomY, RoomZ );

	const int TransformedExits = GetPrecomputedTransformedExits( ExitConfig, Transform );

	SNeighborIndices Neighbors;
	GetNeighborIndices( RoomIndex, Neighbors );

	const int HighX = static_cast<int>( m_MapSizeX ) - 1;
	const int HighY = static_cast<int>( m_MapSizeY ) - 1;
	const int HighZ = static_cast<int>( m_MapSizeZ ) - 1;

	// Bound conditions: our exits must have somewhere to go
	if( RoomX == 0		&& ( TransformedExits & ERE_West ) )	{ return false; }
	if( RoomX == HighX	&& ( TransformedExits & ERE_East ) )	{ return false; }
	if( RoomY == 0		&& ( TransformedExits & ERE_South ) )	{ return false; }
	if( RoomY == HighY	&& ( TransformedExits & ERE_North ) )	{ return false; }
	if( RoomZ == 0		&& ( TransformedExits & ERE_Down ) )	{ return false; }
	if( RoomZ == HighZ	&& ( TransformedExits & ERE_Up ) )		{ return false; }

	// Outgoing neighbor conditions: we can only connect to unopen or open nodes, never closed or locked
	// (This prevents feature rooms being placed adjacent to each other and forming a closed loop.)
	if( AllowUnavailable )
	{
		// Don't check outgoing neighbors.
	}
	else
	{
		if( RoomX > 0		&& ( TransformedExits & ERE_West )	&& !IsAvailableNode( Neighbors.m_WestIndex ) )	{ return false; }
		if( RoomX < HighX	&& ( TransformedExits & ERE_East )	&& !IsAvailableNode( Neighbors.m_EastIndex ) )	{ return false; }
		if( RoomY > 0		&& ( TransformedExits & ERE_South )	&& !IsAvailableNode( Neighbors.m_SouthIndex ) )	{ return false; }
		if( RoomY < HighY	&& ( TransformedExits & ERE_North )	&& !IsAvailableNode( Neighbors.m_NorthIndex ) )	{ return false; }
		if( RoomZ > 0		&& ( TransformedExits & ERE_Down )	&& !IsAvailableNode( Neighbors.m_DownIndex ) )	{ return false; }
		if( RoomZ < HighZ	&& ( TransformedExits & ERE_Up )	&& !IsAvailableNode( Neighbors.m_UpIndex ) )	{ return false; }
	}

	// Incoming neighbor conditions: neighbors that have already been seeded must fit with us.
	if( RoomX > 0		&& ( m_GeneratedRooms[ Neighbors.m_WestIndex ].m_Exits	& ERE_East )	&& ( 0 == ( TransformedExits & ERE_West ) ) )	{ return false; }
	if( RoomX < HighX	&& ( m_GeneratedRooms[ Neighbors.m_EastIndex ].m_Exits	& ERE_West )	&& ( 0 == ( TransformedExits & ERE_East ) ) )	{ return false; }
	if( RoomY > 0		&& ( m_GeneratedRooms[ Neighbors.m_SouthIndex ].m_Exits	& ERE_North )	&& ( 0 == ( TransformedExits & ERE_South ) ) )	{ return false; }
	if( RoomY < HighY	&& ( m_GeneratedRooms[ Neighbors.m_NorthIndex ].m_Exits	& ERE_South )	&& ( 0 == ( TransformedExits & ERE_North ) ) )	{ return false; }
	if( RoomZ > 0		&& ( m_GeneratedRooms[ Neighbors.m_DownIndex ].m_Exits	& ERE_Up )		&& ( 0 == ( TransformedExits & ERE_Down ) ) )	{ return false; }
	if( RoomZ < HighZ	&& ( m_GeneratedRooms[ Neighbors.m_UpIndex ].m_Exits	& ERE_Down )	&& ( 0 == ( TransformedExits & ERE_Up ) ) )		{ return false; }

	// All our exits lead to valid spaces, and all neighboring exits lead validly to this point.
	return true;
}

// Doesn't account for opening loops into closed nodes.
bool EldritchWorldGen::IsAvailableNode( const int RoomIndex )
{
	return IsUnopenNode( RoomIndex ) || IsOpenNode( RoomIndex );
}

bool EldritchWorldGen::IsUnopenNode( const int RoomIndex )
{
	return m_Maze_UnopenRoomSet.Search( RoomIndex ).IsValid();
}

bool EldritchWorldGen::IsOpenNode( const int RoomIndex )
{
	return m_Maze_OpenRoomSet.Search( RoomIndex ).IsValid();
}

bool EldritchWorldGen::IsClosedNode( const int RoomIndex )
{
	return m_Maze_ClosedRoomSet.Search( RoomIndex ).IsValid();
}

bool EldritchWorldGen::IsLockedNode( const int RoomIndex )
{
	return m_Maze_LockedRoomSet.Search( RoomIndex ).IsValid();
}

void EldritchWorldGen::InsertAndLockRoom( const SRoomDef& RoomDef, const SFittingRoom& FittingRoom, const SFeatureRoomDef& FeatureRoomDef )
{
	SGenRoom&			GenRoom				= m_GeneratedRooms[ FittingRoom.m_RoomIndex ];
	const ERoomExits	TransformedExits	= static_cast<ERoomExits>( GetPrecomputedTransformedExits( RoomDef.m_Exits, FittingRoom.m_Transform ) );

	GenRoom.m_UsePrescribedRoom		= true;
	GenRoom.m_Exits					= TransformedExits;
	GenRoom.m_PrescribedFilename	= RoomDef.m_Filename;
	GenRoom.m_PrescribedTransform	= static_cast<ERoomXform>( FittingRoom.m_Transform );

#if BUILD_DEV
	if( !m_SimmingGeneration )
	{
		PRINTF( "Inserted feature room %s\n", RoomDef.m_Filename.CStr()	);
	}
#endif

	DEBUGASSERT( m_Maze_UnopenRoomSet.Search( FittingRoom.m_RoomIndex ).IsValid() );
	DEBUGASSERT( m_Maze_LockedRoomSet.Search( FittingRoom.m_RoomIndex ).IsNull() );

	m_Maze_UnopenRoomSet.Remove( FittingRoom.m_RoomIndex );
	m_Maze_LockedRoomSet.Insert( FittingRoom.m_RoomIndex );

	SNeighborIndices Neighbors;
	GetNeighborIndices( FittingRoom.m_RoomIndex, Neighbors );

	TryExpandMaze( TransformedExits, FittingRoom.m_RoomIndex, Neighbors, FeatureRoomDef.m_OpenNeighbors );
}

void EldritchWorldGen::Generate( const bool SimGeneration /*= false*/ )
{
	PROFILE_FUNCTION;

	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();
	STATIC_HASHED_STRING( Hard );
	m_IsHardMode = Persistence.GetBool( sHard );

	m_SimmingGeneration = SimGeneration;

	InitializeMazeGen();

	AddFeatureRooms();

	MazeExpansion();
	MazeOpenLoops();

	PopulateWorld();
}

void EldritchWorldGen::AddFeatureRooms()
{
	const uint NumFeatureRoomDefs = m_FeatureRoomDefs.Size();
	for( uint FeatureRoomDefIndex = 0; FeatureRoomDefIndex < NumFeatureRoomDefs; ++FeatureRoomDefIndex )
	{
		const SFeatureRoomDef& FeatureRoomDef = m_FeatureRoomDefs[ FeatureRoomDefIndex ];

		if( !Math::RandomF( FeatureRoomDef.m_Chance ) )
		{
			ASSERT( !FeatureRoomDef.m_Required );
			continue;
		}

		ASSERT( FeatureRoomDef.m_RoomDefs.Size() );
		const uint			RoomDefIndex	= Math::ArrayRandom( FeatureRoomDef.m_RoomDefs );
		const SRoomDef&		Room			= m_RoomDefs[ RoomDefIndex ];
		const ERoomExits	RoomExits		= Room.m_Exits;

		Array<SFittingRoom> FittingRooms;
		if( FindFittingRooms( RoomExits, FeatureRoomDef, FittingRooms ) )
		{
			const SFittingRoom& FittingRoom = Math::ArrayRandom( FittingRooms );
			InsertAndLockRoom( Room, FittingRoom, FeatureRoomDef );
		}
		else
		{
			ASSERT( !FeatureRoomDef.m_Required );
		}
	}
}

void EldritchWorldGen::InitializeMazeGen()
{
	m_GeneratedRooms.SetDeflate( false );
	m_GeneratedRooms.Clear();
	m_GeneratedRooms.ResizeZero( m_NumRooms );

	m_Maze_UnopenRoomSet.Clear();
	m_Maze_OpenRoomSet.Clear();
	m_Maze_OpenRoomStack.Clear();
	m_Maze_ClosedRoomSet.Clear();
	m_Maze_LockedRoomSet.Clear();

	// Populate the graph with all rooms.
	for( uint RoomIndex = 0; RoomIndex < m_NumRooms; ++RoomIndex )
	{
		m_Maze_UnopenRoomSet.Insert( RoomIndex );
	}
}

void EldritchWorldGen::GetRoomFilenameAndTransform( const SGenRoom& GenRoom, SimpleString& OutRoomFilename, ERoomXform& OutTransform, const HashedString& ElectedTheme )
{
	if( GenRoom.m_UsePrescribedRoom )
	{
		OutRoomFilename	= GenRoom.m_PrescribedFilename;
		OutTransform	= GenRoom.m_PrescribedTransform;
	}
	else
	{
		if( ElectedTheme == HashedString::NullString )
		{
			LookupRandomRoom( GenRoom, OutRoomFilename, OutTransform );
		}
		else
		{
			LookupRandomThemeRoom( GenRoom, OutRoomFilename, OutTransform, ElectedTheme );
		}
	}
}

void EldritchWorldGen::LookupRandomRoom( const SGenRoom& GenRoom, SimpleString& OutRoomFilename, ERoomXform& OutTransform )
{
	ASSERT( m_RoomMap.Search( GenRoom.m_Exits ).IsValid() );
	const Array<SRoomDefLookup>& RoomsFittingExit = m_RoomMap[ GenRoom.m_Exits ];

	const SRoomDefLookup&	Lookup	= Math::ArrayRandom( RoomsFittingExit );
	const SRoomDef&			RoomDef	= m_RoomDefs[ Lookup.m_RoomIndex ];

	OutRoomFilename	= RoomDef.m_Filename;
	OutTransform	= Lookup.m_Transform;
}

void EldritchWorldGen::LookupRandomThemeRoom( const SGenRoom& GenRoom, SimpleString& OutRoomFilename, ERoomXform& OutTransform, const HashedString& ElectedTheme )
{
	ASSERT( m_RoomMap.Search( GenRoom.m_Exits ).IsValid() );
	const Array<SRoomDefLookup>& RoomsFittingExit = m_RoomMap[ GenRoom.m_Exits ];

	// Use a weighted random selection, adding weight to fit level theme for cohesion.
	const uint NumLookups = RoomsFittingExit.Size();
	Array<float> CumulativeWeightArray;
	CumulativeWeightArray.ResizeZero( NumLookups );

	float WeightSum = 0.0f;
	for( uint LookupIndex = 0; LookupIndex < NumLookups; ++LookupIndex )
	{
		const SRoomDefLookup&	Lookup	= RoomsFittingExit[ LookupIndex ];
		const SRoomDef&			RoomDef	= m_RoomDefs[ Lookup.m_RoomIndex ];
		const float				Weight	= ( RoomDef.m_Theme == ElectedTheme ) ? m_ThemeBias : 1.0f;

		WeightSum += Weight;
		CumulativeWeightArray[ LookupIndex ] = WeightSum;
	}

	const float RolledWeight = Math::Random( 0.0f, WeightSum );
	for( uint LookupIndex = 0; LookupIndex < NumLookups; ++LookupIndex )
	{
		if( RolledWeight <= CumulativeWeightArray[ LookupIndex ] )
		{
			const SRoomDefLookup&	Lookup	= RoomsFittingExit[ LookupIndex ];
			const SRoomDef&			RoomDef	= m_RoomDefs[ Lookup.m_RoomIndex ];

			OutRoomFilename	= RoomDef.m_Filename;
			OutTransform	= Lookup.m_Transform;

			return;
		}
	}

	// We shouldn't ever get here!
	WARN;
}

void EldritchWorldGen::CopyMazeToWorld()
{
	XTRACE_FUNCTION;

	ASSERT( m_World );

	Array<uint8> RoomExits;
	RoomExits.Resize( m_NumRooms );
	for( uint RoomIndex = 0; RoomIndex < m_NumRooms; ++RoomIndex )
	{
		const SGenRoom& GenRoom = m_GeneratedRooms[ RoomIndex ];
		RoomExits[ RoomIndex ] = static_cast<uint8>( GenRoom.m_Exits );
	}

	m_World->SetMaze( RoomExits );
}

void EldritchWorldGen::PopulateWorld()
{
	PROFILE_FUNCTION;

	CopyMazeToWorld();

	// Elect a theme for this world.
	const HashedString ElectedTheme		= m_Themes.Empty() ? HashedString::NullString : Math::ArrayRandom( m_Themes );

	for( uint RoomIndex = 0; RoomIndex < m_NumRooms; ++RoomIndex )
	{
		const SGenRoom& GenRoom = m_GeneratedRooms[ RoomIndex ];

		SimpleString	RoomFilename;
		ERoomXform		Transform;
		GetRoomFilenameAndTransform( GenRoom, RoomFilename, Transform, ElectedTheme );

#if BUILD_DEV
		m_CurrentRoomFilename = RoomFilename;
#endif

		if( m_SimmingGeneration )
		{
#if BUILD_DEV
			// Gather stats instead
			m_RoomStats[ RoomFilename ]++;
#endif
		}
		else
		{
			EldritchRoom Room;
			Room.Load( PackStream( RoomFilename.CStr() ) );

			int X, Y, Z;
			GetRoomCoords( RoomIndex, X, Y, Z );

#if DEBUG_WORLD_GEN
			if( Transform != ERT_None )
			{
				PRINTF( "Applying transform %d to room %s at %d, %d, %d\n", Transform, RoomFilename.CStr(), X, Y, Z );
			}
#endif

			SetVoxelsFromRoom( Room, Transform, X, Y, Z );
			PrepareSpawnEntitiesFromRoom( Room, Transform, X, Y, Z );
		}
	}

	if( !m_SimmingGeneration )
	{
		FilterPreparedSpawns();
		ResolveEntitySpawns();
		ValidateVoxels();

#if BUILD_DEV
		const SimpleString ElectedThemeStr	= ReverseHash::ReversedHash( ElectedTheme );
		PRINTF( "Elected theme %s with %.2f bias.\n", ElectedThemeStr.CStr(), m_ThemeBias );
#endif
	}
}

void EldritchWorldGen::ValidateVoxels() const
{
#if BUILD_DEV
	const vpos_t LoX = 1;
	const vpos_t LoY = 1;
	const vpos_t LoZ = 1;
	const vpos_t HiX = m_World->m_NumVoxelsX - 1;
	const vpos_t HiY = m_World->m_NumVoxelsY - 1;
	const vpos_t HiZ = m_World->m_NumVoxelsZ - 1;

	// Check left and right walls for any openings into the void.
	for( vpos_t Z = LoZ; Z < HiZ; ++Z )
	{
		for( vpos_t Y = LoY; Y < HiY; ++Y )
		{
			ASSERT( m_World->GetVoxel( m_World->GetIndex( LoX, Y, Z ) ) != 0 );
			ASSERT( m_World->GetVoxel( m_World->GetIndex( HiX, Y, Z ) ) != 0 );
		}
	}

	// Check front and back walls for any openings into the void.
	for( vpos_t Z = LoZ; Z < HiZ; ++Z )
	{
		for( vpos_t X = LoX; X < HiX; ++X )
		{
			ASSERT( m_World->GetVoxel( m_World->GetIndex( X, LoY, Z ) ) != 0 );
			ASSERT( m_World->GetVoxel( m_World->GetIndex( X, HiY, Z ) ) != 0 );
		}
	}

	// Check top and bottom walls for any openings into the void.
	for( vpos_t Y = LoY; Y < HiY; ++Y )
	{
		for( vpos_t X = LoX; X < HiX; ++X )
		{
			ASSERT( m_World->GetVoxel( m_World->GetIndex( X, Y, LoZ ) ) != 0 );
			ASSERT( m_World->GetVoxel( m_World->GetIndex( X, Y, HiZ ) ) != 0 );
		}
	}
#endif
}

void EldritchWorldGen::FilterPreparedSpawns()
{
	FOR_EACH_MAP( GroupIter, m_SpawnResolveGroups, HashedString, SSpawnResolveGroup )
	{
		SSpawnResolveGroup& ResolveGroup = GroupIter.GetValue();

		if( ResolveGroup.m_MinRadiusSqFromPlayer > 0.0f )
		{
			FOR_EACH_ARRAY_REVERSE( PreparedSpawnIter, ResolveGroup.m_PreparedSpawns, SPreparedSpawn )
			{
				const SPreparedSpawn&	PreparedSpawn		=	PreparedSpawnIter.GetValue();
				Vector					ToPlayer			=	PreparedSpawn.m_SpawnLocation - m_PlayerStart;
				ToPlayer.z									*=	ResolveGroup.m_ScaleFromPlayerZ;
				const float				DistanceSqToPlayer	=	ToPlayer.LengthSquared();
				if( DistanceSqToPlayer < ResolveGroup.m_MinRadiusSqFromPlayer )
				{
					ResolveGroup.m_PreparedSpawns.FastRemove( PreparedSpawnIter );
				}
			}
		}
	}
}

void EldritchWorldGen::ResolveEntitySpawns()
{
	FOR_EACH_MAP( GroupIter, m_SpawnResolveGroups, HashedString, SSpawnResolveGroup )
	{
		SSpawnResolveGroup& ResolveGroup = GroupIter.GetValue();

		const uint NumPreparedSpawns	= ResolveGroup.m_PreparedSpawns.Size();
		const uint NumDesiredSpawns		= ( ResolveGroup.m_ResolveLimit > 0 ) ? ResolveGroup.m_ResolveLimit : static_cast<uint>( ResolveGroup.m_ResolveChance * NumPreparedSpawns );
		const uint NumSpawns			= Min( NumPreparedSpawns, NumDesiredSpawns );

#if BUILD_DEV
		if( NumSpawns < NumDesiredSpawns )
		{
			const HashedString& ResolveGroupHash	= GroupIter.GetKey();
			const SimpleString	ResolveGroupName	= ReverseHash::ReversedHash( ResolveGroupHash );
			PRINTF( "Not enough spawners in world to satisfy resolve group %s.\n", ResolveGroupName.CStr() );
		}
#endif
		ASSERTDESC( NumSpawns == NumDesiredSpawns, "Not enough spawners in world to satisfy resolve group limit." );

		for( uint SpawnCount = 0; SpawnCount < NumSpawns; ++SpawnCount )
		{
			const uint				PreparedSpawnIndex	= Math::Random( ResolveGroup.m_PreparedSpawns.Size() );
			const SPreparedSpawn&	PreparedSpawn		= ResolveGroup.m_PreparedSpawns[ PreparedSpawnIndex ];
			SpawnPreparedEntity( PreparedSpawn );
			ResolveGroup.m_PreparedSpawns.FastRemove( PreparedSpawnIndex );
		}

		ResolveGroup.m_PreparedSpawns.Clear();
	}
}

void EldritchWorldGen::SpawnPreparedEntity( const SPreparedSpawn& PreparedSpawn )
{
	WBEntity* const				pEntity		= WBWorld::GetInstance()->CreateEntity( PreparedSpawn.m_EntityDef );
	DEVASSERT( pEntity );

	if( !pEntity )
	{
		return;
	}

	WBCompEldTransform* const	pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	if( !pTransform )
	{
		return;
	}

	pTransform->SetLocation( PreparedSpawn.m_SpawnLocation );
	if( PreparedSpawn.m_SpawnOrientation.Yaw != 0.0f )
	{
		pTransform->SetOrientation( PreparedSpawn.m_SpawnOrientation );
	}

#if BUILD_DEV
	STATICHASH( ShouldDebugCheckSpawnClearance );
	MAKEHASHFROM( DefinitionName, pEntity->GetName() );
	const bool ShouldDebugCheckSpawnClearance = ConfigManager::GetInheritedBool( sShouldDebugCheckSpawnClearance, true, sDefinitionName );
	if( ShouldDebugCheckSpawnClearance )
	{
		WBCompEldCollision* const	pCollision	= GET_WBCOMP( pEntity, EldCollision );
		const Vector				TestExtents	= pCollision ? pCollision->GetExtents() : Vector();
		CollisionInfo				Info;
		Info.m_CollideWorld			= true;
		Info.m_CollideEntities		= true;
		Info.m_CollidingEntity		= pEntity;
		Info.m_UserFlags			= EECF_EntityCollision;
		if( m_World->CheckClearance( PreparedSpawn.m_SpawnLocation, TestExtents, Info ) )
		{
			PRINTF( "Spawned entity %s in room %s does not have clearance!\n", pEntity->GetUniqueName().CStr(), PreparedSpawn.m_RoomFilename.CStr() );
			WARNDESC( "Spawned entity does not have clearance!" );
		}
	}
#endif

	WB_MAKE_EVENT( OnInitialOrientationSet, pEntity );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), OnInitialOrientationSet, pEntity );
}

#if BUILD_DEV
void EldritchWorldGen::ValidateSpawners()
{
	FOR_EACH_ARRAY( RoomDefIter, m_RoomDefs, SRoomDef )
	{
		const SRoomDef& RoomDef = RoomDefIter.GetValue();

		EldritchRoom Room;
		Room.Load( PackStream( RoomDef.m_Filename.CStr() ) );

		ValidateSpawnersInRoom( Room, RoomDef.m_Filename );
	}
}

void EldritchWorldGen::ValidateSpawnersInRoom( const EldritchRoom& Room, const SimpleString& RoomFilename )
{
	const Array<EldritchRoom::SSpawner>& Spawners = Room.GetSpawners();
	const uint NumSpawners = Spawners.Size();
	for( uint SpawnerIndex = 0; SpawnerIndex < NumSpawners; ++SpawnerIndex )
	{
		const EldritchRoom::SSpawner&	Spawner					= Spawners[ SpawnerIndex ];
		const SimpleString&				SpawnerDef				= Spawner.m_SpawnerDef;

		MAKEHASH( SpawnerDef );
		STATICHASH( Deprecated );
		const bool Deprecated = ConfigManager::GetBool( sDeprecated, false, sSpawnerDef );
		if( Deprecated )
		{
			PRINTF( "Using deprecated spawner def %s in room %s!\n", SpawnerDef.CStr(), RoomFilename.CStr() );
			WARNDESC( "Deprecated spawner" );
		}
	}
}
#endif

void EldritchWorldGen::PrepareSpawnEntitiesFromRoom( const EldritchRoom& Room, const ERoomXform Transform, const uint RoomX, const uint RoomY, const uint RoomZ )
{
	PROFILE_FUNCTION;

	const Array<EldritchRoom::SSpawner>& Spawners = Room.GetSpawners();
	const uint NumSpawners = Spawners.Size();
	for( uint SpawnerIndex = 0; SpawnerIndex < NumSpawners; ++SpawnerIndex )
	{
		const EldritchRoom::SSpawner&	Spawner			= Spawners[ SpawnerIndex ];
		const SimpleString&				RoomSpawnerDef	= Spawner.m_SpawnerDef;
		const SimpleString				SpawnerDef		= TryGetHardSpawner( m_World->GetSpawnerOverride( RoomSpawnerDef ) );

		MAKEHASH( SpawnerDef );

#if BUILD_DEV
		// NOTE: To fix up deprecated spawners:
		// Deprecated			= true
		// RemappedDef			= "..."
		// RemapOrientation		= true	# Optional
		// RemappedOrientation	= "S"	# Optional
		// (See also RemappedDef in eldritchtools.cpp)
		STATICHASH( Deprecated );
		const bool Deprecated = ConfigManager::GetBool( sDeprecated, false, sSpawnerDef );
		if( Deprecated )
		{
			PRINTF( "Using deprecated spawner def %s in room %s!\n", SpawnerDef.CStr(), m_CurrentRoomFilename.CStr() );
			WARNDESC( "Deprecated spawner" );
		}
#endif

		STATICHASH( Chance );
		STATICHASH( HardChance );
		const float Chance		= ConfigManager::GetFloat( sChance, 1.0f, sSpawnerDef );
		const float UseChance	= m_IsHardMode ? ConfigManager::GetFloat( sHardChance, Chance, sSpawnerDef ) : Chance;
		if( !Math::RandomF( UseChance ) )
		{
			continue;
		}

		STATICHASH( ResolveGroup );
		const HashedString ResolveGroupName = ConfigManager::GetHash( sResolveGroup, HashedString::NullString, sSpawnerDef );

		DEBUGPRINTF( "Using spawner def %s in room %s\n", SpawnerDef.CStr(), m_CurrentRoomFilename.CStr() );

		const SimpleString SubSpawnerDef = GetSubSpawnerDef( SpawnerDef );

		if( SubSpawnerDef == "" )
		{
			// It's valid for a subspawner to be empty
			continue;
		}

		MAKEHASH( SubSpawnerDef );

		const ERoomExits Direction = GetDirection( Spawner.m_Orientation );
		PrepareSpawnEntity( SubSpawnerDef, ResolveGroupName, Spawner.m_Location, Direction, Transform, RoomX, RoomY, RoomZ );
	}
}

SimpleString EldritchWorldGen::TryGetHardSpawner( const SimpleString& BaseSpawnerDef )
{
	if( m_IsHardMode )
	{
		MAKEHASH( BaseSpawnerDef );

		STATICHASH( HardSpawner );
		return ConfigManager::GetString( sHardSpawner, BaseSpawnerDef.CStr(), sBaseSpawnerDef );
	}
	else
	{
		return BaseSpawnerDef;
	}
}

SimpleString EldritchWorldGen::GetSubSpawnerDef( const SimpleString& SuperSpawnerDef )
{
	MAKEHASH( SuperSpawnerDef );

	STATICHASH( NumSubSpawners );
	const uint NumSubSpawners = ConfigManager::GetInt( sNumSubSpawners, 0, sSuperSpawnerDef );

	if( NumSubSpawners > 0 )
	{
		Array<float> CumulativeWeightArray;
		CumulativeWeightArray.ResizeZero( NumSubSpawners );

		float WeightSum = 0.0f;
		for( uint SubSpawnerIndex = 0; SubSpawnerIndex < NumSubSpawners; ++SubSpawnerIndex )
		{
			WeightSum += ConfigManager::GetSequenceFloat( "SubSpawner%dWeight", SubSpawnerIndex, 1.0f, sSuperSpawnerDef );
			CumulativeWeightArray[ SubSpawnerIndex ] = WeightSum;
		}

		const float RolledWeight = Math::Random( 0.0f, WeightSum );
		for( uint SubSpawnerIndex = 0; SubSpawnerIndex < NumSubSpawners; ++SubSpawnerIndex )
		{
			if( RolledWeight <= CumulativeWeightArray[ SubSpawnerIndex ] )
			{
				return ConfigManager::GetSequenceString( "SubSpawner%d", SubSpawnerIndex, "", sSuperSpawnerDef );
			}
		}

		// We shouldn't ever get here!
		WARN;
	}

	return SuperSpawnerDef;
}

float EldritchWorldGen::GetYaw( const int Direction )
{
	if( Direction == ERE_North )	{ return 0.0f; }
	if( Direction == ERE_South )	{ return DEGREES_TO_RADIANS( 180.0f ); }
	if( Direction == ERE_East )		{ return DEGREES_TO_RADIANS( 270.0f ); }
	if( Direction == ERE_West )		{ return DEGREES_TO_RADIANS( 90.0f ); }

	WARN;
	return 0.0f;
}

float EldritchWorldGen::GetTransformedYaw( const int Direction, const int Transform )
{
	return GetYaw( GetTransformedExits( Direction, Transform ) );
}

Vector EldritchWorldGen::GetTransformedOffset( const Vector& Offset, const int Transform )
{
	switch( Transform )
	{
	case ERT_None:										return Vector( Offset.x,	Offset.y,	Offset.z ); break;
	case ERT_MirrorX:									return Vector( -Offset.x,	Offset.y,	Offset.z ); break;
	case ERT_MirrorY:									return Vector( Offset.x,	-Offset.y,	Offset.z ); break;
	case ( ERT_MirrorX | ERT_MirrorY ):					return Vector( -Offset.x,	-Offset.y,	Offset.z ); break;
	case ERT_Rotate90:									return Vector( -Offset.y,	Offset.x,	Offset.z ); break;
	case ( ERT_MirrorX | ERT_Rotate90 ):				return Vector( -Offset.y,	-Offset.x,	Offset.z ); break;
	case ( ERT_MirrorY | ERT_Rotate90 ):				return Vector( Offset.y,	Offset.x,	Offset.z ); break;
	case ( ERT_MirrorX | ERT_MirrorY | ERT_Rotate90):	return Vector( Offset.y,	-Offset.x,	Offset.z ); break;
	}

	WARN;
	return Vector( 1.0f, 1.0f, 1.0f );
}

EldritchWorldGen::ERoomExits EldritchWorldGen::GetDirection( const uint8 Orientation ) const
{
	if( Orientation == ESO_North )	{ return ERE_North; }
	if( Orientation == ESO_South )	{ return ERE_South; }
	if( Orientation == ESO_East )	{ return ERE_East; }
	if( Orientation == ESO_West )	{ return ERE_West; }

	WARN;
	return ERE_North;
}

void EldritchWorldGen::PrepareSpawnEntity( const SimpleString& SpawnerDef, const HashedString& ResolveGroupName, const vidx_t VoxelLocation, const ERoomExits Direction, const ERoomXform Transform, const uint RoomX, const uint RoomY, const uint RoomZ )
{
	const uint RoomSize = m_World->m_RoomSizeX;

	MAKEHASH( SpawnerDef );

	STATICHASH( OffsetXYRange );
	const float OffsetXYRange = ConfigManager::GetFloat( sOffsetXYRange, 0.0f, sSpawnerDef );

	STATICHASH( OffsetX );
	const float OffsetX = ConfigManager::GetFloat( sOffsetX, Math::Random( -OffsetXYRange, OffsetXYRange ), sSpawnerDef );

	STATICHASH( OffsetY );
	const float OffsetY = ConfigManager::GetFloat( sOffsetY, Math::Random( -OffsetXYRange, OffsetXYRange ), sSpawnerDef );

	STATICHASH( OffsetZ );
	const float OffsetZ = ConfigManager::GetFloat( sOffsetZ, 0.0f, sSpawnerDef );

	STATICHASH( OffsetYawRange );
	const float OffsetYawRange = ConfigManager::GetFloat( sOffsetYawRange, 0.0f, sSpawnerDef );

	STATICHASH( OffsetYaw );
	const float OffsetYaw = DEGREES_TO_RADIANS( ConfigManager::GetFloat( sOffsetYaw, Math::Random( 0.0f, OffsetYawRange ), sSpawnerDef ) );

	// OrientOffset: If true, OffsetX/Y is transformed by orientation (yaw)
	STATICHASH( OrientOffset );
	const bool OrientOffset = ConfigManager::GetBool( sOrientOffset, true, sSpawnerDef );

	STATICHASH( Entity );
	const SimpleString EntityDef = ConfigManager::GetString( sEntity, "", sSpawnerDef );
	ASSERT( EntityDef != "" );

	const vidx_t	TransformedLocation		= ( Transform == ERT_None ) ? VoxelLocation : GetTransformedRoomVoxelIndex( VoxelLocation, Transform, RoomSize );
	const vidx_t	SpawnerWorldVoxelIndex	= GetWorldVoxelIndex( TransformedLocation, RoomX, RoomY, RoomZ );
	const Vector	SpawnOffset				= Vector( OffsetX, OffsetY, OffsetZ );
	const float		LocalYaw				= OffsetYaw + GetYaw( Direction );
	const float		GlobalYaw				= OffsetYaw + GetTransformedYaw( Direction, Transform );
	const Angles	LocalOrientation		= Angles( 0.0f, 0.0f, LocalYaw );
	const Angles	GlobalOrientation		= Angles( 0.0f, 0.0f, GlobalYaw );
	const Matrix	OrientationMatrix		= LocalOrientation.ToMatrix();
	const Vector	OrientedOffset			= OrientOffset ? ( SpawnOffset * OrientationMatrix ) : SpawnOffset;
	const Vector	TransformedOffset		= GetTransformedOffset( OrientedOffset, Transform );
	const Vector	SpawnLocation			= m_World->GetVoxelCenter( SpawnerWorldVoxelIndex ) + TransformedOffset;

#if DEBUG_WORLD_GEN
	PRINTF( "Preparing spawn:\n" );
	PRINTF( "SpawnerDef: %s\n", SpawnerDef.CStr() );
	PRINTF( "Transform: %d\n", Transform );
	PRINTF( "RoomSize: %d\n", RoomSize );
	PRINTF( "RoomX: %d\n", RoomX );
	PRINTF( "RoomY: %d\n", RoomY );
	PRINTF( "RoomZ: %d\n", RoomZ );
	PRINTF( "VoxelLocation: %d\n", VoxelLocation );
	PRINTF( "TransformedLocation: %d\n", TransformedLocation );
	PRINTF( "SpawnerWorldVoxelIndex: %d\n", SpawnerWorldVoxelIndex );
	PRINTF( "SpawnerLocation: %s\n", SpawnLocation.GetString() );
#endif

	SSpawnResolveGroup&	ResolveGroup	= m_SpawnResolveGroups[ ResolveGroupName ];
	SPreparedSpawn&		PreparedSpawn	= ResolveGroup.m_PreparedSpawns.PushBack();
	PreparedSpawn.m_EntityDef			= EntityDef;
	PreparedSpawn.m_SpawnLocation		= SpawnLocation;
	PreparedSpawn.m_SpawnOrientation	= GlobalOrientation;
#if BUILD_DEV
	PreparedSpawn.m_RoomFilename		= m_CurrentRoomFilename;
#endif

	// HACK: Identify where the player is spawning. (Hack because I usually avoiding making code be content-aware.)
	const SimpleString kPlayerDef = "Player";
	if( EntityDef == kPlayerDef )
	{
		m_PlayerStart = SpawnLocation;
	}
}

void EldritchWorldGen::GatherStats()
{
#if BUILD_DEV
	ResetStats();

	STATICHASH( EldritchWorldGen );
	STATICHASH( NumStatsIterations );
	const uint NumStatsIterations = ConfigManager::GetInt( sNumStatsIterations, 0, sEldritchWorldGen );

	for( uint GenerateIndex = 0; GenerateIndex < NumStatsIterations; ++GenerateIndex )
	{
		Generate( true );
	}

	PrintStats( FileStream( "worldgen-stats.txt", FileStream::EFM_Write ) );

	ValidateSpawners();
#endif
}

void EldritchWorldGen::ResetStats()
{
#if BUILD_DEV
	m_RoomStats.Clear();
#endif
}

void EldritchWorldGen::PrintStats( const IDataStream& Stream )
{
	Unused( Stream );

#if BUILD_DEV
	STATICHASH( EldritchWorldGen );
	STATICHASH( NumStatsIterations );
	const uint NumStatsIterations = ConfigManager::GetInt( sNumStatsIterations, 0, sEldritchWorldGen );

	Stream.PrintF( "%d iterations\n", NumStatsIterations );
	Stream.PrintF( "Room Filename\tNum Uses\tFrequency\n" );
	FOR_EACH_MAP( StatsIter, m_RoomStats, HashedString, uint )
	{
		const SimpleString ReversedFilename = ReverseHash::ReversedHash( StatsIter.GetKey() );
		const uint NumUses = StatsIter.GetValue();
		const float Frequency = static_cast<float>( NumUses ) / static_cast<float>( NumStatsIterations );
		Stream.PrintF( "%s\t%d\t%f\n", ReversedFilename.CStr(), NumUses, Frequency );
	}
#endif
}

// Convert a room voxel index into a world voxel index, given the room position.
vidx_t EldritchWorldGen::GetWorldVoxelIndex( const vidx_t RoomVoxelIndex, const uint RoomX, const uint RoomY, const uint RoomZ )
{
	const uint RoomSizeX = m_World->m_RoomSizeX;
	const uint RoomSizeY = m_World->m_RoomSizeY;
	const uint RoomSizeZ = m_World->m_RoomSizeZ;

	const vpos_t RoomVoxelX = RoomVoxelIndex % RoomSizeX;
	const vpos_t RoomVoxelY = ( RoomVoxelIndex / RoomSizeX ) % RoomSizeY;
	const vpos_t RoomVoxelZ = ( ( RoomVoxelIndex / RoomSizeX ) / RoomSizeY ) % RoomSizeZ;

	const vpos_t WorldX = 1 + RoomX * RoomSizeX + RoomVoxelX;
	const vpos_t WorldY = 1 + RoomY * RoomSizeY + RoomVoxelY;
	const vpos_t WorldZ = 1 + RoomZ * RoomSizeZ + RoomVoxelZ;

	return m_World->GetIndex( WorldX, WorldY, WorldZ );
}

void EldritchWorldGen::SetVoxelsFromRoom( const EldritchRoom& Room, const ERoomXform Transform, const uint RoomX, const uint RoomY, const uint RoomZ )
{
	PROFILE_FUNCTION;

	uint ScalarX, ScalarY, ScalarZ;
	Room.GetRoomScalars( ScalarX, ScalarY, ScalarZ );

	const uint RoomSizeX = m_World->m_RoomSizeX;
	const uint RoomSizeY = m_World->m_RoomSizeY;
	const uint RoomSizeZ = m_World->m_RoomSizeZ;

	const Array<vval_t>& RoomVoxels = Room.GetVoxelMap();
	ASSERT( RoomSizeX * ScalarX * RoomSizeY * ScalarY * RoomSizeZ * ScalarZ == RoomVoxels.Size() );

	const vpos_t	BaseX = 1 + RoomX * RoomSizeX;
	const vpos_t	BaseY = 1 + RoomY * RoomSizeY;
	const vpos_t	BaseZ = 1 + RoomZ * RoomSizeZ;
	const vpos_t	HighX = BaseX + RoomSizeX * ScalarX;
	const vpos_t	HighY = BaseY + RoomSizeY * ScalarY;
	const vpos_t	HighZ = BaseZ + RoomSizeZ * ScalarZ;

	int				RoomVoxelIndex = 0;
	int				TransformedRoomVoxelIndex = 0;

	// Transforms only work if this is true!
	ASSERT( RoomSizeX == RoomSizeY );

	for( vpos_t PosZ = BaseZ; PosZ < HighZ; ++PosZ )
	{
		for( vpos_t PosY = BaseY; PosY < HighY; ++PosY )
		{
			for( vpos_t PosX = BaseX; PosX < HighX; ++PosX, ++RoomVoxelIndex )
			{
				// Apply transform at the last minute.
				TransformedRoomVoxelIndex = ( Transform == ERT_None ) ? RoomVoxelIndex : GetUntransformedRoomVoxelIndex( RoomVoxelIndex, Transform, RoomSizeX );

				// Calling inline functions in EldritchWorld. This could cause linker errors.
				const vidx_t VoxelIndex = m_World->GetIndex( PosX, PosY, PosZ );
				ASSERT( VoxelIndex != kInvalidVoxelIndex );
				m_World->SetVoxel( VoxelIndex, RoomVoxels[ TransformedRoomVoxelIndex ] );
			}
		}
	}
}

int EldritchWorldGen::GetTransformedRoomVoxelIndex( const int RoomVoxelIndex, const int Transform, const uint RoomSize )
{
	int X = RoomVoxelIndex % RoomSize;
	int Y = ( RoomVoxelIndex / RoomSize ) % RoomSize;
	int Z = ( ( RoomVoxelIndex / RoomSize ) / RoomSize ) % RoomSize;

	if( Transform & ERT_MirrorX )
	{
		X = RoomSize - X - 1;
	}

	if( Transform & ERT_MirrorY )
	{
		Y = RoomSize - Y - 1;
	}

	if( Transform & ERT_Rotate90 )
	{
		const int T = X;
		X = RoomSize - Y - 1;
		Y = T;
	}

	return X + ( Y * RoomSize ) + ( Z * RoomSize * RoomSize );
}

// Apply transforms in reverse order, because we're essentially doing the reverse transformation
// to figure out where to sample in the original room in order to produce the transformed room.
int EldritchWorldGen::GetUntransformedRoomVoxelIndex( const int RoomVoxelIndex, const int Transform, const uint RoomSize )
{
	int X = RoomVoxelIndex % RoomSize;
	int Y = ( RoomVoxelIndex / RoomSize ) % RoomSize;
	int Z = ( ( RoomVoxelIndex / RoomSize ) / RoomSize ) % RoomSize;

	if( Transform & ERT_Rotate90 )
	{
		const int T = RoomSize - X - 1;
		X = Y;
		Y = T;
	}

	if( Transform & ERT_MirrorY )
	{
		Y = RoomSize - Y - 1;
	}

	if( Transform & ERT_MirrorX )
	{
		X = RoomSize - X - 1;
	}

	return X + ( Y * RoomSize ) + ( Z * RoomSize * RoomSize );
}

void EldritchWorldGen::GetNeighborIndices( const int RoomIndex, SNeighborIndices& OutIndices )
{
	OutIndices.m_NorthIndex	= RoomIndex + m_MapSizeX;
	OutIndices.m_SouthIndex	= RoomIndex - m_MapSizeX;
	OutIndices.m_EastIndex	= RoomIndex + 1;
	OutIndices.m_WestIndex	= RoomIndex - 1;
	OutIndices.m_UpIndex	= RoomIndex + m_MapSizeX * m_MapSizeY;
	OutIndices.m_DownIndex	= RoomIndex - m_MapSizeX * m_MapSizeY;
}

// Look for west and south exits.
void EldritchWorldGen::FindOpenLoopOpportunities( Array<SOpenLoopOpportunity>& OutOpportunities )
{
	const int HighX = static_cast<int>( m_MapSizeX - 1 );
	const int HighY = static_cast<int>( m_MapSizeY - 1 );
	const int HighZ = static_cast<int>( m_MapSizeZ - 1 );

	for( int Z = 0; Z <= HighZ; ++Z )
	{
		for( int Y = 1; Y <= HighY; ++Y )		// We're looking for west and south exits, so we can skip the south face.
		{
			for( int X = 1; X <= HighX; ++X )	// We're looking for west and south exits, so we can skip the west face.
			{
				const int RoomIndex = GetRoomIndex( X, Y, Z );

				ASSERT( IsClosedNode( RoomIndex ) || IsLockedNode( RoomIndex ) );

				// Everything should be closed or locked by now. We skip locked rooms.
				if( IsClosedNode( RoomIndex ) )
				{
					SNeighborIndices Neighbors;
					GetNeighborIndices( RoomIndex, Neighbors );

					ASSERT( IsClosedNode( Neighbors.m_WestIndex ) || IsLockedNode( Neighbors.m_WestIndex ) );
					ASSERT( IsClosedNode( Neighbors.m_SouthIndex ) || IsLockedNode( Neighbors.m_SouthIndex ) );

					const ERoomExits	RoomExits		= m_GeneratedRooms[ RoomIndex ].m_Exits;
					const bool			CanOpenWest		= ( ( RoomExits & ERE_West ) == 0 );
					const bool			CanOpenSouth	= ( ( RoomExits & ERE_South ) == 0 );

					if( CanOpenWest && IsClosedNode( Neighbors.m_WestIndex ) )
					{
						SOpenLoopOpportunity& Op	= OutOpportunities.PushBack();
						Op.m_RoomIndex				= RoomIndex;
						Op.m_Exit					= ERE_West;
						Op.m_Neighbors				= Neighbors;
					}

					if( CanOpenSouth && IsClosedNode( Neighbors.m_SouthIndex ) )
					{
						SOpenLoopOpportunity& Op	= OutOpportunities.PushBack();
						Op.m_RoomIndex				= RoomIndex;
						Op.m_Exit					= ERE_South;
						Op.m_Neighbors				= Neighbors;
					}
				}
			}
		}
	}
}

void EldritchWorldGen::MazeOpenLoops()
{
	Array<SOpenLoopOpportunity> Opportunities;
	FindOpenLoopOpportunities( Opportunities );

	const float	fNumOpportunities		= static_cast<float>( Opportunities.Size() );
	const float	fNumOpportunitiesToOpen	= m_Expansion_OpenLoopsRatio * fNumOpportunities;
	const uint	NumOpportunitiesToOpen	= static_cast<uint>( fNumOpportunitiesToOpen + 0.5f );

	for( uint OpCount = 0; OpCount < NumOpportunitiesToOpen; ++OpCount )
	{
		const uint					OpIndex	= Math::Random( Opportunities.Size() );
		const SOpenLoopOpportunity&	Op		= Opportunities[ OpIndex ];

		if( Op.m_Exit & ERE_West )
		{
			AddExit( m_GeneratedRooms[ Op.m_RoomIndex				].m_Exits, ERE_West );
			AddExit( m_GeneratedRooms[ Op.m_Neighbors.m_WestIndex	].m_Exits, ERE_East );
		}

		if( Op.m_Exit & ERE_South )
		{
			AddExit( m_GeneratedRooms[ Op.m_RoomIndex				].m_Exits, ERE_South );
			AddExit( m_GeneratedRooms[ Op.m_Neighbors.m_SouthIndex	].m_Exits, ERE_North );
		}

		Opportunities.FastRemove( OpIndex );
	}

#if DEBUG_WORLD_GEN
	PRINTF( "Opened %d loops.\n", NumOpportunitiesToOpen );
#endif

	PrintMaze();
}

void EldritchWorldGen::MazeExpansion()
{
	PROFILE_FUNCTION;

	// Small arrays for random direction selection
	Array<ERoomExits>	HorizontalExits;
	Array<ERoomExits>	VerticalExits;

	HorizontalExits.Reserve( 4 );
	HorizontalExits.SetDeflate( false );
	VerticalExits.Reserve( 2 );
	VerticalExits.SetDeflate( false );

	const int HighX = static_cast<int>( m_MapSizeX ) - 1;
	const int HighY = static_cast<int>( m_MapSizeY ) - 1;
	const int HighZ = static_cast<int>( m_MapSizeZ ) - 1;

	// Prim's algorithm, modified a la Crest to bias toward horizontal passages
	while( m_Maze_OpenRoomSet.Size() )
	{
#if BUILD_DEBUG
		//PrintMaze();
#endif

		ASSERT( m_Maze_OpenRoomStack.Size() == m_Maze_OpenRoomSet.Size() );

		// Modifying Prim's algorithm: expanding from the last expanded room produces more regular floors for me. So do that some portion of the time.
		const bool FollowLastNode		= Math::RandomF( m_Expansion_FollowLastNodeRatio );
		const int ClosedRoomIndexIndex	= FollowLastNode ? ( m_Maze_OpenRoomStack.Size() - 1 ) : Math::Random( m_Maze_OpenRoomStack.Size() );
		const int ClosedRoomIndex		= m_Maze_OpenRoomStack[ ClosedRoomIndexIndex ];

		int ClosedX, ClosedY, ClosedZ;
		GetRoomCoords( ClosedRoomIndex, ClosedX, ClosedY, ClosedZ );

		SNeighborIndices Neighbors;
		GetNeighborIndices( ClosedRoomIndex, Neighbors );

		HorizontalExits.Clear();
		VerticalExits.Clear();

		// Find all the directions we can expand in.
		// We can expand to any unopened node.
		if( ClosedX > 0		&& m_Maze_UnopenRoomSet.Search( Neighbors.m_WestIndex ).IsValid() )		{ HorizontalExits.PushBack( ERE_West ); }
		if( ClosedX < HighX	&& m_Maze_UnopenRoomSet.Search( Neighbors.m_EastIndex ).IsValid() )		{ HorizontalExits.PushBack( ERE_East ); }
		if( ClosedY > 0		&& m_Maze_UnopenRoomSet.Search( Neighbors.m_SouthIndex ).IsValid() )	{ HorizontalExits.PushBack( ERE_South ); }
		if( ClosedY < HighY	&& m_Maze_UnopenRoomSet.Search( Neighbors.m_NorthIndex ).IsValid() )	{ HorizontalExits.PushBack( ERE_North ); }
		if( ClosedZ > 0		&& m_Maze_UnopenRoomSet.Search( Neighbors.m_DownIndex ).IsValid() )		{ VerticalExits.PushBack( ERE_Down ); }
		if( ClosedZ < HighZ	&& m_Maze_UnopenRoomSet.Search( Neighbors.m_UpIndex ).IsValid() )		{ VerticalExits.PushBack( ERE_Up ); }

		// If we can't expand anywhere, this node is closed
		if( HorizontalExits.Empty() && VerticalExits.Empty() )
		{
			// I can't FastRemove because the order of this array matters for follow last node bias.
			m_Maze_OpenRoomStack.Remove( ClosedRoomIndexIndex );
			m_Maze_OpenRoomSet.Remove( ClosedRoomIndex );
			m_Maze_ClosedRoomSet.Insert( ClosedRoomIndex );
			continue;
		}

		// Select the direction we're going to expand
		bool ExpandHorizontal = false;
		ERoomExits ExpandDirection = ERE_None;
		if( HorizontalExits.Size() && VerticalExits.Size() )
		{
			// Bias toward levels with less vertical expansion
			ExpandHorizontal = Math::RandomF( m_Expansion_HorizontalRatio );
		}
		else
		{
			ExpandHorizontal = ( HorizontalExits.Size() > 0 );
		}
		ExpandDirection = ExpandHorizontal ? Math::ArrayRandom( HorizontalExits ) : Math::ArrayRandom( VerticalExits );

		TryExpandMaze( ExpandDirection, ClosedRoomIndex, Neighbors, true );
	}

	// It's rare, but some configurations of feature rooms can result in unvisited nodes in the corners of the world.
	// Close these, and they'll just be blank rooms (filled in with solid voxels).
	FOR_EACH_SET( UnopenedRoomIter, m_Maze_UnopenRoomSet, int )
	{
		const int UnopenedRoom = UnopenedRoomIter.GetValue();
		ASSERT( m_Maze_ClosedRoomSet.Search( UnopenedRoom ).IsNull() );
		m_Maze_ClosedRoomSet.Insert( UnopenedRoom );
	}
	m_Maze_UnopenRoomSet.Clear();

	PrintMaze();
}

void EldritchWorldGen::TryExpandMaze( const ERoomExits Exits, const int OpenRoomIndex, const SNeighborIndices& Neighbors, const bool OpenRoom )
{
#define TRY_EXPAND_DIR( d, o ) if( Exits & ERE_##d ) { ExpandMaze( OpenRoomIndex, Neighbors.m_##d##Index, ERE_##d, ERE_##o, OpenRoom ); }
	TRY_EXPAND_DIR( North, South );
	TRY_EXPAND_DIR( South, North );
	TRY_EXPAND_DIR( East, West );
	TRY_EXPAND_DIR( West, East );
	TRY_EXPAND_DIR( Up, Down );
	TRY_EXPAND_DIR( Down, Up );
#undef TRY_EXPAND_DIR
}

void EldritchWorldGen::ExpandMaze( const int OpenRoomIndex, const int UnopenRoomIndex, const ERoomExits Direction, const ERoomExits OppositeDirection, const bool OpenRoom )
{
	AddExit( m_GeneratedRooms[ OpenRoomIndex ].m_Exits, Direction );
	AddExit( m_GeneratedRooms[ UnopenRoomIndex ].m_Exits, OppositeDirection );

	if( !OpenRoom )
	{
		return;
	}

	if( IsOpenNode( UnopenRoomIndex ) || IsClosedNode( UnopenRoomIndex ) || IsLockedNode( UnopenRoomIndex ) )
	{
		// We can expand into already opened rooms when seeding the maze with feature rooms.
		// In that case, we need to make sure we don't re-add the room to the stack!
	}
	else
	{
		DEBUGASSERT( m_Maze_UnopenRoomSet.Search( UnopenRoomIndex ).IsValid() );
		DEBUGASSERT( m_Maze_OpenRoomSet.Search( UnopenRoomIndex ).IsNull() );
		m_Maze_UnopenRoomSet.Remove( UnopenRoomIndex );
		m_Maze_OpenRoomStack.PushBack( UnopenRoomIndex );
		m_Maze_OpenRoomSet.Insert( UnopenRoomIndex );
	}
}

void EldritchWorldGen::PrintMaze()
{
#if DEBUG_WORLD_GEN
	if( m_SimmingGeneration )
	{
		return;
	}

	for( int Z = static_cast<int>( m_MapSizeZ - 1 ); Z >= 0; --Z )		// Print top to bottom
	{
		PRINTF( "Floor %d:\n", Z );
		for( int Y = static_cast<int>( m_MapSizeY - 1 ); Y >= 0; --Y )	// Print north to south
		{
			for( int X = 0; X < static_cast<int>( m_MapSizeX ); ++X )	// Print west to east
			{
				const int RoomIndex = GetRoomIndex( X, Y, Z );
				const ERoomExits Exits = m_GeneratedRooms[ RoomIndex ].m_Exits;
				PRINTF( "[%2x]", Exits );
			}
			PRINTF( "\n" );
		}
		PRINTF( "\n" );
	}
#endif
}