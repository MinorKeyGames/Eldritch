#include "core.h"
#include "mathcore.h" // (for Min() in array.h)
#include "eldritchnav.h"
#include "eldritchframework.h"
#include "wbcomponentarrays.h"
#include "Components/wbcompeldcollision.h"
#include "Components/wbcompelddoor.h"
#include "aabb.h"
#include "mathcore.h"
#include "segment.h"
#include "collisioninfo.h"
#include "mathfunc.h"

// NOTE: For simplification, I've removed diagonal movements (except up/down).

static const vidx_t	kInvalidVoxelIndex = 0xffffffff;
static const byte	kInvalidWorldValue = 0xff;

EldritchNav::EldritchNav()
:	m_World( NULL )
,	m_Params()
,	m_Path( NULL )
,	m_PathNodes()
,	m_WorldValues()
,	m_OpenSet()
,	m_ClosedSet()
,	m_ExpandDirections()
{
	m_World = EldritchFramework::GetInstance()->GetWorld();

	InitializePathNodes();
	InitializeExpandDirections();
	InitializeWorldValues();
}

EldritchNav::~EldritchNav()
{
}

/*static*/ EldritchNav* EldritchNav::GetInstance()
{
	EldritchWorld* const pWorld = EldritchFramework::GetInstance()->GetWorld();
	DEVASSERT( pWorld );
	DEVASSERT( pWorld->m_Nav );

	return pWorld->m_Nav;
}

EldritchNav::EPathStatus EldritchNav::FindPath( const SPathfindingParams& Params, TPath& OutPath )
{
	DEVASSERT( m_World );

	m_Params				= Params;
	m_Params.m_PathMode		= EPM_Search;
	m_Path					= &OutPath;

	return FindPath();
}

EldritchNav::EPathStatus EldritchNav::Wander( const SPathfindingParams& Params, TPath& OutPath )
{
	DEVASSERT( m_World );

	m_Params				= Params;
	m_Params.m_PathMode		= EPM_Wander;
	m_Path					= &OutPath;

	return FindPath();
}

EldritchNav::EPathStatus EldritchNav::FindPath()
{
	DEVASSERT( m_World );
	DEVASSERT( m_Path );
	DEVASSERT( m_Params.m_Height > 0 );
	DEVASSERT( m_Params.m_PathMode != EPM_None );
	DEVASSERT( m_Params.m_MotionType != EMT_None );

	InitializePathNodes();
	InitializeExpandDirections();

	// Starting node doesn't need a floor. This fixes cases where the pathing entity is standing
	// slightly off the edge of a block; its first valid move is back onto that block.
	m_Params.m_StartVoxel = m_World->GetIndex( m_Params.m_Start );
	if( !IsAccessible( m_Params.m_StartVoxel, m_Params.m_Height, false ) )
	{
		if( m_Params.m_Verbose ) { PRINTF( "No path found: Start location was inaccessible.\n" ); }
		return EPS_NoPathFound;
	}

	// Ending node doesn't need a floor, for the same reason.
	if( IsSearch() )
	{
		m_Params.m_DestinationVoxel = m_World->GetIndex( m_Params.m_Destination );
		if( !IsAccessible( m_Params.m_DestinationVoxel, m_Params.m_Height, false ) )
		{
			if( m_Params.m_Verbose ) { PRINTF( "No path found: Destination location was inaccessible.\n" ); }
			return EPS_NoPathFound;
		}
	}

	m_Path->Clear();

	if( IsSearch() )
	{
		if( m_Params.m_StartVoxel == m_Params.m_DestinationVoxel )
		{
			if( m_Params.m_Verbose ) { PRINTF( "Path found: Already at destination." ); }
			m_Path->PushBack( m_Params.m_Start );
			m_Path->PushBack( m_Params.m_Destination );
			return EPS_PathFound;
		}
	}

	m_OpenSet.Clear();
	m_ClosedSet.Clear();

	// Seed the open set with the starting position
	m_OpenSet.PushBack( m_Params.m_StartVoxel );
	SPathNode& StartNode	= GetPathNode( m_Params.m_StartVoxel );
	StartNode.m_Status		= SPathNode::ES_Open;
	StartNode.m_Backlink	= kInvalidVoxelIndex;
	StartNode.m_G			= 0.0f;
	StartNode.m_H			= IsSearch() ? GetCost( m_Params.m_StartVoxel, m_Params.m_DestinationVoxel ) : 0.0f;

	return m_World->m_Nav->DoAStarSearch();
}

void EldritchNav::InitializeWorldValues()
{
	m_WorldValues.SetDeflate( false );
	m_WorldValues.ResizeZero( m_World->m_NumVoxels );
}

void EldritchNav::InitializePathNodes()
{
	m_PathNodes.SetDeflate( false );
	m_PathNodes.ResizeZero( m_World->m_NumVoxels );
}

void EldritchNav::InitializeExpandDirections()
{
	DEVASSERT( m_World );

	const int StrideX = 1;
	const int StrideY = m_World->m_NumVoxelsX;
	const int StrideZ = m_World->m_NumVoxelsX * m_World->m_NumVoxelsY;

	// NOTE: Configurate these if needed.
	static const float kLevelCost	= 1.0f;
	static const float kUpCost		= 2.0f;
	static const float kDownCost	= 1.5f;

	m_ExpandDirections.SetDeflate( false );
	m_ExpandDirections.Clear();

	m_ExpandDirections.PushBack( SExpandDirection( StrideY, kLevelCost, false, false ) );	// North
	m_ExpandDirections.PushBack( SExpandDirection( -StrideY, kLevelCost, false, false ) );	// South
	m_ExpandDirections.PushBack( SExpandDirection( StrideX, kLevelCost, false, false ) );	// East
	m_ExpandDirections.PushBack( SExpandDirection( -StrideX, kLevelCost, false, false ) );	// West

	if( IsWalking() )
	{
		m_ExpandDirections.PushBack( SExpandDirection( StrideY - StrideZ, kUpCost, false, true ) );		// North down
		m_ExpandDirections.PushBack( SExpandDirection( -StrideY - StrideZ, kUpCost, false, true ) );	// South down
		m_ExpandDirections.PushBack( SExpandDirection( StrideX - StrideZ, kUpCost, false, true ) );		// East down
		m_ExpandDirections.PushBack( SExpandDirection( -StrideX - StrideZ, kUpCost, false, true ) );	// West down

		if( m_Params.m_CanJump )
		{
			m_ExpandDirections.PushBack( SExpandDirection( StrideY + StrideZ, kDownCost, true, false ) );	// North up
			m_ExpandDirections.PushBack( SExpandDirection( -StrideY + StrideZ, kDownCost, true, false ) );	// South up
			m_ExpandDirections.PushBack( SExpandDirection( StrideX + StrideZ, kDownCost, true, false ) );	// East up
			m_ExpandDirections.PushBack( SExpandDirection( -StrideX + StrideZ, kDownCost, true, false ) );	// West up
		}
	}
	else if( IsFlying() )
	{
		m_ExpandDirections.PushBack( SExpandDirection( StrideZ, kLevelCost, true, false ) );	// Up
		m_ExpandDirections.PushBack( SExpandDirection( -StrideZ, kLevelCost, false, true ) );	// Down
	}
}

void EldritchNav::RecursiveBuildPath( const vidx_t StepVoxelIndex, const uint NumSteps )
{
	DEVASSERT( m_Path );
	TPath& Path = *m_Path;

	const SPathNode& PathNode = GetPathNode( StepVoxelIndex );

	if( PathNode.m_Backlink == kInvalidVoxelIndex )
	{
		// Recursion terminating base case; this should be our start node. Allocate the array once.
		Path.Reserve( NumSteps + 2 );		// Add 2 for the start and end nodes
		Path.PushBack( m_Params.m_Start );
	}
	else
	{
		RecursiveBuildPath( PathNode.m_Backlink, NumSteps + 1 );
	}

	Path.PushBack( m_World->GetVoxelCenter( StepVoxelIndex ) );
}

void EldritchNav::BuildPath( const vidx_t FinalVoxelIndex )
{
	DEVASSERT( m_Path );
	TPath& Path = *m_Path;

	DEVASSERT( Path.Empty() );

	RecursiveBuildPath( FinalVoxelIndex, 1 );

	if( FinalVoxelIndex == m_Params.m_DestinationVoxel )
	{
		Path.PushBack( m_Params.m_Destination );
	}

	if( m_Params.m_Verbose )
	{
		PRINTF( "Path size: %d\n", Path.Size() );
		for( uint PathIndex = 0; PathIndex < Path.Size(); ++PathIndex )
		{
			PRINTF( "Node %d: %s\n", PathIndex, Path[ PathIndex ].GetString().CStr() );
		}
	}
}

EldritchNav::EPathStatus EldritchNav::DoAStarSearch()
{
	PROFILE_FUNCTION;

	DEVASSERT( m_OpenSet.Size() );

	uint Step = 0;
	while( m_OpenSet.Size() && Step++ < m_Params.m_MaxSteps )
	{
		const uint			MinOpenSetIndex	= GetMinOpenSetIndex();
		const vidx_t		OpenNodeIndex	= m_OpenSet[ MinOpenSetIndex ];
		const SPathNode&	OpenNode		= GetPathNode( OpenNodeIndex );

		if( ( IsSearch() && OpenNodeIndex == m_Params.m_DestinationVoxel ) ||
			( IsWander() && OpenNode.m_G >= m_Params.m_TargetDistance ) )
		{
			// We've found the destination. Build the path and return.

			if( m_Params.m_Verbose )
			{
				if( IsSearch() )
				{
					PRINTF( "Reached destination %s from %s - G: %f H: %f\n",
						m_Params.m_Destination.GetString().CStr(),
						m_World->GetVoxelCenter( OpenNode.m_Backlink ).GetString().CStr(),
						OpenNode.m_G,
						OpenNode.m_H );
				}
				else if( IsWander() )
				{
					PRINTF( "Reached destination %s from %s - G: %f\n",
						m_World->GetVoxelCenter( OpenNodeIndex ).GetString().CStr(),
						m_World->GetVoxelCenter( OpenNode.m_Backlink ).GetString().CStr(),
						OpenNode.m_G );
				}
			}

			BuildPath( OpenNodeIndex );

			return EPS_PathFound;
		}
		else
		{
			// Close the open node and expand.

			const vidx_t ClosedNodeIndex = m_ClosedSet.PushBack( OpenNodeIndex );
			SPathNode& ClosedNode = GetPathNode( ClosedNodeIndex );
			DEVASSERT( ClosedNode.m_Status == SPathNode::ES_Open );
			ClosedNode.m_Status = SPathNode::ES_Closed;
			m_OpenSet.FastRemove( MinOpenSetIndex );

			const uint NumExpandDirections = m_ExpandDirections.Size();
			for( uint DirectionIndex = 0; DirectionIndex < NumExpandDirections; ++DirectionIndex )
			{
				TryExpand( m_ExpandDirections[ DirectionIndex ], ClosedNodeIndex, ClosedNode );
			}
		}
	}

	// We didn't find a path, or ran out of steps.
	if( m_Params.m_UsePartialPath )
	{
		if( m_Params.m_Verbose ) { PRINTF( "Pathfinding finished with partial path.\n" ); }
		BuildPath( GetBestPartialDestination() );
		return EPS_PartialPathFound;
	}
	else
	{
		if( m_Params.m_Verbose ) { PRINTF( "Pathfinding finished with no path found.\n" ); }
		return EPS_NoPathFound;
	}
}

void EldritchNav::TryExpand( const EldritchNav::SExpandDirection& Direction, const vidx_t ClosedIndex, const SPathNode& ClosedNode )
{
	// Upward path requires extra clearance locally.
	// No need to recheck floor here, we've already closed this node and we're only concerned with the extra clearance.
	if( Direction.m_IsUp && !IsAccessible( ClosedIndex, m_Params.m_Height + 1, false ) )
	{
		return;
	}

	const vidx_t	ExpandVoxel		= ClosedIndex + Direction.m_Offset;
	const uint		ExpandHeight	= Direction.m_IsDown ? ( m_Params.m_Height + 1 ) : m_Params.m_Height;

	if( IsAccessible( ExpandVoxel, ExpandHeight, true ) )
	{
		Expand( ExpandVoxel, ClosedIndex, ClosedNode, Direction.m_Cost );
	}
}

void EldritchNav::Expand( const vidx_t ExpandVoxel, const vidx_t ClosedIndex, const SPathNode& ClosedNode, const float Cost )
{
	SPathNode& ExpandNode = GetPathNode( ExpandVoxel );
	if( ExpandNode.m_Status == SPathNode::ES_Closed )
	{
		// Node is already closed, ignore it.
	}
	else if( ExpandNode.m_Status == SPathNode::ES_Open )
	{
		// Node is open. Update it if we have a faster path.
		const float G = ClosedNode.m_G + Cost;
		if( G < ExpandNode.m_G )
		{
			ExpandNode.m_G = G;
			ExpandNode.m_Backlink = ClosedIndex;
		}
	}
	else
	{
		// Node is unopen. Open it.
		m_OpenSet.PushBack( ExpandVoxel );
		ExpandNode.m_Status		= SPathNode::ES_Open;
		ExpandNode.m_Backlink	= ClosedIndex;
		ExpandNode.m_G			= ClosedNode.m_G + Cost;
		// For wandering, add a small random weight so we don't bias based on order of directions.
		ExpandNode.m_H			= IsSearch() ? GetCost( ExpandVoxel, m_Params.m_DestinationVoxel ) : Math::Random( 0.0f, 0.1f );
	}
}

uint EldritchNav::GetMinOpenSetIndex() const
{
	DEVASSERT( m_OpenSet.Size() );

	bool		FoundMin		= false;
	float		MinF			= 0.0f;
	uint		MinOpenSetIndex	= 0;

	const uint	OpenSetSize		= m_OpenSet.Size();
	for( uint OpenSetIndex = 0; OpenSetIndex < OpenSetSize; ++OpenSetIndex )
	{
		const vidx_t		IterIndex	= m_OpenSet[ OpenSetIndex ];
		const SPathNode&	IterValue	= GetPathNode( IterIndex );
		const float			F			= IterValue.F();

		if( !FoundMin || F < MinF )
		{
			FoundMin		= true;
			MinOpenSetIndex	= OpenSetIndex;
			MinF			= F;
		}
	}

	return MinOpenSetIndex;
}

vidx_t EldritchNav::GetBestPartialDestination() const
{
	DEVASSERT( m_ClosedSet.Size() );

	bool	FoundBest	= false;
	float	Best		= 0.0f;
	vidx_t	BestIndex	= kInvalidVoxelIndex;

	const uint ClosedSetSize = m_ClosedSet.Size();
	for( uint ClosedSetIndex = 0; ClosedSetIndex < ClosedSetSize; ++ClosedSetIndex )
	{
		const vidx_t		ClosedVoxelIndex	= m_ClosedSet[ ClosedSetIndex ];
		const SPathNode&	ClosedNode			= GetPathNode( ClosedVoxelIndex );

		if( IsSearch() )
		{
			if( !FoundBest || ClosedNode.m_H < Best )
			{
				FoundBest	= true;
				BestIndex	= ClosedVoxelIndex;
				Best		= ClosedNode.m_H;
			}
		}
		else if( IsWander() )
		{
			if( !FoundBest || ClosedNode.m_G > Best )
			{
				FoundBest	= true;
				BestIndex	= ClosedVoxelIndex;
				Best		= ClosedNode.m_G;
			}
		}
	}

	return BestIndex;
}

float EldritchNav::GetCost( const vidx_t From, const vidx_t To ) const
{
	const Vector FromLocation	= m_World->GetVoxelCenter( From );
	const Vector ToLocation		= m_World->GetVoxelCenter( To );
	return ( ToLocation - FromLocation ).Length();
}

bool EldritchNav::IsAccessible( const vidx_t Voxel, const uint Height, const bool CheckFloor ) const
{
	if( Voxel == kInvalidVoxelIndex )
	{
		// Voxel isn't a valid index
		return false;
	}

	const uint FloorStride = m_World->m_NumVoxelsX * m_World->m_NumVoxelsY;
	if( IsWalking() && CheckFloor && Voxel >= FloorStride )
	{
		if( GetWorldValue( Voxel - FloorStride ) == EWV_None )
		{
			// Floor voxel is nonsolid.
			return false;
		}
	}

	// This means there is a door and nothing else on this spot.
	static const byte kDoorValue		= EWV_Door | 1;
	static const byte kLockedDoorValue	= EWV_LockedDoor | 1;

	for( uint HeightIndex = 0; HeightIndex < Height; ++HeightIndex )
	{
		const byte WorldValue = SafeGetWorldValue( Voxel + HeightIndex * FloorStride );
		if( WorldValue == 0 ||
			( WorldValue == kDoorValue && m_Params.m_CanOpenDoors ) ||
			( WorldValue == kLockedDoorValue && m_Params.m_CanOpenDoors && m_Params.m_CanUnlockDoors ) )
		{
			// This is ok.
		}
		else
		{
			// Voxel is collideable, a door we can't open, or invalid.
			return false;
		}
	}

	return true;
}

inline EldritchNav::SPathNode& EldritchNav::GetPathNode( const vidx_t Voxel )
{
	return m_PathNodes[ Voxel ];
}

inline const EldritchNav::SPathNode& EldritchNav::GetPathNode( const vidx_t Voxel ) const
{
	return m_PathNodes[ Voxel ];
}

inline byte EldritchNav::GetWorldValue( const vidx_t VoxelIndex ) const
{
	return m_WorldValues[ VoxelIndex ];
}

inline byte EldritchNav::SafeGetWorldValue( const vidx_t VoxelIndex ) const
{
	return m_World->IsValidIndex( VoxelIndex ) ? GetWorldValue( VoxelIndex ) : kInvalidWorldValue;
}

void EldritchNav::UpdateWorldFromAllVoxels()
{
	PROFILE_FUNCTION;

	if( m_World->m_DisableCompute )
	{
		return;
	}

	const uint NumVoxels = m_World->m_NumVoxels;
	for( uint VoxelIndex = 0; VoxelIndex < NumVoxels; ++VoxelIndex )
	{
		const vval_t VoxelValue = m_World->GetVoxel( VoxelIndex );
		if( VoxelValue == 0 )
		{
			m_WorldValues[ VoxelIndex ] &= ~EWV_World;
		}
		else
		{
			m_WorldValues[ VoxelIndex ] |= EWV_World;
		}
	}
}

void EldritchNav::UpdateWorldFromChangedVoxels( const Set<vidx_t>& ChangedVoxels )
{
	PROFILE_FUNCTION;

	if( m_World->m_DisableCompute )
	{
		return;
	}

	FOR_EACH_SET( VoxelIter, ChangedVoxels, vidx_t )
	{
		const vidx_t VoxelIndex = VoxelIter.GetValue();
		const vval_t VoxelValue = m_World->GetVoxel( VoxelIndex );
		if( VoxelValue == 0 )
		{
			m_WorldValues[ VoxelIndex ] &= ~EWV_World;
		}
		else
		{
			m_WorldValues[ VoxelIndex ] |= EWV_World;
		}
	}
}

// "Rasterize" all entity bounds into the world array.
void EldritchNav::UpdateWorldFromEntity( WBCompEldCollision* const pCollision, const bool Add )
{
	PROFILE_FUNCTION;

	DEVASSERT( pCollision );

	WBEntity* const			pEntity			= pCollision->GetEntity();
	WBCompEldDoor* const	pDoor			= GET_WBCOMP( pEntity, EldDoor );
	const bool				IsDoor			= ( pDoor != NULL );
	const bool				IsLockedDoor	= ( pDoor && pDoor->IsLocked() );

	static const byte kEntitiesMask			= EWV_Entities;
	static const byte kEntitiesNotMask		= static_cast<byte>( ~EWV_Entities );
	static const byte kDoorMask				= EWV_Door;
	static const byte kDoorNotMask			= static_cast<byte>( ~EWV_Door );
	static const byte kLockedDoorMask		= EWV_LockedDoor;
	static const byte kLockedDoorNotMask	= static_cast<byte>( ~EWV_LockedDoor );

	const int NumVoxelsX = m_World->m_NumVoxelsX;
	const int NumVoxelsY = m_World->m_NumVoxelsY;
	const int NumVoxelsZ = m_World->m_NumVoxelsZ;

	const AABB EntityBounds = pCollision->GetBounds();
	vpos_t MinVoxelX = Clamp( static_cast<vpos_t>( EntityBounds.m_Min.x ), 0, NumVoxelsX - 1 );
	vpos_t MinVoxelY = Clamp( static_cast<vpos_t>( EntityBounds.m_Min.y ), 0, NumVoxelsY - 1 );
	vpos_t MinVoxelZ = Clamp( static_cast<vpos_t>( EntityBounds.m_Min.z ), 0, NumVoxelsZ - 1 );
	vpos_t MaxVoxelX = Clamp( static_cast<vpos_t>( EntityBounds.m_Max.x ), 0, NumVoxelsX - 1 );
	vpos_t MaxVoxelY = Clamp( static_cast<vpos_t>( EntityBounds.m_Max.y ), 0, NumVoxelsY - 1 );
	vpos_t MaxVoxelZ = Clamp( static_cast<vpos_t>( EntityBounds.m_Max.z ), 0, NumVoxelsZ - 1 );

	const uint	DeltaX			= 1 + MaxVoxelX - MinVoxelX;
	const uint	DeltaY			= 1 + MaxVoxelY - MinVoxelY;
	const uint	SkipVoxelsY		= NumVoxelsX - DeltaX;
	const uint	SkipVoxelsZ		= ( NumVoxelsY - DeltaY ) * NumVoxelsX;

	vidx_t		VoxelIndex		= m_World->GetIndex( MinVoxelX, MinVoxelY, MinVoxelZ );
	DEVASSERT( m_World->IsValidIndex( VoxelIndex ) );

	for( vpos_t VoxelZ = MinVoxelZ; VoxelZ <= MaxVoxelZ; ++VoxelZ, VoxelIndex += SkipVoxelsZ )
	{
		for( vpos_t VoxelY = MinVoxelY; VoxelY <= MaxVoxelY; ++VoxelY, VoxelIndex += SkipVoxelsY )
		{
			for( vpos_t VoxelX = MinVoxelX; VoxelX <= MaxVoxelX; ++VoxelX, ++VoxelIndex )
			{
				byte& WorldValue = m_WorldValues[ VoxelIndex ];
				const byte CurrentCount = WorldValue & kEntitiesMask;

				if( Add )
				{
					DEVASSERT( CurrentCount < EWV_Entities );
					WorldValue &= kEntitiesNotMask;
					WorldValue |= ( CurrentCount + 1 );
					if( IsDoor )		{ WorldValue |= kDoorMask; }
					if( IsLockedDoor )	{ WorldValue |= kLockedDoorMask; }
				}
				else
				{
					// Remove
					DEVASSERT( CurrentCount > 0 );
					WorldValue &= kEntitiesNotMask;
					WorldValue |= ( CurrentCount - 1 );
					if( IsDoor )		{ WorldValue &= kDoorNotMask; }
					if( IsLockedDoor )	{ WorldValue &= kLockedDoorNotMask; }
				}
			}
		}
	}
}

void EldritchNav::PrintSlice( const Vector& Location )
{
	Unused( Location );

#if BUILD_DEV
	const vidx_t LocationIndex = m_World->GetIndex( Location );

	vpos_t X, Y, Z;
	m_World->GetCoords( LocationIndex, X, Y, Z );

	for( vpos_t VoxelY = m_World->m_NumVoxelsY - 1; VoxelY >= 0; --VoxelY )
	{
		for( vpos_t VoxelX = 0; VoxelX < m_World->m_NumVoxelsX; ++VoxelX )
		{
			const vidx_t VoxelIndex = m_World->GetIndex( VoxelX, VoxelY, Z );
			PRINTF( "[%2x]", GetWorldValue( VoxelIndex ) );
		}
		PRINTF( "\n" );
	}
#endif
}

void EldritchNav::SmoothPath( const Vector& Extents, EldritchNav::TPath& InOutPath )
{
	PROFILE_FUNCTION;

	DEVASSERT( m_World );
	DEVASSERT( InOutPath.Size() > 0 );	// We should have a start point at least

	// Shift locations up to entity midpoint
	const float		OffsetZ			= Extents.z - 0.5f;
	const Vector	OffsetZVector	= Vector( 0.0f, 0.0f, OffsetZ );

	for( uint AnchorIndex = InOutPath.Size() - 1; AnchorIndex > 0; )
	{
		const uint NextAnchorIndex = GetNextSmoothingNode( InOutPath[ AnchorIndex ], Extents, OffsetZVector, InOutPath, AnchorIndex );
		const uint RemoveBase = NextAnchorIndex + 1;
		const int NodesToRemove = AnchorIndex - RemoveBase;
		if( NodesToRemove > 0 )
		{
			InOutPath.Remove( RemoveBase, NodesToRemove );
		}
		DEVASSERT( AnchorIndex != NextAnchorIndex );
		AnchorIndex = NextAnchorIndex;
	}
}

// Return the index of the next node that is required for the smoothing path.
// It will be the next anchor.
uint EldritchNav::GetNextSmoothingNode( const Vector& Anchor, const Vector& Extents, const Vector& OffsetZVector, const TPath& Path, const uint PathStart ) const
{
	DEVASSERT( Path.Size() > 0 );
	DEVASSERT( PathStart < Path.Size() );
	DEVASSERT( PathStart > 0 );

	for( uint PathIndex = PathStart - 1; PathIndex > 0; --PathIndex )
	{
		if( CheckForSmoothingNode( Anchor, Path[ PathIndex - 1 ], Extents, OffsetZVector ) )
		{
			// There's a collision, the previous node is the anchor.
			return PathIndex;
		}
	}

	// There's no remaining anchors in the path.
	return 0;
}

bool EldritchNav::CheckForSmoothingNode( const Vector& Anchor, const Vector& Location, const Vector& Extents, const Vector& OffsetZVector ) const
{
	if( IsWalking() )
	{
		const float AnchorFloor		= Floor( Anchor.z );
		const float LocationFloor	= Floor( Location.z );
		if( AnchorFloor != LocationFloor )
		{
			// Any significant (block-height) discrepancy between two nodes is critical.
			return true;
		}
	}

	// Elevate sweep segment by OffsetZVector, which puts it at the entity's actual height.
	const Segment SweepSegment( Anchor + OffsetZVector, Location + OffsetZVector );

	CollisionInfo Info;
	Info.m_CollideWorld		= true;
	Info.m_CollideEntities	= true;
	Info.m_UserFlags		= EECF_Nav;

	if( m_World->Sweep( SweepSegment, Extents, Info ) )
	{
		// Collision between nodes marks a critical node.
		return true;
	}

	const Vector DownVector( 0.0f, 0.0f, -1.0f );
	Segment FloorSweepSegment( Anchor + DownVector, Location + DownVector );
	if( IsWalking() && SweepFloor( FloorSweepSegment, Extents ) )
	{
		// There is a significant hole in the floor. This is a critical node.
		return true;
	}

	// This node is not a critical smoothing node and will be smoothed over.
	return false;
}

bool EldritchNav::SweepFloor( const Segment& SweepSegment, const Vector& Extents ) const
{
	PROFILE_FUNCTION;

	const int NumVoxelsX = m_World->m_NumVoxelsX;
	const int NumVoxelsY = m_World->m_NumVoxelsY;
	const int NumVoxelsZ = m_World->m_NumVoxelsZ;

	Vector MinCorner;
	Vector MaxCorner;
	Vector::MinMax( SweepSegment.m_Point1, SweepSegment.m_Point2, MinCorner, MaxCorner );

	const AABB SegmentBounds( MinCorner, MaxCorner );
	vpos_t MinVoxelX = Clamp( static_cast<vpos_t>( SegmentBounds.m_Min.x ), 0, NumVoxelsX - 1 );
	vpos_t MinVoxelY = Clamp( static_cast<vpos_t>( SegmentBounds.m_Min.y ), 0, NumVoxelsY - 1 );
	vpos_t MinVoxelZ = Clamp( static_cast<vpos_t>( SegmentBounds.m_Min.z ), 0, NumVoxelsZ - 1 );
	vpos_t MaxVoxelX = Clamp( static_cast<vpos_t>( SegmentBounds.m_Max.x ), 0, NumVoxelsX - 1 );
	vpos_t MaxVoxelY = Clamp( static_cast<vpos_t>( SegmentBounds.m_Max.y ), 0, NumVoxelsY - 1 );
	vpos_t MaxVoxelZ = Clamp( static_cast<vpos_t>( SegmentBounds.m_Max.z ), 0, NumVoxelsZ - 1 );

	const uint	DeltaX			= 1 + MaxVoxelX - MinVoxelX;
	const uint	DeltaY			= 1 + MaxVoxelY - MinVoxelY;
	const uint	SkipVoxelsY		= NumVoxelsX - DeltaX;
	const uint	SkipVoxelsZ		= ( NumVoxelsY - DeltaY ) * NumVoxelsX;

	vidx_t		VoxelIndex		= m_World->GetIndex( MinVoxelX, MinVoxelY, MinVoxelZ );
	DEVASSERT( m_World->IsValidIndex( VoxelIndex ) );

	for( vpos_t VoxelZ = MinVoxelZ; VoxelZ <= MaxVoxelZ; ++VoxelZ, VoxelIndex += SkipVoxelsZ )
	{
		for( vpos_t VoxelY = MinVoxelY; VoxelY <= MaxVoxelY; ++VoxelY, VoxelIndex += SkipVoxelsY )
		{
			for( vpos_t VoxelX = MinVoxelX; VoxelX <= MaxVoxelX; ++VoxelX, ++VoxelIndex )
			{
				// We're expecting a floor to exist, so we only need to sweep against the boxes of empty cells.
				// NOTE: This might behave weirdly with entities that only fill a portion of a cell, but don't
				// worry about that for right now. Just treat those as if they're fully solid and walkable floors.
				if( m_WorldValues[ VoxelIndex ] > EWV_None )
				{
					continue;
				}

				const float VoxelBaseX = static_cast<float>( VoxelX );
				const float VoxelBaseY = static_cast<float>( VoxelY );
				const float VoxelBaseZ = static_cast<float>( VoxelZ );
				const Vector VoxelBase( VoxelBaseX, VoxelBaseY, VoxelBaseZ );
				const Vector VoxelEnd = VoxelBase + Vector( 1.0f, 1.0f, 1.0f );
				AABB VoxelBox = AABB( VoxelBase, VoxelEnd );

				// Contract box on sides where it is adjacent to a floor.
				{
					const uint ValueNorth	= m_WorldValues[ VoxelIndex + NumVoxelsX ];
					const uint ValueSouth	= m_WorldValues[ VoxelIndex - NumVoxelsX ];
					const uint ValueEast	= m_WorldValues[ VoxelIndex + 1 ];
					const uint ValueWest	= m_WorldValues[ VoxelIndex - 1 ];

					if( ValueNorth != EWV_None )	{ VoxelBox.m_Max.y -= Extents.y; }
					if( ValueSouth != EWV_None )	{ VoxelBox.m_Min.y += Extents.y; }
					if( ValueEast != EWV_None )		{ VoxelBox.m_Max.x -= Extents.x; }
					if( ValueWest != EWV_None )		{ VoxelBox.m_Min.x += Extents.x; }
				}

				if( SweepSegment.Intersects( VoxelBox ) )
				{
					return true;
				}
			}
		}
	}

	return false;
}