#include "core.h"
#include "eldritchtools.h"

#if BUILD_ELDRITCH_TOOLS

#include "eldritchframework.h"
#include "irenderer.h"
#include "eldritchworld.h"
#include "meshfactory.h"
#include "shadermanager.h"
#include "texturemanager.h"
#include "ivertexbuffer.h"
#include "ivertexdeclaration.h"
#include "iindexbuffer.h"
#include "mesh.h"
#include "mouse.h"
#include "keyboard.h"
#include "mathcore.h"
#include "mathfunc.h"
#include "vector2.h"
#include "idatastream.h"
#include "filestream.h"
#include "file.h"
#include "windowwrapper.h"
#include "fontmanager.h"
#include "3d.h"
#include "segment.h"
#include "collisioninfo.h"
#include "eldritchroom.h"
#include "configmanager.h"
#include "view.h"
#include "Common/uimanagercommon.h"
#include "uiscreen.h"
#include "uistack.h"
#include "irendertarget.h"
#include "eldritchtargetmanager.h"

#include <Windows.h>	// For MessageBox

static const vidx_t	kInvalidVoxelIndex	= 0xffffffff;
static const vval_t	kEmptyVoxelValue	= 0x00;
static const vval_t	kInvalidVoxelValue	= 0xff;

EldritchTools::EldritchTools()
:	m_Framework( NULL )
,	m_World( NULL )
,	m_ToolMode( false )
,	m_SubTool( EST_None )
,	m_RoomSizeX( 0 )
,	m_RoomSizeY( 0 )
,	m_RoomSizeZ( 0 )
,	m_NumVoxels( 0 )
,	m_RoomScalarX( 0 )
,	m_RoomScalarY( 0 )
,	m_RoomScalarZ( 0 )
,	m_GridMeshes()
,	m_VoxelMeshes()
,	m_PaletteVoxels()
,	m_SpawnerCategoryButtons()
,	m_SpawnerButtons()
,	m_SpawnerCategoryMap()
,	m_ShowAltHelp( false )
,	m_HelpMesh( NULL )
,	m_AltHelpMesh( NULL )
,	m_VoxelMap()
,	m_CameraLocation()
,	m_CameraOrientation()
,	m_CameraSpeed( 0.0f )
,	m_CameraVelocity()
,	m_CameraRotationalVelocity()
,	m_BrushBase()
,	m_BrushSize()
,	m_BoundPlanes()
,	m_Palette()
,	m_SpawnerCategories()
,	m_SpawnerDefs()
,	m_CurrentSpawnerCategoryIndex( 0 )
,	m_CurrentSpawnerDefIndex( 0 )
,	m_Spawners()
,	m_CurrentMapName()
{
}

EldritchTools::~EldritchTools()
{
	ShutDown();
}

void EldritchTools::ShutDown()
{
	const uint NumGridMeshes = m_GridMeshes.Size();
	for( uint GridMeshIndex = 0; GridMeshIndex < NumGridMeshes; ++GridMeshIndex )
	{
		SafeDelete( m_GridMeshes[ GridMeshIndex ] );
	}
	m_GridMeshes.Clear();

	const uint NumVoxelMeshes = m_VoxelMeshes.Size();
	for( uint VoxelMeshIndex = 0; VoxelMeshIndex < NumVoxelMeshes; ++VoxelMeshIndex )
	{
		SafeDelete( m_VoxelMeshes[ VoxelMeshIndex ] );
	}
	m_VoxelMeshes.Clear();

	const uint NumPaletteVoxels = m_PaletteVoxels.Size();
	for( uint PaletteVoxelIndex = 0; PaletteVoxelIndex < NumPaletteVoxels; ++PaletteVoxelIndex )
	{
		SPalVox& PaletteVoxel = m_PaletteVoxels[ PaletteVoxelIndex ];
		SafeDelete( PaletteVoxel.m_Mesh );
	}
	m_PaletteVoxels.Clear();

	const uint NumSpawnerCategoryButtons = m_SpawnerCategoryButtons.Size();
	for( uint SpawnerCategoryButtonIndex = 0; SpawnerCategoryButtonIndex < NumSpawnerCategoryButtons; ++SpawnerCategoryButtonIndex )
	{
		SSpawnerCategoryButton& SpawnerCategoryButton = m_SpawnerCategoryButtons[ SpawnerCategoryButtonIndex ];
		SafeDelete( SpawnerCategoryButton.m_Mesh );
	}
	m_SpawnerCategoryButtons.Clear();

	const uint NumSpawnerButtons = m_SpawnerButtons.Size();
	for( uint SpawnerButtonIndex = 0; SpawnerButtonIndex < NumSpawnerButtons; ++SpawnerButtonIndex )
	{
		SSpawnerButton& SpawnerButton = m_SpawnerButtons[ SpawnerButtonIndex ];
		SafeDelete( SpawnerButton.m_Mesh );
	}
	m_SpawnerButtons.Clear();

	SafeDelete( m_HelpMesh );
	SafeDelete( m_AltHelpMesh );
}

void EldritchTools::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( NumSpawnerCategories );
	const uint NumSpawnerCategories = ConfigManager::GetInt( sNumSpawnerCategories, 0, sDefinitionName );

	for( uint CategoryIndex = 0; CategoryIndex < NumSpawnerCategories; ++CategoryIndex )
	{
		const SimpleString Category = ConfigManager::GetSequenceString( "SpawnerCategory%d", CategoryIndex, "", sDefinitionName );
		m_SpawnerCategories.PushBack( Category );

		MAKEHASH( Category );

		STATICHASH( NumSpawnerDefs );
		const uint NumSpawnerDefs = ConfigManager::GetInt( sNumSpawnerDefs, 0, sCategory );

		for( uint SpawnerDefCategoryIndex = 0; SpawnerDefCategoryIndex < NumSpawnerDefs; ++SpawnerDefCategoryIndex )
		{
			const uint SpawnerDefIndex = m_SpawnerDefs.Size();
			const SimpleString SpawnerDef = ConfigManager::GetSequenceString( "SpawnerDef%d", SpawnerDefCategoryIndex, "", sCategory );
			m_SpawnerDefs.PushBack( SpawnerDef );
			m_SpawnerCategoryMap[ SpawnerDefIndex ] = CategoryIndex;
		}
	}

	Reinitialize();
}

void EldritchTools::Reinitialize( const uint RoomScalarX /*= 1*/, const uint RoomScalarY /*= 1*/, const uint RoomScalarZ /*= 1*/ )
{
	ShutDown();

	m_CurrentMapName = "";

	m_RoomScalarX = RoomScalarX;
	m_RoomScalarY = RoomScalarY;
	m_RoomScalarZ = RoomScalarZ;

	m_Framework = EldritchFramework::GetInstance();
	ASSERT( m_Framework );

	m_World = m_Framework->GetWorld();
	ASSERT( m_World );

	uint RoomSizeX, RoomSizeY, RoomSizeZ;
	m_World->GetRoomDimensions( RoomSizeX, RoomSizeY, RoomSizeZ );

	m_RoomSizeX = RoomSizeX * RoomScalarX;
	m_RoomSizeY = RoomSizeY * RoomScalarY;
	m_RoomSizeZ = RoomSizeZ * RoomScalarZ;
	m_NumVoxels = m_RoomSizeX * m_RoomSizeY * m_RoomSizeZ;

	m_VoxelMeshes.ResizeZero( m_NumVoxels );
	m_VoxelMap.ResizeZero( m_NumVoxels );

	m_PaletteVoxels.Reserve( m_World->GetWorldDef().m_VoxelDefs.Size() );

	m_CameraLocation.x = static_cast<float>( m_RoomSizeX ) * 0.5f;
	m_CameraLocation.y = static_cast<float>( m_RoomSizeY ) * 0.25f;
	m_CameraLocation.z = static_cast<float>( m_RoomSizeZ ) * 0.75f;
	m_CameraOrientation = Angles();

	m_BrushBase.ResizeZero( 3 );
	m_BrushSize.ResizeZero( 3 );
	ResetBrush();

	m_BoundPlanes.Clear();
	m_BoundPlanes.PushBack( Plane( Vector( 1.0f, 0.0f, 0.0f ),	Vector( 0.0f, 0.0f, 0.0f ) ) );
	m_BoundPlanes.PushBack( Plane( Vector( -1.0f, 0.0f, 0.0f ),	Vector( static_cast<float>( m_RoomSizeX ), 0.0f, 0.0f ) ) );
	m_BoundPlanes.PushBack( Plane( Vector( 0.0f, 1.0f, 0.0f ),	Vector( 0.0f, 0.0f, 0.0f ) ) );
	m_BoundPlanes.PushBack( Plane( Vector( 0.0f, -1.0f, 0.0f ),	Vector( 0.0f, static_cast<float>( m_RoomSizeY ), 0.0f ) ) );
	m_BoundPlanes.PushBack( Plane( Vector( 0.0f, 0.0f, 1.0f ),	Vector( 0.0f, 0.0f, 0.0f ) ) );
	m_BoundPlanes.PushBack( Plane( Vector( 0.0f, 0.0f, -1.0f ),	Vector( 0.0f, 0.0f, static_cast<float>( m_RoomSizeZ ) ) ) );

	InitializeHelpMeshes();
	InitializeGridMeshes();
	InitializePaletteMeshes();
	InitializeSpawnerMeshes();

	if( m_Palette.Empty() )
	{
		m_Palette.PushBack( 1 );
	}

	Clear( true, false );
}

void EldritchTools::InitializeHelpMeshes()
{
	// This is (for the moment) easier than using the UI stack for tools work.
	IRenderer* const pRenderer = m_Framework->GetRenderer();

	const SimpleString HelpText = SimpleString( "F6: Save\tF7: Clear\tF8: Fill\tF9: Load\nQ: Up\tE: Down\nEnter: Fill\tShift+Enter: Erase\nHold Alt for more options." );
	m_HelpMesh = pRenderer->Print( HelpText.CStr(), pRenderer->GetFontManager()->GetFont( DEFAULT_FONT ), SRect(), 0 );

	const SimpleString AltHelpText = SimpleString( "Alt+1: New 1x1x1\tAlt+2: New 1x2x1\tAlt+3: New 1x1x2\tAlt+4: New 2x2x1\nAlt+F7: Clear with floor\tAlt+F8: Clear with shell" );
	m_AltHelpMesh = pRenderer->Print( AltHelpText.CStr(), pRenderer->GetFontManager()->GetFont( DEFAULT_FONT ), SRect(), 0 );
}

void EldritchTools::CreateGridMesh( const Array<Vector>& Corners, const uint Color )
{
	ASSERT( Corners.Size() == 4 );

	IRenderer* const pRenderer = m_Framework->GetRenderer();

	Mesh* const pQuadMesh = pRenderer->GetMeshFactory()->CreateDebugQuad( Corners[0], Corners[1], Corners[2], Corners[3], Color );
	pQuadMesh->SetMaterialDefinition( DEFAULT_NEWMATERIAL, pRenderer );
	pQuadMesh->SetTexture( 0, pRenderer->GetTextureManager()->GetTexture( DEFAULT_TEXTURE ) );
	pQuadMesh->SetMaterialFlags( MAT_DEBUG_WORLD );
	m_GridMeshes.PushBack( pQuadMesh );
}

void EldritchTools::InitializeGridMeshes()
{
	m_GridMeshes.Reserve( m_RoomSizeX * m_RoomSizeY * m_RoomSizeZ );

	static const float	kBuffer			= 0.01f;
	static const float	kMin			= kBuffer;
	static const float	kMax			= 1.0f - kBuffer;
	static const uint	kGridColor		= ARGB_TO_COLOR( 255, 224, 208, 192 );
	static const uint	kExitGuideColor	= ARGB_TO_COLOR( 255, 0, 255, 0 );

	const float	fRoomSizeX	= static_cast<float>( m_RoomSizeX );
	const float	fRoomSizeY	= static_cast<float>( m_RoomSizeY );
	const float	fRoomSizeZ	= static_cast<float>( m_RoomSizeZ );
	const float fHalfX		= fRoomSizeX * 0.5f;
	const float fHalfY		= fRoomSizeY * 0.5f;

	Array<Vector> Corners;
	Corners.Resize( 4 );

	// Exit guides
	Corners[0] = Vector( -kBuffer, fHalfY - 1.0f, 2.0f );
	Corners[1] = Vector( -kBuffer, fHalfY + 1.0f, 2.0f );
	Corners[2] = Vector( -kBuffer, fHalfY - 1.0f, 4.0f );
	Corners[3] = Vector( -kBuffer, fHalfY + 1.0f, 4.0f );
	CreateGridMesh( Corners, kExitGuideColor );

	Corners[0] = Vector( fRoomSizeX + kBuffer, fHalfY - 1.0f, 2.0f );
	Corners[1] = Vector( fRoomSizeX + kBuffer, fHalfY + 1.0f, 2.0f );
	Corners[2] = Vector( fRoomSizeX + kBuffer, fHalfY - 1.0f, 4.0f );
	Corners[3] = Vector( fRoomSizeX + kBuffer, fHalfY + 1.0f, 4.0f );
	CreateGridMesh( Corners, kExitGuideColor );

	Corners[0] = Vector( fHalfX - 1.0f, -kBuffer, 2.0f );
	Corners[1] = Vector( fHalfX + 1.0f, -kBuffer, 2.0f );
	Corners[2] = Vector( fHalfX - 1.0f, -kBuffer, 4.0f );
	Corners[3] = Vector( fHalfX + 1.0f, -kBuffer, 4.0f );
	CreateGridMesh( Corners, kExitGuideColor );

	Corners[0] = Vector( fHalfX - 1.0f, fRoomSizeY + kBuffer, 2.0f );
	Corners[1] = Vector( fHalfX + 1.0f, fRoomSizeY + kBuffer, 2.0f );
	Corners[2] = Vector( fHalfX - 1.0f, fRoomSizeY + kBuffer, 4.0f );
	Corners[3] = Vector( fHalfX + 1.0f, fRoomSizeY + kBuffer, 4.0f );
	CreateGridMesh( Corners, kExitGuideColor );

	Corners[0] = Vector( fHalfX - 2.0f, fHalfY - 2.0f, -kBuffer );
	Corners[1] = Vector( fHalfX + 2.0f, fHalfY - 2.0f, -kBuffer );
	Corners[2] = Vector( fHalfX - 2.0f, fHalfY + 2.0f, -kBuffer );
	Corners[3] = Vector( fHalfX + 2.0f, fHalfY + 2.0f, -kBuffer );
	CreateGridMesh( Corners, kExitGuideColor );

	Corners[0] = Vector( fHalfX - 2.0f, fHalfY - 2.0f, fRoomSizeZ + kBuffer );
	Corners[1] = Vector( fHalfX + 2.0f, fHalfY - 2.0f, fRoomSizeZ + kBuffer );
	Corners[2] = Vector( fHalfX - 2.0f, fHalfY + 2.0f, fRoomSizeZ + kBuffer );
	Corners[3] = Vector( fHalfX + 2.0f, fHalfY + 2.0f, fRoomSizeZ + kBuffer );
	CreateGridMesh( Corners, kExitGuideColor );

	for( int Y = 0; Y < m_RoomSizeY; ++Y )
	{
		for( int X = 0; X < m_RoomSizeX; ++X )
		{
			const float fX = static_cast<float>( X );
			const float fY = static_cast<float>( Y );

			Corners[0] = Vector( fX + kMin, fY + kMin, -kBuffer );
			Corners[1] = Vector( fX + kMax, fY + kMin, -kBuffer );
			Corners[2] = Vector( fX + kMin, fY + kMax, -kBuffer );
			Corners[3] = Vector( fX + kMax, fY + kMax, -kBuffer );
			CreateGridMesh( Corners, kGridColor );

			Corners[0] = Vector( fX + kMin, fY + kMin, fRoomSizeZ + kBuffer );
			Corners[1] = Vector( fX + kMax, fY + kMin, fRoomSizeZ + kBuffer );
			Corners[2] = Vector( fX + kMin, fY + kMax, fRoomSizeZ + kBuffer );
			Corners[3] = Vector( fX + kMax, fY + kMax, fRoomSizeZ + kBuffer );
			CreateGridMesh( Corners, kGridColor );
		}
	}

	for( int Z = 0; Z < m_RoomSizeZ; ++Z )
	{
		for( int X = 0; X < m_RoomSizeX; ++X )
		{
			const float fX = static_cast<float>( X );
			const float fZ = static_cast<float>( Z );

			Corners[0] = Vector( fX + kMin, -kBuffer, fZ + kMin );
			Corners[1] = Vector( fX + kMax, -kBuffer, fZ + kMin );
			Corners[2] = Vector( fX + kMin, -kBuffer, fZ + kMax );
			Corners[3] = Vector( fX + kMax, -kBuffer, fZ + kMax );
			CreateGridMesh( Corners, kGridColor );

			Corners[0] = Vector( fX + kMin, fRoomSizeY + kBuffer, fZ + kMin );
			Corners[1] = Vector( fX + kMax, fRoomSizeY + kBuffer, fZ + kMin );
			Corners[2] = Vector( fX + kMin, fRoomSizeY + kBuffer, fZ + kMax );
			Corners[3] = Vector( fX + kMax, fRoomSizeY + kBuffer, fZ + kMax );
			CreateGridMesh( Corners, kGridColor );
		}
	}

	for( int Z = 0; Z < m_RoomSizeZ; ++Z )
	{
		for( int Y = 0; Y < m_RoomSizeY; ++Y )
		{
			const float fY = static_cast<float>( Y );
			const float fZ = static_cast<float>( Z );

			Corners[0] = Vector( -kBuffer, fY + kMin, fZ + kMin );
			Corners[1] = Vector( -kBuffer, fY + kMax, fZ + kMin );
			Corners[2] = Vector( -kBuffer, fY + kMin, fZ + kMax );
			Corners[3] = Vector( -kBuffer, fY + kMax, fZ + kMax );
			CreateGridMesh( Corners, kGridColor );

			Corners[0] = Vector( fRoomSizeX + kBuffer, fY + kMin, fZ + kMin );
			Corners[1] = Vector( fRoomSizeX + kBuffer, fY + kMax, fZ + kMin );
			Corners[2] = Vector( fRoomSizeX + kBuffer, fY + kMin, fZ + kMax );
			Corners[3] = Vector( fRoomSizeX + kBuffer, fY + kMax, fZ + kMax );
			CreateGridMesh( Corners, kGridColor );
		}
	}
}

void EldritchTools::SetVoxel( const vidx_t VoxelIndex, const vval_t VoxelValue )
{
	if( VoxelIndex == kInvalidVoxelIndex )
	{
		return;
	}

	ASSERT( VoxelIndex < m_NumVoxels );
	m_VoxelMap[ VoxelIndex ] = VoxelValue;
	RefreshVoxelMesh( VoxelIndex );
}

vval_t EldritchTools::GetPaletteValue()
{
	if( m_Palette.Size() )
	{
		return m_Palette[ Math::Random( m_Palette.Size() ) ];
	}
	else
	{
		return 1;
	}
}

void EldritchTools::InitializeSpawnerMeshes()
{
	static const float kBase	= 16.0f;
	static const float kSpacing	= 24.0f;

	IRenderer* const pRenderer = m_Framework->GetRenderer();

	float			MaxCategoryWidth = 0.0f;
	Map<uint, uint>	CategoryCountMap;

	const uint NumCategories = m_SpawnerCategories.Size();
	for( uint CategoryIndex = 0; CategoryIndex < NumCategories; ++CategoryIndex )
	{
		const SimpleString& SpawnerCategory = m_SpawnerCategories[ CategoryIndex ];
		SRect Rect = SRect( kBase, kBase + CategoryIndex * kSpacing, 0.0f, 0.0f );
		Mesh* const pCategoryMesh = pRenderer->Print( SpawnerCategory.CStr(), pRenderer->GetFontManager()->GetFont( DEFAULT_FONT ), Rect, 0 );

		SSpawnerCategoryButton CategoryButton;
		CategoryButton.m_Mesh					= pCategoryMesh;
		CategoryButton.m_SpawnerCategoryIndex	= CategoryIndex;
		CategoryButton.m_BoxMin					= Vector2( pCategoryMesh->m_AABB.m_Min.x, pCategoryMesh->m_AABB.m_Min.z );
		CategoryButton.m_BoxMax					= Vector2( pCategoryMesh->m_AABB.m_Max.x, pCategoryMesh->m_AABB.m_Max.z );

		const float CategoryWidth = pCategoryMesh->m_AABB.m_Max.x - pCategoryMesh->m_AABB.m_Min.x;
		MaxCategoryWidth = Max( CategoryWidth, MaxCategoryWidth );

		m_SpawnerCategoryButtons.PushBack( CategoryButton );

		CategoryCountMap[ CategoryIndex ] = 0;
	}

	const uint NumSpawnerDefs = m_SpawnerDefs.Size();
	for( uint SpawnerDefIndex = 0; SpawnerDefIndex < NumSpawnerDefs; ++SpawnerDefIndex )
	{
		const uint CategoryIndex = m_SpawnerCategoryMap[ SpawnerDefIndex ];
		const uint CategoryCount = CategoryCountMap[ CategoryIndex ]++;

		const SimpleString& SpawnerDef = m_SpawnerDefs[ SpawnerDefIndex ];
		SRect Rect = SRect( kBase + MaxCategoryWidth + kSpacing, kBase + CategoryCount * kSpacing, 0.0f, 0.0f );
		Mesh* const pSpawnerDefMesh = pRenderer->Print( SpawnerDef.CStr(), pRenderer->GetFontManager()->GetFont( DEFAULT_FONT ), Rect, 0 );

		SSpawnerButton SpawnerButton;
		SpawnerButton.m_Mesh			= pSpawnerDefMesh;
		SpawnerButton.m_SpawnerDefIndex	= SpawnerDefIndex;
		SpawnerButton.m_BoxMin			= Vector2( pSpawnerDefMesh->m_AABB.m_Min.x, pSpawnerDefMesh->m_AABB.m_Min.z );
		SpawnerButton.m_BoxMax			= Vector2( pSpawnerDefMesh->m_AABB.m_Max.x, pSpawnerDefMesh->m_AABB.m_Max.z );

		m_SpawnerButtons.PushBack( SpawnerButton );
	}
}

void EldritchTools::InitializePaletteMeshes()
{
	const Map<vval_t, EldritchWorld::SVoxelDef>& VoxelDefs = m_World->GetWorldDef().m_VoxelDefs;

	FOR_EACH_MAP( VoxelDefIter, VoxelDefs, vval_t, EldritchWorld::SVoxelDef )
	{
		CreatePaletteMesh( VoxelDefIter.GetKey(), VoxelDefIter.GetValue() );
	}
}

void EldritchTools::CreatePaletteMesh( const vval_t VoxelValue, const EldritchWorld::SVoxelDef& VoxelDef )
{
	IRenderer* const pRenderer = m_Framework->GetRenderer();

	static const uint	kNumVertices	= 12;
	static const uint	kNumIndices		= 18;
	static const float	kCos60			= 0.5f;
	static const float	kSin60			= 0.866f;
	static const float	kMeshScale		= 48.0f;
	static const float	kSpacing		= 32.0f;
	static const float	kHalfDimX		= kSin60 * kMeshScale;
	static const float	kHalfDimY		= kMeshScale;
	static const float	kBaseX			= kSpacing + kHalfDimX;
	static const float	kSpacingX		= kSpacing + kHalfDimX * 2.0f;
	static const float	kBaseY			= kSpacing + kHalfDimY;
	static const float	kSpacingY		= kSpacing + kHalfDimY * 2.0f;

	Array<Vector>	Positions;
	Array<Vector4>	Colors;
	Array<Vector2>	UVs;
	Array<index_t>	Indices;

	Positions.Reserve(	kNumVertices );
	Colors.Reserve(		kNumVertices );
	UVs.Reserve(		kNumVertices );
	Indices.Reserve(	kNumIndices );

	// Ordered:
	//		A
	//	B		C
	//		D
	//	E		F
	//		G

	static const Vector kVA = Vector( 0.0f,		0.0f, -1.0f );
	static const Vector kVB = Vector( -0.866f,	0.0f, -0.5f );
	static const Vector kVC = Vector( 0.866f,	0.0f, -0.5f );
	static const Vector kVD = Vector( 0.0f,		0.0f, 0.0f );
	static const Vector kVE = Vector( -0.866f,	0.0f, 0.5f );
	static const Vector kVF = Vector( 0.866f,	0.0f, 0.5f );
	static const Vector kVG = Vector( 0.0f,		0.0f, 1.0f );

#define PUSH_POSITIONS( a, b, c, d ) Positions.PushBack( kV##a ); Positions.PushBack( kV##b ); Positions.PushBack( kV##c ); Positions.PushBack( kV##d )
	PUSH_POSITIONS( A, C, D, B );	// Top face
	PUSH_POSITIONS( B, D, G, E );	// Left face
	PUSH_POSITIONS( D, C, F, G );	// Right face
#undef PUSH_POSITIONS

	static const Vector4	kDefaultColor		= Vector4( 1.0f, 1.0f, 1.0f, 1.0f );
	static const Vector4	kUnbreakableColor	= Vector4( 1.0f, 0.0f, 1.0f, 1.0f );
	const Vector4			VoxelColor			= ( ( VoxelValue & 0x80 ) == 0 ) ? kDefaultColor : kUnbreakableColor;
	for( uint ColorIndex = 0; ColorIndex < kNumVertices; ++ColorIndex )
	{
		Colors.PushBack( VoxelColor );
	}

	const Vector2		UVBaseTop				= m_World->GetTileUV( VoxelDef.m_TopTile );
	const Vector2		UVBaseSide				= m_World->GetTileUV( VoxelDef.m_SideTile );
	const float			TileU					= m_World->GetTileU();
	const float			TileV					= m_World->GetTileV();
	const Vector2		UV0						= Vector2( 0.0f,	0.0f );
	const Vector2		UV1						= Vector2( TileU,	0.0f );
	const Vector2		UV2						= Vector2( 0.0f,	TileV );
	const Vector2		UV3						= Vector2( TileU,	TileV );

#define PUSH_UVS( a, b, c, d, s ) UVs.PushBack( UVBase##s + UV##a ); UVs.PushBack( UVBase##s + UV##b ); UVs.PushBack( UVBase##s + UV##c ); UVs.PushBack( UVBase##s + UV##d )
	PUSH_UVS( 0, 1, 3, 2, Top );	// Top face
	PUSH_UVS( 0, 1, 3, 2, Side );	// Left face
	PUSH_UVS( 0, 1, 3, 2, Side );	// Right face
#undef PUSH_UVS

	// Indices in CCW winding order around quad.
#define PUSH_INDICES( a, b, c, d ) Indices.PushBack( a ); Indices.PushBack( b ); Indices.PushBack( d ); Indices.PushBack( b ); Indices.PushBack( c ); Indices.PushBack( d )
	PUSH_INDICES(  0,  3,  2,  1 );	// Top face
	PUSH_INDICES(  4,  7,  6,  5 );	// Left face
	PUSH_INDICES(  8, 11, 10,  9 );	// Right face
#undef PUSH_INDICES

	IVertexBuffer* const		pVertexBuffer		= pRenderer->CreateVertexBuffer();
	IVertexDeclaration* const	pVertexDeclaration	= pRenderer->GetVertexDeclaration( VD_POSITIONS | VD_UVS | VD_FLOATCOLORS_SM2 );
	IIndexBuffer* const			pIndexBuffer		= pRenderer->CreateIndexBuffer();

	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices	= kNumVertices;
	InitStruct.Positions	= Positions.GetData();
	InitStruct.FloatColors1	= Colors.GetData();
	InitStruct.UVs			= UVs.GetData();
	pVertexBuffer->Init( InitStruct );
	pIndexBuffer->Init( kNumIndices, Indices.GetData() );
	pIndexBuffer->SetPrimitiveType( EPT_TRIANGLELIST );

	Mesh* pPaletteMesh = new Mesh( pVertexBuffer, pVertexDeclaration, pIndexBuffer );

	pPaletteMesh->m_AABB = AABB( Vector( -0.5f, 0.0f, -0.5f ), Vector( 0.5f, 0.0f, 0.5f ) );

	pPaletteMesh->SetMaterialDefinition( "Material_WorldTools", pRenderer );
	const SimpleString& WorldTileset = m_World->GetWorldDef().m_Tileset;
	pPaletteMesh->SetTexture( 0, pRenderer->GetTextureManager()->GetTexture( WorldTileset.CStr() ) );
	pPaletteMesh->SetMaterialFlags( MAT_HUD );

	const float fDisplayWidth = static_cast<float>( m_Framework->GetDisplay()->m_Width );
	const uint NumPalettesX = static_cast<uint>( Floor( ( fDisplayWidth - kBaseX ) / kSpacingX ) );
	const uint PaletteX = m_PaletteVoxels.Size() % NumPalettesX;
	const uint PaletteY = m_PaletteVoxels.Size() / NumPalettesX;

	const float LocationX	= kBaseX + kSpacingX * PaletteX;
	const float LocationY	= kBaseY + kSpacingY * PaletteY;
	const Vector Location	= Vector( LocationX, 0.0f, LocationY );

	pPaletteMesh->m_Location = Location;
	pPaletteMesh->m_Scale = Vector( kMeshScale, 1.0f, kMeshScale );

	SPalVox PaletteVoxel;
	PaletteVoxel.m_Mesh = pPaletteMesh;
	PaletteVoxel.m_Voxel = VoxelValue;
	PaletteVoxel.m_BoxMin = Vector2( LocationX - kHalfDimX, LocationY - kHalfDimY );
	PaletteVoxel.m_BoxMax = Vector2( LocationX + kHalfDimX, LocationY + kHalfDimY );

	m_PaletteVoxels.PushBack( PaletteVoxel );
}

void EldritchTools::RefreshVoxelMeshes()
{
	for( uint VoxelIndex = 0; VoxelIndex < m_NumVoxels; ++VoxelIndex )
	{
		RefreshVoxelMesh( VoxelIndex );
	}
}

void EldritchTools::RefreshVoxelMesh( const vidx_t VoxelIndex )
{
	ASSERT( VoxelIndex < m_NumVoxels );

	if( m_World->IsVisible( m_VoxelMap[ VoxelIndex ] ) )
	{
		CreateVoxelMesh( VoxelIndex );
	}
	else
	{
		DestroyVoxelMesh( VoxelIndex );
	}
}

void EldritchTools::CreateVoxelMesh( const vidx_t VoxelIndex )
{
	ASSERT( VoxelIndex < m_NumVoxels );

	DestroyVoxelMesh( VoxelIndex );

	static const uint	kNumVertices	= 24;
	static const uint	kNumIndices		= 36;

	// Ordered X-Y-Z:
	//   6---7
	//  /|  /|
	// 4-2-5-3
	// |/  |/
	// 0---1

	// Used in macro magic below
	static const Vector	kV0				= Vector( 0.0f, 0.0f, 0.0f );
	static const Vector	kV1				= Vector( 1.0f, 0.0f, 0.0f );
	static const Vector	kV2				= Vector( 0.0f, 1.0f, 0.0f );
	static const Vector	kV3				= Vector( 1.0f, 1.0f, 0.0f );
	static const Vector	kV4				= Vector( 0.0f, 0.0f, 1.0f );
	static const Vector	kV5				= Vector( 1.0f, 0.0f, 1.0f );
	static const Vector	kV6				= Vector( 0.0f, 1.0f, 1.0f );
	static const Vector	kV7				= Vector( 1.0f, 1.0f, 1.0f );

	const vval_t		VoxelValue				= m_VoxelMap[ VoxelIndex ];
	const EldritchWorld::SVoxelDef& VoxelDef	= m_World->GetVoxelDef( VoxelValue );
	const Vector2		UVBaseBottom			= m_World->GetTileUV( VoxelDef.m_BottomTile );
	const Vector2		UVBaseTop				= m_World->GetTileUV( VoxelDef.m_TopTile );
	const Vector2		UVBaseSide				= m_World->GetTileUV( VoxelDef.m_SideTile );
	const float			TileU					= m_World->GetTileU();
	const float			TileV					= m_World->GetTileV();
	const Vector2		UV0						= Vector2( 0.0f,	0.0f );
	const Vector2		UV1						= Vector2( TileU,	0.0f );
	const Vector2		UV2						= Vector2( 0.0f,	TileV );
	const Vector2		UV3						= Vector2( TileU,	TileV );

	const Vector VoxelBase = GetCoordsForVoxel( VoxelIndex );

	Array<Vector>	Positions;
	Array<Vector4>	Colors;
	Array<Vector2>	UVs;
	Array<index_t>	Indices;

	Positions.Reserve(	kNumVertices );
	Colors.Reserve(		kNumVertices );
	UVs.Reserve(		kNumVertices );
	Indices.Reserve(	kNumIndices );

	// Faces ordered Z-, Z+, Y-, Y+, X-, X+
#define PUSH_POSITIONS( a, b, c, d ) Positions.PushBack( VoxelBase + kV##a ); Positions.PushBack( VoxelBase + kV##b ); Positions.PushBack( VoxelBase + kV##c ); Positions.PushBack( VoxelBase + kV##d )
	PUSH_POSITIONS( 0, 1, 2, 3 );	// Z- (bottom face)
	PUSH_POSITIONS( 4, 5, 6, 7 );	// Z+ (top face)
	PUSH_POSITIONS( 0, 1, 4, 5 );	// Y- (back face)
	PUSH_POSITIONS( 2, 3, 6, 7 );	// Y+ (front face)
	PUSH_POSITIONS( 0, 2, 4, 6 );	// X- (left face)
	PUSH_POSITIONS( 1, 3, 5, 7 );	// X+ (right face)
#undef PUSH_POSITIONS

	static const Vector4	kTopColor			= Vector4( 1.0f, 1.0f, 1.0f, 1.0f );
	static const Vector4	kBottomColor		= Vector4( 0.4f, 0.4f, 0.4f, 1.0f );
	static const Vector4	kXColor				= Vector4( 0.8f, 0.8f, 0.8f, 1.0f );
	static const Vector4	kYColor				= Vector4( 0.6f, 0.6f, 0.6f, 1.0f );
	static const Vector4	kColors[]			= { kBottomColor, kTopColor, kYColor, kYColor, kXColor, kXColor };
	static const Vector4	kUnbreakableColor	= Vector4( 1.0f, 0.0f, 1.0f, 1.0f );
	for( uint ColorIndex = 0; ColorIndex < kNumVertices; ++ColorIndex )
	{
		const Vector4		FaceColor			= kColors[ ColorIndex / 4 ];
		const Vector4		VoxelColor			= ( ( VoxelValue & 0x80 ) == 0 ) ? FaceColor : kUnbreakableColor;

		Colors.PushBack( VoxelColor );
	}

#define PUSH_UVS( a, b, c, d, s ) UVs.PushBack( UVBase##s + UV##a ); UVs.PushBack( UVBase##s + UV##b ); UVs.PushBack( UVBase##s + UV##c ); UVs.PushBack( UVBase##s + UV##d )
	PUSH_UVS( 0, 1, 2, 3, Bottom );	// Z- (bottom face)
	PUSH_UVS( 2, 3, 0, 1, Top );	// Z+ (top face)
	PUSH_UVS( 2, 3, 0, 1, Side );	// Y- (back face)
	PUSH_UVS( 3, 2, 1, 0, Side );	// Y+ (front face)
	PUSH_UVS( 3, 2, 1, 0, Side );	// X- (left face)
	PUSH_UVS( 2, 3, 0, 1, Side );	// X+ (right face)
#undef PUSH_UVS

	// Indices in CCW winding order around quad.
#define PUSH_INDICES( a, b, c, d ) Indices.PushBack( a ); Indices.PushBack( b ); Indices.PushBack( d ); Indices.PushBack( b ); Indices.PushBack( c ); Indices.PushBack( d )
	PUSH_INDICES(  0,  2,  3,  1 );		// Z- (bottom face)
	PUSH_INDICES(  4,  5,  7,  6 );		// Z+ (top face)
	PUSH_INDICES(  8,  9, 11, 10 );		// Y- (back face)
	PUSH_INDICES( 12, 14, 15, 13 );		// Y+ (front face)
	PUSH_INDICES( 16, 18, 19, 17 );		// X- (left face)
	PUSH_INDICES( 20, 21, 23, 22 );		// X+ (right face)
#undef PUSH_INDICES

	IRenderer* const			pRenderer			= m_Framework->GetRenderer();
	IVertexBuffer* const		pVertexBuffer		= pRenderer->CreateVertexBuffer();
	IVertexDeclaration* const	pVertexDeclaration	= pRenderer->GetVertexDeclaration( VD_POSITIONS | VD_UVS | VD_FLOATCOLORS_SM2 );
	IIndexBuffer* const			pIndexBuffer		= pRenderer->CreateIndexBuffer();

	IVertexBuffer::SInit		InitStruct;
	InitStruct.NumVertices	= kNumVertices;
	InitStruct.Positions	= Positions.GetData();
	InitStruct.FloatColors1	= Colors.GetData();
	InitStruct.UVs			= UVs.GetData();

	pVertexBuffer->Init( InitStruct );
	pIndexBuffer->Init( kNumIndices, Indices.GetData() );
	pIndexBuffer->SetPrimitiveType( EPT_TRIANGLELIST );

	Mesh* pVoxelMesh = new Mesh( pVertexBuffer, pVertexDeclaration, pIndexBuffer );
	pVoxelMesh->m_AABB = AABB( VoxelBase, VoxelBase + kV7 );
	const SimpleString& WorldTileset = m_World->GetWorldDef().m_Tileset;
	pVoxelMesh->SetTexture( 0, pRenderer->GetTextureManager()->GetTexture( WorldTileset.CStr() ) );
	pVoxelMesh->SetMaterialDefinition( "Material_WorldTools", pRenderer );
	pVoxelMesh->SetMaterialFlags( MAT_DEBUG_WORLD );

	m_VoxelMeshes[ VoxelIndex ] = pVoxelMesh;
}

void EldritchTools::DestroyVoxelMesh( const vidx_t VoxelIndex )
{
	ASSERT( VoxelIndex < m_VoxelMeshes.Size() );
	SafeDelete( m_VoxelMeshes[ VoxelIndex ] );
}

uint EldritchTools::GetVoxelIndexForCoords( vpos_t X, vpos_t Y, vpos_t Z )
{
	ASSERT( X < m_RoomSizeX );
	ASSERT( Y < m_RoomSizeY );
	ASSERT( Z < m_RoomSizeZ );

	return X + ( Y * m_RoomSizeX ) + ( Z * m_RoomSizeX * m_RoomSizeY );
}

uint EldritchTools::SafeGetVoxelIndexForCoords( const Vector& Location )
{
	if( Location.x < 0.0f || Location.y < 0.0f || Location.z < 0.0f )
	{
		return kInvalidVoxelIndex;
	}

	vpos_t X = static_cast<vpos_t>( Location.x );
	vpos_t Y = static_cast<vpos_t>( Location.y );
	vpos_t Z = static_cast<vpos_t>( Location.z );
	return SafeGetVoxelIndexForCoords( X, Y, Z );
}

uint EldritchTools::SafeGetVoxelIndexForCoords( vpos_t X, vpos_t Y, vpos_t Z )
{
	if( X >= 0 && X < m_RoomSizeX &&
		Y >= 0 && Y < m_RoomSizeY &&
		Z >= 0 && Z < m_RoomSizeZ )
	{
		return GetVoxelIndexForCoords( X, Y, Z );
	}
	else
	{
		return kInvalidVoxelIndex;
	}
}

vval_t EldritchTools::SafeGetVoxel( const vidx_t VoxelIndex )
{
	if( VoxelIndex < m_NumVoxels )
	{
		return m_VoxelMap[ VoxelIndex ];
	}
	else
	{
		return kInvalidVoxelValue;
	}
}

void EldritchTools::GetCoordsForVoxel( const vidx_t VoxelIndex, vpos_t& X, vpos_t& Y, vpos_t& Z )
{
	ASSERT( VoxelIndex < m_NumVoxels );

	X = VoxelIndex % m_RoomSizeX;
	Y = ( VoxelIndex / m_RoomSizeX ) % m_RoomSizeY;
	Z = ( ( VoxelIndex / m_RoomSizeX ) / m_RoomSizeY ) % m_RoomSizeZ;
}

Vector EldritchTools::GetCoordsForVoxel( const vidx_t VoxelIndex )
{
	vpos_t X, Y, Z;
	GetCoordsForVoxel( VoxelIndex, X, Y, Z );

	const float fX = static_cast<float>( X );
	const float fY = static_cast<float>( Y );
	const float fZ = static_cast<float>( Z );

	return Vector( fX, fY, fZ );
}

bool EldritchTools::IsInToolMode() const
{
	return m_ToolMode;
}

void EldritchTools::ToggleToolMode()
{
	m_ToolMode = !m_ToolMode;

	STATIC_HASHED_STRING( HUD );
	STATIC_HASHED_STRING( ToolsHUD );
	UIManager* const	pUIManager	= m_Framework->GetUIManager();
	UIScreen* const		pOldHUD		= pUIManager->GetScreen( m_ToolMode ? sHUD : sToolsHUD );
	UIScreen* const		pNewHUD		= pUIManager->GetScreen( m_ToolMode ? sToolsHUD : sHUD );
	pUIManager->GetUIStack()->Replace( pOldHUD, pNewHUD );

	if( !m_ToolMode )
	{
		SetSubTool( EST_None );
	}
}

void EldritchTools::Tick( const float DeltaTime )
{
	Unused( DeltaTime );

	static const float kVelocity			= 2.0f;
	static const float kRotationVelocity	= 0.5f;

	m_CameraLocation			+= m_CameraVelocity				* m_CameraSpeed		* DeltaTime;
	m_CameraOrientation			+= m_CameraRotationalVelocity	* kRotationVelocity	* DeltaTime;

	m_CameraOrientation.Pitch	= Clamp( m_CameraOrientation.Pitch, -PI * 0.49f, PI * 0.49f );

	m_Framework->SetMainViewTransform( m_CameraLocation, m_CameraOrientation );

	// Zero these after we're done with them.
	m_CameraVelocity.Zero();
	m_CameraRotationalVelocity.Zero();
}

void EldritchTools::TickInput()
{
	Keyboard* const pKeyboard = m_Framework->GetKeyboard();

	if( pKeyboard->OnRise( Keyboard::EB_F1 ) )
	{
		if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )
		{
			PrevSubTool();
		}
		else
		{
			NextSubTool();
		}
	}

	if( pKeyboard->OnRise( Keyboard::EB_F2 ) )
	{
		SetSubTool( EST_None );
	}

	if( pKeyboard->OnRise( Keyboard::EB_F3 ) )
	{
		SetSubTool( EST_Palette );
	}

	if( pKeyboard->OnRise( Keyboard::EB_F4 ) )
	{
		SetSubTool( EST_Spawners );
	}

	if( m_SubTool == EST_Palette )
	{
		TickPaletteInput();
	}
	else if( m_SubTool == EST_Spawners )
	{
		TickSpawnersInput();
	}
	else
	{
		TickNormalInput();
	}
}

void EldritchTools::TickPaletteInput()
{
	Keyboard* const pKeyboard = m_Framework->GetKeyboard();
	Mouse* const pMouse = m_Framework->GetMouse();

	if( pKeyboard->OnRise( Keyboard::EB_Mouse_Left ) )
	{
		const uint NumPaletteVoxels = m_PaletteVoxels.Size();
		for( uint PaletteVoxelIndex = 0; PaletteVoxelIndex < NumPaletteVoxels; ++PaletteVoxelIndex )
		{
			const SPalVox& PaletteVoxel = m_PaletteVoxels[ PaletteVoxelIndex ];

			int X, Y;
			pMouse->GetPosition( X, Y, m_Framework->GetWindow() );

			const float fX = static_cast<float>( X );
			const float fY = static_cast<float>( Y );

			if( fX >= PaletteVoxel.m_BoxMin.x &&
				fY >= PaletteVoxel.m_BoxMin.y &&
				fX <= PaletteVoxel.m_BoxMax.x &&
				fY <= PaletteVoxel.m_BoxMax.y )
			{
				// Clicked
				if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) || pKeyboard->IsHigh( Keyboard::EB_LeftControl ) )
				{
					// Add to palette, or remove if it's already there
					uint ExistingIndex;
					if( m_Palette.Find( PaletteVoxel.m_Voxel, ExistingIndex ) )
					{
						m_Palette.Remove( ExistingIndex );
					}
					else
					{
						m_Palette.PushBack( PaletteVoxel.m_Voxel );
					}
				}
				else
				{
					// Reset palette and close
					m_Palette.Clear();
					m_Palette.PushBack( PaletteVoxel.m_Voxel );
					SetSubTool( EST_None );
				}
			}
		}
	}
}

void EldritchTools::TickSpawnersInput()
{
	Keyboard* const pKeyboard = m_Framework->GetKeyboard();
	Mouse* const pMouse = m_Framework->GetMouse();

	if( pKeyboard->OnRise( Keyboard::EB_Mouse_Left ) )
	{
		const uint NumCategoryButtons = m_SpawnerCategoryButtons.Size();
		for( uint CategoryIndex = 0; CategoryIndex < NumCategoryButtons; ++CategoryIndex )
		{
			const SSpawnerCategoryButton& CategoryButton = m_SpawnerCategoryButtons[ CategoryIndex ];

			int X, Y;
			pMouse->GetPosition( X, Y, m_Framework->GetWindow() );

			const float fX = static_cast<float>( X );
			const float fY = static_cast<float>( Y );

			if( fX >= CategoryButton.m_BoxMin.x &&
				fY >= CategoryButton.m_BoxMin.y &&
				fX <= CategoryButton.m_BoxMax.x &&
				fY <= CategoryButton.m_BoxMax.y )
			{
				// Clicked
				m_CurrentSpawnerCategoryIndex = CategoryButton.m_SpawnerCategoryIndex;
			}
		}

		const uint NumSpawnerButtons = m_SpawnerButtons.Size();
		for( uint SpawnerButtonIndex = 0; SpawnerButtonIndex < NumSpawnerButtons; ++SpawnerButtonIndex )
		{
			const SSpawnerButton& SpawnerButton = m_SpawnerButtons[ SpawnerButtonIndex ];

			const uint SpawnerDefIndex = SpawnerButton.m_SpawnerDefIndex;
			const uint SpawnerCategoryIndex = m_SpawnerCategoryMap[ SpawnerDefIndex ];
			if( SpawnerCategoryIndex != m_CurrentSpawnerCategoryIndex )
			{
				continue;
			}

			int X, Y;
			pMouse->GetPosition( X, Y, m_Framework->GetWindow() );

			const float fX = static_cast<float>( X );
			const float fY = static_cast<float>( Y );

			if( fX >= SpawnerButton.m_BoxMin.x &&
				fY >= SpawnerButton.m_BoxMin.y &&
				fX <= SpawnerButton.m_BoxMax.x &&
				fY <= SpawnerButton.m_BoxMax.y )
			{
				// Clicked
				m_CurrentSpawnerDefIndex = SpawnerDefIndex;
				SetSubTool( EST_None );
			}
		}
	}
}

void EldritchTools::SetSubTool( const ESubTool SubTool )
{
	Mouse* const pMouse = m_Framework->GetMouse();

	m_SubTool = SubTool;

	if( m_SubTool == EST_Palette || m_SubTool == EST_Spawners )
	{
		pMouse->SetActive( false );
	}
	else
	{
		pMouse->SetActive( true );
	}
}

void EldritchTools::PrevSubTool()
{
	const ESubTool PrevTool = static_cast<ESubTool>( ( m_SubTool + EST_COUNT - 1 ) % EST_COUNT );
	SetSubTool( PrevTool );
}

void EldritchTools::NextSubTool()
{
	const ESubTool NextTool = static_cast<ESubTool>( ( m_SubTool + 1 ) % EST_COUNT );
	SetSubTool( NextTool );
}

void EldritchTools::TickNormalInput()
{
	Keyboard* const pKeyboard = m_Framework->GetKeyboard();
	Mouse* const pMouse = m_Framework->GetMouse();

	Vector X, Y, Z;
	m_CameraOrientation.GetAxes( X, Y, Z );

	Z = Vector( 0.0f, 0.0f, 1.0f );

	static const float kNormalVelocity	= 4.0f;
	static const float kFastVelocity	= 8.0f;
	m_CameraSpeed = pKeyboard->IsHigh( Keyboard::EB_LeftShift ) ? kFastVelocity : kNormalVelocity;

	if( pKeyboard->IsHigh( Keyboard::EB_W ) )
	{
		m_CameraVelocity += Y;
	}

	if( pKeyboard->IsHigh( Keyboard::EB_S ) && !pKeyboard->IsHigh( Keyboard::EB_LeftControl ) )
	{
		m_CameraVelocity -= Y;
	}

	if( pKeyboard->IsHigh( Keyboard::EB_A ) )
	{
		m_CameraVelocity -= X;
	}

	if( pKeyboard->IsHigh( Keyboard::EB_D ) )
	{
		m_CameraVelocity += X;
	}

	if( pKeyboard->IsHigh( Keyboard::EB_Q ) )
	{
		m_CameraVelocity -= Z;
	}

	if( pKeyboard->IsHigh( Keyboard::EB_E ) )
	{
		m_CameraVelocity += Z;
	}

	m_ShowAltHelp = pKeyboard->IsHigh( Keyboard::EB_LeftAlt );

	if( pKeyboard->IsHigh( Keyboard::EB_LeftAlt ) && pKeyboard->OnRise( Keyboard::EB_1 ) )
	{
		Reinitialize( 1, 1, 1 );
	}

	if( pKeyboard->IsHigh( Keyboard::EB_LeftAlt ) && pKeyboard->OnRise( Keyboard::EB_2 ) )
	{
		Reinitialize( 1, 2, 1 );
	}

	if( pKeyboard->IsHigh( Keyboard::EB_LeftAlt ) && pKeyboard->OnRise( Keyboard::EB_3 ) )
	{
		Reinitialize( 1, 1, 2 );
	}

	if( pKeyboard->IsHigh( Keyboard::EB_LeftAlt ) && pKeyboard->OnRise( Keyboard::EB_4 ) )
	{
		Reinitialize( 2, 2, 1 );
	}

	if( pKeyboard->OnRise( Keyboard::EB_S ) && pKeyboard->IsHigh( Keyboard::EB_LeftControl ) )
	{
		TryQuickSave();
	}

	if( pKeyboard->OnRise( Keyboard::EB_F6 ) )
	{
		TrySave();
	}

	if( pKeyboard->OnRise( Keyboard::EB_F9 ) )
	{
		TryLoad();
	}

	if( pKeyboard->OnRise( Keyboard::EB_F7 ) )
	{
		TryClear( pKeyboard->IsHigh( Keyboard::EB_LeftAlt ), false );
	}

	if( pKeyboard->OnRise( Keyboard::EB_F8 ) )
	{
		if( pKeyboard->IsHigh( Keyboard::EB_LeftAlt ) )
		{
			TryClear( false, true );
		}
		else
		{
			TryFill();
		}
	}

	if( pKeyboard->OnRise( Keyboard::EB_Mouse_Left ) )
	{
		vidx_t HitVoxel, FacingVoxel;
		TraceVoxel( HitVoxel, FacingVoxel );

		const vidx_t AddVoxel = ( HitVoxel != kInvalidVoxelIndex && pKeyboard->IsHigh( Keyboard::EB_LeftControl ) ) ? HitVoxel : FacingVoxel;
		if( AddVoxel != kInvalidVoxelIndex )
		{
			SetVoxel( AddVoxel, GetPaletteValue() );
		}
	}

	if( pKeyboard->OnRise( Keyboard::EB_Mouse_Right ) )
	{
		vidx_t HitVoxel, FacingVoxel;
		TraceVoxel( HitVoxel, FacingVoxel );

		if( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) || pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )
		{
			// Eyedropper
			vval_t VoxelValue = SafeGetVoxel( HitVoxel );
			if( VoxelValue != kInvalidVoxelValue )
			{
				if( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) && pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )
				{
					// Add to palette
					m_Palette.PushBackUnique( VoxelValue );
				}
				else
				{
					// Reset palette
					m_Palette.Clear();
					m_Palette.PushBack( VoxelValue );
				}
			}
		}
		else
		{
			// Erase
			SetVoxel( HitVoxel, 0 );
		}
	}

	if( pKeyboard->OnRise( Keyboard::EB_Mouse_Middle ) )
	{
		vidx_t HitVoxel, FacingVoxel;
		TraceVoxel( HitVoxel, FacingVoxel );

		// Hold Ctrl to move brush around hit voxel instead of facing voxel
		const vidx_t BrushVoxel = ( HitVoxel != kInvalidVoxelIndex && pKeyboard->IsHigh( Keyboard::EB_LeftControl ) ) ? HitVoxel : FacingVoxel;
		if( BrushVoxel != kInvalidVoxelIndex )
		{
			// Hold Shift to expand brush to location
			if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )
			{
				ExpandBrushTo( BrushVoxel );
			}
			else
			{
				ShrinkBrushTo( BrushVoxel );
			}
		}
	}

	if( pKeyboard->OnRise( Keyboard::EB_R ) )
	{
		vidx_t HitVoxel, FacingVoxel;
		TraceVoxel( HitVoxel, FacingVoxel );

		if( FacingVoxel != kInvalidVoxelIndex )
		{
			if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )
			{
				m_Spawners.Remove( FacingVoxel );
			}
			else if( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) )
			{
				Map<vidx_t, SPlacedSpawner>::Iterator SpawnerIter = m_Spawners.Search( FacingVoxel );
				if( SpawnerIter.IsValid() )
				{
					SPlacedSpawner& PlacedSpawner = SpawnerIter.GetValue();
					PlacedSpawner.m_Orientation = ( PlacedSpawner.m_Orientation + 1 ) % 4;
				}
			}
			else
			{
				SPlacedSpawner& PlacedSpawner = m_Spawners[ FacingVoxel ];
				PlacedSpawner.m_SpawnerDef = m_CurrentSpawnerDefIndex;
				PlacedSpawner.m_Orientation = ESO_North;
			}
		}
	}

	TickBrushInput();

	m_CameraRotationalVelocity.Yaw		-= pMouse->GetPosition( Mouse::EA_X );
	m_CameraRotationalVelocity.Pitch	-= pMouse->GetPosition( Mouse::EA_Y );
}

// [ / ]: X axis
// ; / ': Y axis
// . / /: Z axis
// Shift: Adjust low bound
// Ctrl:  Adjust high bound
void EldritchTools::TickBrushInput()
{
	Keyboard* const pKeyboard = m_Framework->GetKeyboard();

	if( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) && pKeyboard->OnRise( Keyboard::EB_A ) )
	{
		MaximizeBrush();
	}

	if( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) && pKeyboard->OnRise( Keyboard::EB_D ) )
	{
		ResetBrush();
	}

	if( pKeyboard->OnRise( Keyboard::EB_Enter ) || pKeyboard->OnRise( Keyboard::EB_F ) )
	{
		if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )
		{
			EraseBrush();
		}
		else
		{
			FillBrush();
		}
	}

	if( pKeyboard->OnRise( Keyboard::EB_LeftBrace ) )
	{
		if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )			{ AdjustBrush( 0, -1, 1 ); }	// Expand brush left
		else if( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) )	{ AdjustBrush( 0, 0, -1 ); }	// Shrink brush left
		else													{ AdjustBrush( 0, -1, 0 ); }	// Move brush left
	}

	if( pKeyboard->OnRise( Keyboard::EB_RightBrace ) )
	{
		if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )			{ AdjustBrush( 0, 0, 1 ); }		// Expand brush right
		else if( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) )	{ AdjustBrush( 0, 1, -1 ); }	// Shrink brush right
		else													{ AdjustBrush( 0, 1, 0 ); }		// Move brush right
	}

	if( pKeyboard->OnRise( Keyboard::EB_Semicolon ) )
	{
		if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )			{ AdjustBrush( 1, -1, 1 ); }	// Expand brush back
		else if( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) )	{ AdjustBrush( 1, 0, -1 ); }	// Shrink brush back
		else													{ AdjustBrush( 1, -1, 0 ); }	// Move brush back
	}

	if( pKeyboard->OnRise( Keyboard::EB_Apostrophe ) )
	{
		if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )			{ AdjustBrush( 1, 0, 1 ); }		// Expand brush forward
		else if( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) )	{ AdjustBrush( 1, 1, -1 ); }	// Shrink brush forward
		else													{ AdjustBrush( 1, 1, 0 ); }		// Move brush forward
	}

	if( pKeyboard->OnRise( Keyboard::EB_Comma ) )
	{
		if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )			{ AdjustBrush( 2, -1, 1 ); }	// Expand brush down
		else if( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) )	{ AdjustBrush( 2, 0, -1 ); }	// Shrink brush down
		else													{ AdjustBrush( 2, -1, 0 ); }	// Move brush down
	}

	if( pKeyboard->OnRise( Keyboard::EB_Period ) )
	{
		if( pKeyboard->IsHigh( Keyboard::EB_LeftShift ) )			{ AdjustBrush( 2, 0, 1 ); }		// Expand brush up
		else if( pKeyboard->IsHigh( Keyboard::EB_LeftControl ) )	{ AdjustBrush( 2, 1, -1 ); }	// Shrink brush up
		else													{ AdjustBrush( 2, 1, 0 ); }		// Move brush up
	}
}

void EldritchTools::ResetBrush()
{
	m_BrushBase[0] = m_RoomSizeX / 2;
	m_BrushBase[1] = m_RoomSizeY / 2;
	m_BrushBase[2] = m_RoomSizeZ / 2;
	m_BrushSize[0] = 1;
	m_BrushSize[1] = 1;
	m_BrushSize[2] = 1;
}

void EldritchTools::MaximizeBrush()
{
	m_BrushBase[0] = 0;
	m_BrushBase[1] = 0;
	m_BrushBase[2] = 0;
	m_BrushSize[0] = m_RoomSizeX;
	m_BrushSize[1] = m_RoomSizeY;
	m_BrushSize[2] = m_RoomSizeZ;
}

void EldritchTools::AdjustBrush( const uint Axis, const int BaseChange, const int SizeChange )
{
	if( m_BrushSize[ Axis ] + SizeChange > 0 )
	{
		m_BrushBase[ Axis ] += BaseChange;
		m_BrushSize[ Axis ] += SizeChange;
	}
}

void EldritchTools::ExpandBrushTo( const vidx_t VoxelIndex )
{
	ASSERT( VoxelIndex < m_NumVoxels );

	vpos_t X, Y, Z;
	GetCoordsForVoxel( VoxelIndex, X, Y, Z );

	const int LoX = Min( m_BrushBase[0], X );
	const int LoY = Min( m_BrushBase[1], Y );
	const int LoZ = Min( m_BrushBase[2], Z );
	const int HiX = Max( m_BrushBase[0] + m_BrushSize[0] - 1, X );
	const int HiY = Max( m_BrushBase[1] + m_BrushSize[1] - 1, Y );
	const int HiZ = Max( m_BrushBase[2] + m_BrushSize[2] - 1, Z );

	m_BrushBase[0] = LoX;
	m_BrushBase[1] = LoY;
	m_BrushBase[2] = LoZ;
	m_BrushSize[0] = HiX - m_BrushBase[0] + 1;
	m_BrushSize[1] = HiY - m_BrushBase[1] + 1;
	m_BrushSize[2] = HiZ - m_BrushBase[2] + 1;
}

void EldritchTools::ShrinkBrushTo( const vidx_t VoxelIndex )
{
	ASSERT( VoxelIndex < m_NumVoxels );

	vpos_t X, Y, Z;
	GetCoordsForVoxel( VoxelIndex, X, Y, Z );

	m_BrushBase[0] = X;
	m_BrushBase[1] = Y;
	m_BrushBase[2] = Z;
	m_BrushSize[0] = 1;
	m_BrushSize[1] = 1;
	m_BrushSize[2] = 1;
}

void EldritchTools::FillBrush()
{
	FillBrush( true );
}

void EldritchTools::EraseBrush()
{
	FillBrush( false, 0 );
}

void EldritchTools::FillBrush( const bool UsePalette, const vval_t VoxelValue /*= 0*/ )
{
	const int MaxX = m_RoomSizeX - 1;
	const int MaxY = m_RoomSizeY - 1;
	const int MaxZ = m_RoomSizeZ - 1;

	const int LoX = Clamp( m_BrushBase[0],						0, MaxX );
	const int HiX = Clamp( m_BrushBase[0] + m_BrushSize[0] - 1,	0, MaxX );
	const int LoY = Clamp( m_BrushBase[1],						0, MaxY );
	const int HiY = Clamp( m_BrushBase[1] + m_BrushSize[1] - 1,	0, MaxY );
	const int LoZ = Clamp( m_BrushBase[2],						0, MaxZ );
	const int HiZ = Clamp( m_BrushBase[2] + m_BrushSize[2] - 1,	0, MaxZ );

	for( int Z = LoZ; Z <= HiZ; ++Z )
	{
		for( int Y = LoY; Y <= HiY; ++Y )
		{
			for( int X = LoX; X <= HiX; ++X )
			{
				const uint VoxelIndex = GetVoxelIndexForCoords( X, Y, Z );
				SetVoxel( VoxelIndex, UsePalette ? GetPaletteValue() : VoxelValue );
			}
		}
	}
}

void EldritchTools::TraceVoxel( vidx_t& HitVoxel, vidx_t& FacingVoxel )
{
	HitVoxel = kInvalidVoxelIndex;
	FacingVoxel = kInvalidVoxelIndex;

	const float RayLength = 2.0f * SqRt( static_cast<float>( Square( m_RoomSizeX ) + Square( m_RoomSizeY ) + Square( m_RoomSizeZ ) ) );
	Segment TraceSegment( m_CameraLocation, m_CameraLocation + m_CameraOrientation.ToVector() * RayLength );

	CollisionInfo Info;
	if( Trace( TraceSegment, Info ) || TraceBoundPlanes( TraceSegment, Info ) )
	{
		const Vector&	HitPoint		= Info.m_Intersection;
		const Vector&	HitNormal		= Info.m_Plane.m_Normal;
		const Vector	HitVoxelLoc		= HitPoint - ( HitNormal * 0.5f );
		const Vector	FacingVoxelLoc	= HitPoint + ( HitNormal * 0.5f );

		HitVoxel	= SafeGetVoxelIndexForCoords( HitVoxelLoc );
		FacingVoxel	= SafeGetVoxelIndexForCoords( FacingVoxelLoc );
	}
}

bool EldritchTools::TraceBoundPlanes( const Segment& TraceSegment, CollisionInfo& Info )
{
	CollisionInfo MinInfo;

	for( uint BoundPlaneIndex = 0; BoundPlaneIndex < m_BoundPlanes.Size(); ++BoundPlaneIndex )
	{
		const Plane& BoundPlane = m_BoundPlanes[ BoundPlaneIndex ];

		// Ignore bound planes traced from the back side
		if( TraceSegment.GetDirection().Dot( BoundPlane.m_Normal ) > 0.0f )
		{
			continue;
		}

		CollisionInfo CheckInfo;
		if( TraceSegment.Intersects( BoundPlane, &CheckInfo ) )
		{
			if( CheckInfo.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
			{
				MinInfo = CheckInfo;
			}
		}
	}

	Info.CopyOutParametersFrom( MinInfo );
	return Info.m_Collision;
}

bool EldritchTools::Trace( const Segment& TraceSegment, CollisionInfo& Info )
{
	CollisionInfo MinInfo;

	Vector MinCorner;
	Vector MaxCorner;
	Vector::MinMax( TraceSegment.m_Point1, TraceSegment.m_Point2, MinCorner, MaxCorner );

	AABB TraceBox( MinCorner, MaxCorner );
	vpos_t MinVoxelX = static_cast<vpos_t>( TraceBox.m_Min.x );
	vpos_t MinVoxelY = static_cast<vpos_t>( TraceBox.m_Min.y );
	vpos_t MinVoxelZ = static_cast<vpos_t>( TraceBox.m_Min.z );
	vpos_t MaxVoxelX = static_cast<vpos_t>( TraceBox.m_Max.x );
	vpos_t MaxVoxelY = static_cast<vpos_t>( TraceBox.m_Max.y );
	vpos_t MaxVoxelZ = static_cast<vpos_t>( TraceBox.m_Max.z );

	for( vpos_t VoxelZ = MinVoxelZ; VoxelZ <= MaxVoxelZ; ++VoxelZ )
	{
		for( vpos_t VoxelY = MinVoxelY; VoxelY <= MaxVoxelY; ++VoxelY )
		{
			for( vpos_t VoxelX = MinVoxelX; VoxelX <= MaxVoxelX; ++VoxelX )
			{
				const vidx_t VoxelIndex = SafeGetVoxelIndexForCoords( VoxelX, VoxelY, VoxelZ );
				if( VoxelIndex == kInvalidVoxelIndex )
				{
					continue;
				}

				const vval_t VoxelValue = SafeGetVoxel( VoxelIndex );
				if( VoxelValue == kEmptyVoxelValue ||
					VoxelValue == kInvalidVoxelValue )
				{
					continue;
				}

				const float VoxelBoxMinX = static_cast<float>( VoxelX );
				const float VoxelBoxMinY = static_cast<float>( VoxelY );
				const float VoxelBoxMinZ = static_cast<float>( VoxelZ );
				const float VoxelBoxMaxX = static_cast<float>( VoxelX + 1 );
				const float VoxelBoxMaxY = static_cast<float>( VoxelY + 1 );
				const float VoxelBoxMaxZ = static_cast<float>( VoxelZ + 1 );
				Vector VoxelMinCorner( VoxelBoxMinX, VoxelBoxMinY, VoxelBoxMinZ );
				Vector VoxelMaxCorner( VoxelBoxMaxX, VoxelBoxMaxY, VoxelBoxMaxZ );
				AABB VoxelBox = AABB( VoxelMinCorner, VoxelMaxCorner );

				CollisionInfo CheckInfo;
				if( TraceSegment.Intersects( VoxelBox, &CheckInfo ) )
				{
					if( CheckInfo.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
					{
						MinInfo = CheckInfo;
					}
				}
			}
		}
	}

	Info.CopyOutParametersFrom( MinInfo );
	return Info.m_Collision;
};

void EldritchTools::TryClear( const bool WithFloor, const bool WithShell )
{
	const int Response = MessageBox( NULL, "Clear world?", "Clear world?", MB_YESNO | MB_TASKMODAL | MB_SETFOREGROUND );
	if( Response == IDYES )
	{
		Clear( WithFloor, WithShell );
	}
}
void EldritchTools::Clear( const bool WithFloor, const bool WithShell )
{
	m_Spawners.Clear();

	for( uint VoxelIndex = 0; VoxelIndex < m_NumVoxels; ++VoxelIndex )
	{
		vpos_t X, Y, Z;
		GetCoordsForVoxel( VoxelIndex, X, Y, Z );

		const bool IsFloor = WithFloor && ( Z == 0 );
		const bool IsShell = WithShell && ( X == 0 || X == m_RoomSizeX - 1 || Y == 0 || Y == m_RoomSizeY - 1 || Z == 0 || Z == m_RoomSizeZ - 1 );

		vval_t ClearValue = ( IsFloor || IsShell ) ? GetPaletteValue() : 0;
		SetVoxel( VoxelIndex, ClearValue );
	}
}

void EldritchTools::TryFill()
{
	const int Response = MessageBox( NULL, "Fill world?", "Fill world?", MB_YESNO | MB_TASKMODAL | MB_SETFOREGROUND );
	if( Response == IDYES )
	{
		Fill();
	}
}
void EldritchTools::Fill()
{
	m_Spawners.Clear();

	for( uint VoxelIndex = 0; VoxelIndex < m_NumVoxels; ++VoxelIndex )
	{
		SetVoxel( VoxelIndex, GetPaletteValue() );
	}
}

void EldritchTools::RenderCoords()
{
	static const float	kCoordLength	= 0.5f;
	static const uint	kCoordXColor	= ARGB_TO_COLOR( 255, 255, 0, 0 );
	static const uint	kCoordYColor	= ARGB_TO_COLOR( 255, 0, 255, 0 );
	static const uint	kCoordZColor	= ARGB_TO_COLOR( 255, 0, 0, 255 );

	const float			fBrushMidX		= static_cast<float>( m_BrushBase[0] ) + static_cast<float>( m_BrushSize[0] ) * 0.5f;
	const float			fBrushMidY		= static_cast<float>( m_BrushBase[1] ) + static_cast<float>( m_BrushSize[1] ) * 0.5f;
	const float			fBrushMidZ		= static_cast<float>( m_BrushBase[2] ) + static_cast<float>( m_BrushSize[2] ) * 0.5f;

	const Vector		CoordBase		= Vector( fBrushMidX, fBrushMidY, fBrushMidZ );
	const Vector		CoordX			= CoordBase + Vector( kCoordLength, 0.0f, 0.0f );
	const Vector		CoordY			= CoordBase + Vector( 0.0f, kCoordLength, 0.0f );
	const Vector		CoordZ			= CoordBase + Vector( 0.0f, 0.0f, kCoordLength );

	m_Framework->GetRenderer()->DEBUGDrawLine( CoordBase, CoordX, kCoordXColor );
	m_Framework->GetRenderer()->DEBUGDrawLine( CoordBase, CoordY, kCoordYColor );
	m_Framework->GetRenderer()->DEBUGDrawLine( CoordBase, CoordZ, kCoordZColor );
}

void EldritchTools::RenderBrush()
{
	static const float	kBuffer			= 0.005f;
	static const Vector	kBufferVector	= Vector( kBuffer, kBuffer, kBuffer );
	static const uint	kBrushColor		= ARGB_TO_COLOR( 255, 255, 0, 0 );

	const float			fBrushMinX		= static_cast<float>( m_BrushBase[0] );
	const float			fBrushMinY		= static_cast<float>( m_BrushBase[1] );
	const float			fBrushMinZ		= static_cast<float>( m_BrushBase[2] );
	const float			fBrushMaxX		= static_cast<float>( m_BrushBase[0] + m_BrushSize[0] );
	const float			fBrushMaxY		= static_cast<float>( m_BrushBase[1] + m_BrushSize[1] );
	const float			fBrushMaxZ		= static_cast<float>( m_BrushBase[2] + m_BrushSize[2] );

	const Vector		BrushMin		= Vector( fBrushMinX, fBrushMinY, fBrushMinZ );
	const Vector		BrushMax		= Vector( fBrushMaxX, fBrushMaxY, fBrushMaxZ );

	m_Framework->GetRenderer()->DEBUGDrawBox( BrushMin - kBufferVector, BrushMax + kBufferVector, kBrushColor );
	m_Framework->GetRenderer()->DEBUGDrawBox( BrushMin + kBufferVector, BrushMax - kBufferVector, kBrushColor );
}

void EldritchTools::TickRender()
{
	IRenderer* const pRenderer = m_Framework->GetRenderer();

	static const uint	kClearColor				= ARGB_TO_COLOR( 255, 16, 24, 32 );
	static const uint	kSelectedPaletteColor	= ARGB_TO_COLOR( 255, 128, 192, 255 );
	static const uint	kSpawnerCrossColor		= ARGB_TO_COLOR( 255, 224, 96, 16 );
	static const Vector	kHalfVoxelOffset		= Vector( 0.5f, 0.5f, 0.5f );
	static const float	kSpawnerCrossLength		= 0.25f;
	static const float	kSpawnerArrowLength		= 0.5f;

	EldritchTargetManager* const pTargetManager = m_Framework->GetTargetManager();
	IRenderTarget* const pPrimaryRenderTarget = pTargetManager->GetPrimaryRenderTarget();
	pRenderer->SetRenderTarget( pPrimaryRenderTarget );
	pRenderer->Clear( CLEAR_COLOR, kClearColor );

	RenderCoords();
	RenderBrush();

	if( m_SubTool == EST_None )
	{
		pRenderer->AddMesh( m_ShowAltHelp ? m_AltHelpMesh : m_HelpMesh );
	}

	const uint NumGridMeshes = m_GridMeshes.Size();
	for( uint GridMeshIndex = 0; GridMeshIndex < NumGridMeshes; ++GridMeshIndex )
	{
		Mesh* const pGridMesh = m_GridMeshes[ GridMeshIndex ];
		pRenderer->AddMesh( pGridMesh );
	}

	const uint NumVoxelMeshes = m_VoxelMeshes.Size();
	for( uint VoxelMeshIndex = 0; VoxelMeshIndex < NumVoxelMeshes; ++VoxelMeshIndex )
	{
		Mesh* const pVoxelMesh = m_VoxelMeshes[ VoxelMeshIndex ];
		if( pVoxelMesh )
		{
			pRenderer->AddMesh( pVoxelMesh );
		}
	}

	const Matrix	ViewMatrix			= m_Framework->GetMainView()->GetViewMatrix();
	const Matrix	ProjectionMatrix	= m_Framework->GetMainView()->GetProjectionMatrix();
	const Matrix	VPMatrix			= ViewMatrix * ProjectionMatrix;
	const float		DisplayWidth		= static_cast<float>( m_Framework->GetDisplay()->m_Width );
	const float		DisplayHeight		= static_cast<float>( m_Framework->GetDisplay()->m_Height );

	FOR_EACH_MAP( SpawnerIter, m_Spawners, vidx_t, SPlacedSpawner )
	{
		vidx_t					VoxelIndex			= SpawnerIter.GetKey();
		const Vector4			Location			= kHalfVoxelOffset + GetCoordsForVoxel( VoxelIndex );
		const Vector4			ProjectedLocation	= Location * VPMatrix;

		if( ProjectedLocation.z < 0.0f )
		{
			continue;
		}

		const Vector4			NormalizedLocation	= ProjectedLocation / ProjectedLocation.w;
		const Vector2			ScaledLocation		= Vector2( DisplayWidth * ( NormalizedLocation.x * 0.5f + 0.5f ), DisplayHeight * ( -NormalizedLocation.y * 0.5f + 0.5f ) );
		const SRect				Rect				= SRect( Floor( ScaledLocation.x ), Floor( ScaledLocation.y ), 0.0f, 0.0f );
		const SRect				ShadowRect			= SRect( Rect.m_Left + 1.0f, Rect.m_Top + 1.0f, 0.0f, 0.0f );

		const SPlacedSpawner&	PlacedSpawner		= SpawnerIter.GetValue();
		const SimpleString&		SpawnerDef			= m_SpawnerDefs[ PlacedSpawner.m_SpawnerDef ];
		Font* const				pFont				= pRenderer->GetFontManager()->GetFont( DEFAULT_FONT );

		pRenderer->DEBUGDrawCross( Location, kSpawnerCrossLength, kSpawnerCrossColor );
		pRenderer->DEBUGPrint( SpawnerDef.CStr(), pFont, ShadowRect, ARGB_TO_COLOR( 128, 8, 8, 32 ) );
		pRenderer->DEBUGPrint( SpawnerDef.CStr(), pFont, Rect, ARGB_TO_COLOR( 255, 255, 255, 192 ) );

		Angles Orientation;
		Orientation.Yaw = GetYaw( PlacedSpawner.m_Orientation );
		pRenderer->DEBUGDrawArrow( Location, Orientation, kSpawnerArrowLength, kSpawnerCrossColor );
	}

	if( m_SubTool == EST_Palette )
	{
		const uint NumPaletteVoxels = m_PaletteVoxels.Size();
		for( uint PaletteVoxelIndex = 0; PaletteVoxelIndex < NumPaletteVoxels; ++PaletteVoxelIndex )
		{
			const SPalVox& PaletteVoxel = m_PaletteVoxels[ PaletteVoxelIndex ];
			pRenderer->AddMesh( PaletteVoxel.m_Mesh );

			if( m_Palette.Find( PaletteVoxel.m_Voxel ) )
			{
				const Vector BoxMin( PaletteVoxel.m_BoxMin.x, 0.0f, PaletteVoxel.m_BoxMin.y );
				const Vector BoxMax( PaletteVoxel.m_BoxMax.x, 0.0f, PaletteVoxel.m_BoxMax.y );
				pRenderer->DEBUGDrawBox2D( BoxMin, BoxMax, kSelectedPaletteColor );
			}
		}
	}

	if( m_SubTool == EST_Spawners )
	{
		// Draw categories
		const uint NumCategoryButtons = m_SpawnerCategoryButtons.Size();
		for( uint CategoryButtonIndex = 0; CategoryButtonIndex < NumCategoryButtons; ++CategoryButtonIndex )
		{
			const SSpawnerCategoryButton& CategoryButton = m_SpawnerCategoryButtons[ CategoryButtonIndex ];
			pRenderer->AddMesh( CategoryButton.m_Mesh );

			// Draw a box around the selected category
			if( m_CurrentSpawnerCategoryIndex == CategoryButton.m_SpawnerCategoryIndex )
			{
				const Vector BoxMin( CategoryButton.m_BoxMin.x, 0.0f, CategoryButton.m_BoxMin.y );
				const Vector BoxMax( CategoryButton.m_BoxMax.x, 0.0f, CategoryButton.m_BoxMax.y );
				pRenderer->DEBUGDrawBox2D( BoxMin, BoxMax, kSelectedPaletteColor );
			}
		}

		const uint NumSpawnerDefs = m_SpawnerDefs.Size();
		for( uint SpawnerDefIndex = 0; SpawnerDefIndex < NumSpawnerDefs; ++SpawnerDefIndex )
		{
			// Draw only spawners in the selected category
			const uint CategoryIndex = m_SpawnerCategoryMap[ SpawnerDefIndex ];
			if( CategoryIndex == m_CurrentSpawnerCategoryIndex )
			{
				const SSpawnerButton& SpawnerButton = m_SpawnerButtons[ SpawnerDefIndex ];
				pRenderer->AddMesh( SpawnerButton.m_Mesh );

				// Draw a box around the selected spawner
				if( m_CurrentSpawnerDefIndex == SpawnerButton.m_SpawnerDefIndex )
				{
					const Vector BoxMin( SpawnerButton.m_BoxMin.x, 0.0f, SpawnerButton.m_BoxMin.y );
					const Vector BoxMax( SpawnerButton.m_BoxMax.x, 0.0f, SpawnerButton.m_BoxMax.y );
					pRenderer->DEBUGDrawBox2D( BoxMin, BoxMax, kSelectedPaletteColor );
				}
			}
		}
	}
}

void EldritchTools::TryQuickSave()
{
	if( m_CurrentMapName == "" )
	{
		TrySave();
		return;
	}

	const SimpleString Message = SimpleString::PrintF( "Save %s?", m_CurrentMapName.CStr() );
	const int Response = MessageBox( NULL, Message.CStr(), Message.CStr(), MB_YESNO | MB_TASKMODAL | MB_SETFOREGROUND );
	if( Response == IDYES )
	{
		Save( FileStream( m_CurrentMapName.CStr(), FileStream::EFM_Write ) );
	}
}

void EldritchTools::TrySave()
{
	SimpleString SaveFileName;
	if( FileUtil::GetSaveFile( m_Framework->GetWindow()->GetHWnd(), "Eldritch Room Files", "eldritchroom", SaveFileName ) )
	{
		m_CurrentMapName = SaveFileName;
		Save( FileStream( SaveFileName.CStr(), FileStream::EFM_Write ) );
	}
}

void EldritchTools::TryLoad()
{
	SimpleString LoadFileName;
	if( FileUtil::GetLoadFile( m_Framework->GetWindow()->GetHWnd(), "Eldritch Room Files", "eldritchroom", LoadFileName ) )
	{
		Load( FileStream( LoadFileName.CStr(), FileStream::EFM_Read ) );
		m_CurrentMapName = LoadFileName;
	}
}

void EldritchTools::Save( const IDataStream& Stream )
{
	EldritchRoom TempRoom;

	TempRoom.m_RoomScalarX	= m_RoomScalarX;
	TempRoom.m_RoomScalarY	= m_RoomScalarY;
	TempRoom.m_RoomScalarZ	= m_RoomScalarZ;
	TempRoom.m_VoxelMap		= m_VoxelMap;

	FOR_EACH_MAP( SpawnerIter, m_Spawners, vidx_t, SPlacedSpawner )
	{
		EldritchRoom::SSpawner& Spawner = TempRoom.m_Spawners.PushBack();
		Spawner.m_Location		= SpawnerIter.GetKey();

		const SPlacedSpawner& PlacedSpawner = SpawnerIter.GetValue();
		Spawner.m_SpawnerDef	= m_SpawnerDefs[ PlacedSpawner.m_SpawnerDef ];
		Spawner.m_Orientation	= PlacedSpawner.m_Orientation;
	}

	TempRoom.Save( Stream );
}

void EldritchTools::Load( const IDataStream& Stream )
{
	EldritchRoom TempRoom;

	TempRoom.Load( Stream );

	Reinitialize( TempRoom.m_RoomScalarX, TempRoom.m_RoomScalarY, TempRoom.m_RoomScalarZ );

	m_VoxelMap = TempRoom.m_VoxelMap;
	RefreshVoxelMeshes();

	m_Spawners.Clear();
	for( uint SpawnerIndex = 0; SpawnerIndex < TempRoom.m_Spawners.Size(); ++SpawnerIndex )
	{
		const EldritchRoom::SSpawner& Spawner = TempRoom.m_Spawners[ SpawnerIndex ];

		uint SpawnerDefIndex = 0xffffffff;
		if( m_SpawnerDefs.Find( Spawner.m_SpawnerDef, SpawnerDefIndex ) )
		{
			SPlacedSpawner& PlacedSpawner	= m_Spawners[ Spawner.m_Location ];
			PlacedSpawner.m_SpawnerDef		= SpawnerDefIndex;
			PlacedSpawner.m_Orientation		= Spawner.m_Orientation;
		}
		else
		{
			MAKEHASHFROM( OriginalSpawnerDef, Spawner.m_SpawnerDef );

			STATICHASH( RemappedDef );
			const SimpleString RemappedSpawnerDef = ConfigManager::GetString( sRemappedDef, "", sOriginalSpawnerDef );

			STATICHASH( RemapOrientation );
			const bool RemapOrientation = ConfigManager::GetBool( sRemapOrientation, false, sOriginalSpawnerDef );

			STATICHASH( RemappedOrientation );
			STATIC_HASHED_STRING( North );
			const HashedString RemappedOrientation = ConfigManager::GetHash( sRemappedOrientation, sNorth, sOriginalSpawnerDef );

			if( m_SpawnerDefs.Find( RemappedSpawnerDef, SpawnerDefIndex ) )
			{
				SPlacedSpawner& PlacedSpawner	= m_Spawners[ Spawner.m_Location ];
				PlacedSpawner.m_SpawnerDef		= SpawnerDefIndex;
				PlacedSpawner.m_Orientation		= RemapOrientation ? GetOrientation( RemappedOrientation ) : Spawner.m_Orientation;
			}
			else
			{
				WARNDESC( "Unknown and unremapped spawner loaded; if you resave this map, the spawners will be lost." );
			}
		}
	}
}

float EldritchTools::GetYaw( const uint8 Orientation ) const
{
	if( Orientation == ESO_North )	{ return 0.0f; }
	if( Orientation == ESO_South )	{ return DEGREES_TO_RADIANS( 180.0f ); }
	if( Orientation == ESO_East )	{ return DEGREES_TO_RADIANS( 270.0f ); }
	if( Orientation == ESO_West )	{ return DEGREES_TO_RADIANS( 90.0f ); }

	WARN;
	return 0.0f;
}

uint8 EldritchTools::GetOrientation( const HashedString& Orientation ) const
{
	STATIC_HASHED_STRING( North );
	STATIC_HASHED_STRING( N );
	STATIC_HASHED_STRING( South );
	STATIC_HASHED_STRING( S );
	STATIC_HASHED_STRING( East );
	STATIC_HASHED_STRING( E );
	STATIC_HASHED_STRING( West );
	STATIC_HASHED_STRING( W );

	if( Orientation == sNorth	|| Orientation == sN )	{ return ESO_North; }
	if( Orientation == sSouth	|| Orientation == sS )	{ return ESO_South; }
	if( Orientation == sEast	|| Orientation == sE )	{ return ESO_East; }
	if( Orientation == sWest	|| Orientation == sW )	{ return ESO_West; }

	WARN;
	return ESO_North;
}

#endif // BUILD_DEV