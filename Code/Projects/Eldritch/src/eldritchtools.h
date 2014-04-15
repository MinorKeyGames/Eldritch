#ifndef ELDRITCHTOOLS_H
#define ELDRITCHTOOLS_H

#define BUILD_ELDRITCH_TOOLS BUILD_DEV && BUILD_WINDOWS_NO_SDL

#if BUILD_ELDRITCH_TOOLS

#include "eldritchworld.h"
#include "eldritchworldgen.h"
#include "array.h"
#include "vector.h"
#include "angles.h"
#include "plane.h"

class EldritchFramework;
class EldritchWorld;
class Mesh;
class IDataStream;
class Segment;

class EldritchTools
{
public:
	EldritchTools();
	~EldritchTools();

	void	InitializeFromDefinition( const SimpleString& DefinitionName );
	void	Reinitialize( const uint RoomScalarX = 1, const uint RoomScalarY = 1, const uint RoomScalarZ = 1 );
	void	ShutDown();

	bool	IsInToolMode() const;
	void	ToggleToolMode();

	void	Tick( const float DeltaTime );
	void	TickInput();
	void	TickRender();

private:
	void	TickNormalInput();
	void	TickBrushInput();
	void	TickPaletteInput();
	void	TickSpawnersInput();

	struct SPlacedSpawner
	{
		SPlacedSpawner()
		:	m_SpawnerDef( 0 )
		,	m_Orientation( ESO_North )
		{
		}

		uint	m_SpawnerDef;	// Index into m_SpawnerDefs
		uint8	m_Orientation;
	};

	enum ESubTool
	{
		EST_None,
		EST_Palette,
		EST_Spawners,
		EST_COUNT,
	};

	void	SetSubTool( const ESubTool SubTool );
	void	PrevSubTool();
	void	NextSubTool();

	void	InitializeGridMeshes();
	void	CreateGridMesh( const Array<Vector>& Corners, const uint Color );
	void	InitializeHelpMeshes();

	void	InitializePaletteMeshes();
	void	CreatePaletteMesh( const vval_t VoxelValue, const EldritchWorld::SVoxelDef& VoxelDef );

	void	InitializeSpawnerMeshes();

	void	RenderCoords();

	void	RenderBrush();
	void	ResetBrush();
	void	MaximizeBrush();
	void	AdjustBrush( const uint Axis, const int BaseChange, const int SizeChange );
	void	ExpandBrushTo( const vidx_t VoxelIndex );
	void	ShrinkBrushTo( const vidx_t VoxelIndex );
	void	FillBrush();
	void	EraseBrush();
	void	FillBrush( const bool UsePalette, const vval_t VoxelValue = 0 );

	// Helper function, trace world from camera view (which is all I expect I'll ever really need for tools)
	void	TraceVoxel( vidx_t& HitVoxel, vidx_t& FacingVoxel );
	bool	Trace( const Segment& TraceSegment, CollisionInfo& Info );
	bool	TraceBoundPlanes( const Segment& TraceSegment, CollisionInfo& Info );

	void	RefreshVoxelMeshes();
	void	RefreshVoxelMesh( const vidx_t VoxelIndex );
	void	CreateVoxelMesh( const vidx_t VoxelIndex );
	void	DestroyVoxelMesh( const vidx_t VoxelIndex );

	void	GetCoordsForVoxel( const vidx_t VoxelIndex, vpos_t& X, vpos_t& Y, vpos_t& Z );
	Vector	GetCoordsForVoxel( const vidx_t VoxelIndex );
	uint	GetVoxelIndexForCoords( vpos_t X, vpos_t Y, vpos_t Z );
	uint	SafeGetVoxelIndexForCoords( const Vector& Location );
	uint	SafeGetVoxelIndexForCoords( vpos_t X, vpos_t Y, vpos_t Z );
	vval_t	SafeGetVoxel( const vidx_t VoxelIndex );

	void	SetVoxel( const vidx_t VoxelIndex, const vval_t VoxelValue );
	vval_t	GetPaletteValue();

	float	GetYaw( const uint8 Orientation ) const;
	uint8	GetOrientation( const HashedString& Orientation ) const;

	void	TryClear( const bool WithFloor, const bool WithShell );
	void	Clear( const bool WithFloor, const bool WithShell );

	void	TryFill();
	void	Fill();

	void	TryQuickSave();
	void	TrySave();
	void	TryLoad();

	void	Save( const IDataStream& Stream );
	void	Load( const IDataStream& Stream );

	EldritchFramework*	m_Framework;
	EldritchWorld*		m_World;

	bool				m_ToolMode;

	ESubTool			m_SubTool;

	// Mirrors the values in EldritchWorld
	int					m_RoomSizeX;
	int					m_RoomSizeY;
	int					m_RoomSizeZ;
	uint				m_NumVoxels;

	uint				m_RoomScalarX;
	uint				m_RoomScalarY;
	uint				m_RoomScalarZ;

	Array<Mesh*>		m_GridMeshes;
	Array<Mesh*>		m_VoxelMeshes;

	struct SPalVox
	{
		Mesh*	m_Mesh;
		vval_t	m_Voxel;
		Vector2	m_BoxMin;
		Vector2	m_BoxMax;
	};
	Array<SPalVox>		m_PaletteVoxels;

	struct SSpawnerCategoryButton
	{
		Mesh*			m_Mesh;
		uint			m_SpawnerCategoryIndex;
		Vector2			m_BoxMin;
		Vector2			m_BoxMax;
	};

	struct SSpawnerButton
	{
		Mesh*			m_Mesh;
		uint			m_SpawnerDefIndex;
		Vector2			m_BoxMin;
		Vector2			m_BoxMax;
	};

	Array<SSpawnerCategoryButton>	m_SpawnerCategoryButtons;
	Array<SSpawnerButton>			m_SpawnerButtons;			// Should be parallel with spawner defs
	Map<uint, uint>					m_SpawnerCategoryMap;		// Maps from a spawner def index to its containing category

	bool				m_ShowAltHelp;
	Mesh*				m_HelpMesh;
	Mesh*				m_AltHelpMesh;

	Array<vval_t>		m_VoxelMap;

	Vector				m_CameraLocation;
	Angles				m_CameraOrientation;

	float				m_CameraSpeed;
	Vector				m_CameraVelocity;
	Angles				m_CameraRotationalVelocity;

	Array<int>			m_BrushBase;
	Array<int>			m_BrushSize;

	Array<Plane>		m_BoundPlanes;

	Array<vval_t>		m_Palette;

	Array<SimpleString>	m_SpawnerCategories;
	Array<SimpleString>	m_SpawnerDefs;
	uint				m_CurrentSpawnerCategoryIndex;
	uint				m_CurrentSpawnerDefIndex;

	Map<vidx_t, SPlacedSpawner>	m_Spawners;	// Map from voxel index to spawner

	SimpleString		m_CurrentMapName;
};

#endif // BUILD_ELDRITCH_TOOLS

#endif // ELDRITCHTOOLS_H