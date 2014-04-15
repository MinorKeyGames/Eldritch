#ifndef ELDRITCHNAV_H
#define ELDRITCHNAV_H

#include "array.h"
#include "vector.h"
#include "eldritchworld.h"
#include "set.h"

class WBCompEldCollision;
class Segment;

class EldritchNav
{
public:
	EldritchNav();
	~EldritchNav();

	enum EPathMode
	{
		EPM_None,
		EPM_Search,
		EPM_Wander,
	};

	enum EMotionType
	{
		EMT_None,
		EMT_Walking,
		EMT_Flying,
	};

	struct SPathfindingParams
	{
		SPathfindingParams()
		:	m_Start()
		,	m_Destination()
		,	m_Height( 0 )
		,	m_MaxSteps( 0 )
		,	m_TargetDistance( 0.0f )
		,	m_CanJump( false )
		,	m_CanOpenDoors( false )
		,	m_CanUnlockDoors( false )
		,	m_UsePartialPath( false )
		,	m_Verbose( false )
		,	m_PathMode( EPM_None )
		,	m_MotionType( EMT_None )
		,	m_StartVoxel( 0 )
		,	m_DestinationVoxel( 0 )
		{
		}

		// TODO NAV: Add a maximum radius from the start which may be considered.

		// For client to set
		Vector		m_Start;
		Vector		m_Destination;
		uint		m_Height;
		uint		m_MaxSteps;
		float		m_TargetDistance;	// For wandering
		EMotionType	m_MotionType;
		bool		m_CanJump;
		bool		m_CanOpenDoors;
		bool		m_CanUnlockDoors;
		bool		m_UsePartialPath;	// If there's no path, or a path isn't found after MaxSteps, return the partial path anyway.
		bool		m_Verbose;

		// For internal use
		EPathMode	m_PathMode;
		vidx_t		m_StartVoxel;
		vidx_t		m_DestinationVoxel;
	};

	typedef Array<Vector> TPath;

	enum EPathStatus
	{
		EPS_None,
		EPS_NoPathFound,
		EPS_PartialPathFound,
		EPS_PathFound,
	};

	// Primary interface
	static EldritchNav*	GetInstance();
	EPathStatus			FindPath( const SPathfindingParams& Params, TPath& OutPath );
	EPathStatus			Wander( const SPathfindingParams& Params, TPath& OutPath );
	void				SmoothPath( const Vector& Extents, TPath& InOutPath );
	void				UpdateWorldFromEntity( WBCompEldCollision* const pCollision, const bool Add );

	// For debugging
	void				PrintSlice( const Vector& Location );

private:
	friend class EldritchWorld;

	struct SPathNode
	{
		enum EStatus
		{
			ES_Unopen,
			ES_Open,
			ES_Closed,
		};

		float F() const
		{
			return m_G + m_H;
		}

		EStatus	m_Status;
		vidx_t	m_Backlink;
		float	m_G;
		float	m_H;
	};

	struct SExpandDirection
	{
		SExpandDirection( int Offset, float Cost, bool Up, bool Down )
		:	m_Offset( Offset )
		,	m_Cost( Cost )
		,	m_IsUp( Up )
		,	m_IsDown( Down )
		{
		}

		int		m_Offset;
		float	m_Cost;
		bool	m_IsUp;		// Up requires an extra height check at the from node
		bool	m_IsDown;	// Down requires an extra height check at the to node
	};

	enum EWorldValue
	{
		EWV_None		= 0x0,	// Cell is open
		EWV_World		= 0x80,	// Cell is blocked by world collision
		EWV_Door		= 0x40,	// Cell is blocked by a door
		EWV_LockedDoor	= 0x20,	// Cell is blocked by a door
		EWV_Entities	= 0x1f, // Remaining bits are used to count collidable entities
	};

	void		InitializeWorldValues();
	void		InitializePathNodes();
	void		InitializeExpandDirections();

	bool		IsSearch() const { return m_Params.m_PathMode == EPM_Search; }
	bool		IsWander() const { return m_Params.m_PathMode == EPM_Wander; }
	bool		IsWalking() const { return m_Params.m_MotionType == EMT_Walking; }
	bool		IsFlying() const { return m_Params.m_MotionType == EMT_Flying; }

	EPathStatus	FindPath();
	EPathStatus	DoAStarSearch();
	void		TryExpand( const SExpandDirection& Direction, const vidx_t ClosedIndex, const SPathNode& ClosedNode );
	void		Expand( const vidx_t ExpandVoxel, const vidx_t ClosedIndex, const SPathNode& ClosedNode, const float Cost );
	void		BuildPath( const vidx_t FinalVoxelIndex );
	void		RecursiveBuildPath( const vidx_t StepVoxelIndex, const uint NumSteps );

	bool		IsAccessible( const vidx_t Voxel, const uint Height, const bool CheckFloor ) const;
	float		GetCost( const vidx_t From, const vidx_t To ) const;
	uint		GetMinOpenSetIndex() const;
	vidx_t		GetBestPartialDestination() const;

	uint		GetNextSmoothingNode( const Vector& Anchor, const Vector& Extents, const Vector& OffsetZVector, const TPath& Path, const uint PathStart ) const;
	bool		CheckForSmoothingNode( const Vector& Anchor, const Vector& Location, const Vector& Extents, const Vector& OffsetZVector ) const;
	bool		SweepFloor( const Segment& SweepSegment, const Vector& Extents ) const;	// sweep floor lol

	SPathNode&			GetPathNode( const vidx_t Voxel );
	const SPathNode&	GetPathNode( const vidx_t Voxel ) const;
	byte		GetWorldValue( const vidx_t VoxelIndex ) const;
	byte		SafeGetWorldValue( const vidx_t VoxelIndex ) const;

	// Interface for updating world values
	void		UpdateWorldFromAllVoxels();
	void		UpdateWorldFromChangedVoxels( const Set<vidx_t>& ChangedVoxels );

	EldritchWorld*			m_World;

	SPathfindingParams		m_Params;
	TPath*					m_Path;

	// Fixed size array, one node per voxel. Open and closed set index into this.
	Array<SPathNode>		m_PathNodes;
	Array<byte>				m_WorldValues;	// Parallel to m_PathNodes, one per voxel. Represents collision and other path-relevant flags.

	// Using arrays instead of sets so I don't waste time allocating.
	Array<vidx_t>			m_OpenSet;
	Array<vidx_t>			m_ClosedSet;

	Array<SExpandDirection>	m_ExpandDirections;
};

#endif // ELDRITCHNAV_H