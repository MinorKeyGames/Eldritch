#include "core.h"
#include "wbcompeldminimap.h"
#include "wbcompeldtransform.h"
#include "idatastream.h"
#include "meshfactory.h"
#include "eldritchworld.h"
#include "eldritchframework.h"
#include "irenderer.h"
#include "texturemanager.h"
#include "shadermanager.h"
#include "wbentity.h"
#include "mesh.h"
#include "configmanager.h"
#include "wbeventmanager.h"
#include "mathcore.h"
#include "ivertexdeclaration.h"
#include "ivertexbuffer.h"

WBCompEldMinimap::WBCompEldMinimap()
:	m_RoomStatus()
,	m_ShowingAllMarkers( false )
,	m_Markers()
,	m_MinimapRoomMeshes()
,	m_MinimapFloorMeshes()
,	m_LastAtlasIndices()
,	m_ActiveRoomIndex( -1 )
,	m_ActiveFloor( 0 )
,	m_RoomAtlasTexture( NULL )
,	m_Material()
{
	WBEventManager* const pEventManager = GetEventManager();
	ASSERT( pEventManager );

	STATIC_HASHED_STRING( AddMapMarker );
	pEventManager->AddObserver( sAddMapMarker, this );

	STATIC_HASHED_STRING( UpdateMapMarker );
	pEventManager->AddObserver( sUpdateMapMarker, this );

	STATIC_HASHED_STRING( RemoveMapMarker );
	pEventManager->AddObserver( sRemoveMapMarker, this );
}

WBCompEldMinimap::~WBCompEldMinimap()
{
	DeleteMinimapRoomMeshes();

	FOR_EACH_MAP( MarkerIter, m_Markers, WBEntity*, SMarker )
	{
		SMarker& Marker = MarkerIter.GetValue();
		SafeDelete( Marker.m_Mesh );
	}

	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( AddMapMarker );
		pEventManager->RemoveObserver( sAddMapMarker, this );

		STATIC_HASHED_STRING( UpdateMapMarker );
		pEventManager->RemoveObserver( sUpdateMapMarker, this );

		STATIC_HASHED_STRING( RemoveMapMarker );
		pEventManager->RemoveObserver( sRemoveMapMarker, this );
	}
}

/*virtual*/ void WBCompEldMinimap::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	EldritchFramework* const	pFramework	= GetFramework();
	ASSERT( pFramework );

	IRenderer* const			pRenderer	= pFramework->GetRenderer();
	ASSERT( pRenderer );

	STATICHASH( EldMinimap );

	STATICHASH( RoomAtlasTexture );
	const SimpleString RoomAtlasTexture = ConfigManager::GetString( sRoomAtlasTexture, "", sEldMinimap );
	m_RoomAtlasTexture = pRenderer->GetTextureManager()->GetTexture( RoomAtlasTexture.CStr() );
	ASSERT( m_RoomAtlasTexture );

	STATICHASH( Material );
	const SimpleString Material = ConfigManager::GetString( sMaterial, "", sEldMinimap );
	m_Material.SetDefinition( Material, pRenderer, pRenderer->GetVertexDeclaration( VD_POSITIONS | VD_UVS ) );

	EldritchWorld* const		pWorld		= GetWorld();
	m_RoomStatus.ResizeZero( pWorld->GetMapSizeX() * pWorld->GetMapSizeY() * pWorld->GetMapSizeZ() );

	CreateMinimapRoomMeshes();
}

/*virtual*/ void WBCompEldMinimap::HandleEvent( const WBEvent& Event )
{
	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( AddMapMarker );
	STATIC_HASHED_STRING( UpdateMapMarker );
	STATIC_HASHED_STRING( RemoveMapMarker );
	STATIC_HASHED_STRING( ShowAllMapMarkers );
	STATIC_HASHED_STRING( HideAllMapMarkers );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sAddMapMarker )
	{
		STATIC_HASHED_STRING( EventOwner );
		WBEntity* const pEntity = Event.GetEntity( sEventOwner );

		STATIC_HASHED_STRING( Texture );
		const SimpleString Texture = Event.GetString( sTexture );

		AddMarker( pEntity, Texture );
	}
	else if( EventName == sUpdateMapMarker )
	{
		STATIC_HASHED_STRING( EventOwner );
		WBEntity* const pEntity = Event.GetEntity( sEventOwner );

		STATIC_HASHED_STRING( Location );
		const Vector Location = Event.GetVector( sLocation );

		STATIC_HASHED_STRING( Orientation );
		const Angles Orientation = Event.GetAngles( sOrientation );

		STATIC_HASHED_STRING( MarkAsRoom );
		const bool MarkAsRoom = Event.GetBool( sMarkAsRoom );

		UpdateMarker( pEntity, Location, Orientation, MarkAsRoom );
	}
	else if( EventName == sRemoveMapMarker )
	{
		STATIC_HASHED_STRING( EventOwner );
		WBEntity* const pEntity = Event.GetEntity( sEventOwner );

		RemoveMarker( pEntity );
	}
	else if( EventName == sShowAllMapMarkers )
	{
		m_ShowingAllMarkers = true;
	}
	else if( EventName == sHideAllMapMarkers )
	{
		m_ShowingAllMarkers = false;
	}
}

/*virtual*/ void WBCompEldMinimap::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	Unused( DeltaTime );

	EldritchWorld* const		pWorld				= GetWorld();
	ASSERT( pWorld );

	WBEntity* const				pEntity				= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform			= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector				Location			= pTransform->GetLocation();
	const int					CurrentRoomIndex	= pWorld->GetRoomIndex( Location );

	if( CurrentRoomIndex != m_ActiveRoomIndex )
	{
		int RoomX, RoomY, RoomZ;
		pWorld->GetRoomCoords( CurrentRoomIndex, RoomX, RoomY, RoomZ );

		const int OldRoomIndex	= m_ActiveRoomIndex;
		m_ActiveRoomIndex		= CurrentRoomIndex;
		m_ActiveFloor			= RoomZ;

		// Update room status
		if( OldRoomIndex >= 0 )
		{
			m_RoomStatus[ OldRoomIndex ]	= ERS_Inactive;
		}
		m_RoomStatus[ m_ActiveRoomIndex ]	= ERS_Active;

		// Update minimap textures from status
		UpdateMinimapRoomMeshes();
	}
}

void WBCompEldMinimap::UpdateMinimapRoomMeshes()
{
	XTRACE_FUNCTION;

	uint			RoomIndex	= 0;
	const uint		NumFloors	= m_MinimapFloorMeshes.Size();
	EldritchWorld*	pWorld		= GetWorld();

	for( uint FloorIndex = 0; FloorIndex < NumFloors; ++FloorIndex )
	{
		const bool IsActiveFloor = ( FloorIndex == m_ActiveFloor );

		const Array<Mesh*>& FloorMeshes = m_MinimapFloorMeshes[ FloorIndex ];
		FOR_EACH_ARRAY( MeshIter, FloorMeshes, Mesh* )
		{
			Mesh* const pMesh = MeshIter.GetValue();
			ASSERT( pMesh );

			const uint		RoomStatus		= m_RoomStatus[ RoomIndex ];
			const uint		RoomExits		= pWorld->GetMazeValue( RoomIndex );
			const uint		AtlasIndex		= GetAtlasIndex( RoomExits, RoomStatus, IsActiveFloor );

			const uint		LastAtlasIndex	= m_LastAtlasIndices[ RoomIndex ];
			if( LastAtlasIndex != AtlasIndex )
			{
				XTRACE_NAMED( RebuildRoomMesh );

				// HACK: Rebuild mesh in place and reassign properties.
				m_LastAtlasIndices[ RoomIndex ] = AtlasIndex;
				const Vector LastLocation = pMesh->m_Location;

				CreateQuad( AtlasIndex, pMesh );

				pMesh->m_Location		= LastLocation;
				pMesh->m_Material	= m_Material;
				pMesh->SetMaterialFlags( MAT_OFFSCREEN_1 );
				pMesh->SetTexture( 0, m_RoomAtlasTexture );
			}

			++RoomIndex;
		}
	}
}

void WBCompEldMinimap::DeleteMinimapRoomMeshes()
{
	FOR_EACH_ARRAY( MeshIter, m_MinimapRoomMeshes, Mesh* )
	{
		Mesh* pMesh = MeshIter.GetValue();
		ASSERT( pMesh );

		SafeDelete( pMesh );
	}

	m_MinimapRoomMeshes.Clear();
	m_MinimapFloorMeshes.Clear();
}

uint WBCompEldMinimap::GetAtlasIndex( const uint RoomExits, const uint RoomStatus, const bool ActiveFloor ) const
{
	// Atlas is arranged as a 2x2 map of 8x8 tiles, ordered: inactive floor (0), hidden (1), inactive (2), active (3)
	const uint RoomStyle	= ActiveFloor ? ( RoomStatus + 1 ) : 0;
	const uint RoomStyleX	= RoomStyle % 2;
	const uint RoomStyleY	= RoomStyle / 2;

	const uint TileIndex	= RoomExits;
	const uint TileIndexX	= TileIndex % 8;
	const uint TileIndexY	= TileIndex / 8;

	const uint AtlasIndexX	= ( RoomStyleX * 8 ) + TileIndexX;
	const uint AtlasIndexY	= ( RoomStyleY * 8 ) + TileIndexY;
	const uint AtlasIndex	= ( AtlasIndexY * 16 ) + AtlasIndexX;

	return AtlasIndex;
}

Mesh* WBCompEldMinimap::CreateQuad( const uint AtlasIndex, Mesh* const pMesh ) const
{
	XTRACE_FUNCTION;

	static const int	kNumVertices	= 4;
	static const int	kNumIndices	= 6;

	Array<Vector> Positions;
	Positions.Resize( kNumVertices );

	Array<Vector2> UVs;
	UVs.Resize( kNumVertices );

	Array<index_t> Indices;
	Indices.Resize( kNumIndices );

	Positions[0]	= Vector( -0.5f, -0.5f, 0.0f );
	Positions[1]	= Vector( 0.5f, -0.5f, 0.0f );
	Positions[2]	= Vector( -0.5f, 0.5f, 0.0f );
	Positions[3]	= Vector( 0.5f, 0.5f, 0.0f );

	static const float	kAtlasStep	= 1.0f / 16.0f;
	const uint	AtlasIndexX	= AtlasIndex % 16;
	const uint	AtlasIndexY	= AtlasIndex / 16;
	const float	U0			= static_cast<float>( AtlasIndexX ) / 16.0f;
	const float	V0			= static_cast<float>( AtlasIndexY ) / 16.0f;
	const float	U1			= U0 + kAtlasStep;
	const float V1			= V0 + kAtlasStep;

	UVs[0]			= Vector2( U0, V1 );
	UVs[1]			= Vector2( U1, V1 );
	UVs[2]			= Vector2( U0, V0 );
	UVs[3]			= Vector2( U1, V0 );

	Indices[0]		= 0;
	Indices[1]		= 1;
	Indices[2]		= 2;
	Indices[3]		= 1;
	Indices[4]		= 3;
	Indices[5]		= 2;

	EldritchFramework* const	pFramework	= GetFramework();
	ASSERT( pFramework );

	IRenderer* const			pRenderer	= pFramework->GetRenderer();
	ASSERT( pRenderer );

	IVertexBuffer*		pVertexBuffer		= pRenderer->CreateVertexBuffer();
	IVertexDeclaration*	pVertexDeclaration	= pRenderer->GetVertexDeclaration( VD_POSITIONS | VD_UVS );
	IIndexBuffer*		pIndexBuffer		= pRenderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices	= kNumVertices;
	InitStruct.Positions	= Positions.GetData();
	InitStruct.UVs			= UVs.GetData();
	pVertexBuffer->Init( InitStruct );
	pIndexBuffer->Init( kNumIndices, Indices.GetData() );
	pIndexBuffer->SetPrimitiveType( EPT_TRIANGLELIST );

	Mesh* pQuadMesh = NULL;
	if( pMesh )
	{
		// HACK: Apparently deleting and allocating in place doesn't play nice with my allocator?
		// So instead, destruct and new over it.
		pMesh->~Mesh();
		pQuadMesh = new( pMesh ) Mesh( pVertexBuffer, pVertexDeclaration, pIndexBuffer );
	}
	else
	{
		pQuadMesh = new Mesh( pVertexBuffer, pVertexDeclaration, pIndexBuffer );
	}

	pQuadMesh->m_AABB = AABB( Vector( -0.5f, -0.5f, 0.0f ), Vector( 0.5f, 0.5f, 0.0f ) );

#if BUILD_DEBUG
	pQuadMesh->m_Name = "MinimapQuad";
#endif

	return pQuadMesh;
}

void WBCompEldMinimap::CreateMinimapRoomMeshes()
{
	XTRACE_FUNCTION;

	DeleteMinimapRoomMeshes();

	EldritchWorld* const		pWorld		= GetWorld();
	ASSERT( pWorld );

	EldritchFramework* const	pFramework	= GetFramework();
	ASSERT( pFramework );

	IRenderer* const			pRenderer	= pFramework->GetRenderer();
	ASSERT( pRenderer );

	const int		MapSizeX	= pWorld->GetMapSizeX();
	const int		MapSizeY	= pWorld->GetMapSizeY();
	const int		MapSizeZ	= pWorld->GetMapSizeZ();

	const float		MapOffsetX	= 0.5f * static_cast<float>( 1 - MapSizeX );
	const float		MapOffsetY	= 0.5f * static_cast<float>( 1 - MapSizeY );
	const float		MapOffsetZ	= 0.5f * static_cast<float>( 1 - MapSizeZ );
	const Vector	MapOffset	= Vector( MapOffsetX, MapOffsetY, MapOffsetZ );

	// Draw floors bottom to top
	for( int RoomZ = 0; RoomZ < MapSizeZ; ++RoomZ )
	{
		Array<Mesh*>& FloorMeshes = m_MinimapFloorMeshes.PushBack();

		for( int RoomY = 0; RoomY < MapSizeY; ++RoomY )
		{
			for( int RoomX = 0; RoomX < MapSizeX; ++RoomX )
			{
				Mesh* const pRoomMesh = CreateQuad( 0, NULL );

				const float		fRoomX		= static_cast<float>( RoomX );
				const float		fRoomY		= static_cast<float>( RoomY );
				const float		fRoomZ		= static_cast<float>( RoomZ );
				const Vector	RoomVector	= Vector( fRoomX, fRoomY, fRoomZ );
				pRoomMesh->m_Location		= RoomVector + MapOffset;

				pRoomMesh->m_Material	= m_Material;
				pRoomMesh->SetMaterialFlags( MAT_OFFSCREEN_1 );
				pRoomMesh->SetTexture( 0, m_RoomAtlasTexture );

				m_MinimapRoomMeshes.PushBack( pRoomMesh );
				FloorMeshes.PushBack( pRoomMesh );

				m_LastAtlasIndices.PushBack( 0xffffffff );
			}
		}
	}

	ASSERT( m_LastAtlasIndices.Size() == m_MinimapRoomMeshes.Size() );
}

void WBCompEldMinimap::AddMarker( WBEntity* const pEntity, const SimpleString& Texture )
{
	DEVASSERT( pEntity );
	ASSERT( m_Markers.Search( pEntity ).IsNull() );

	EldritchFramework* const	pFramework			= GetFramework();
	ASSERT( pFramework );

	IRenderer* const			pRenderer			= pFramework->GetRenderer();
	ASSERT( pRenderer );

	// CreateQuad gives us a mesh with color, normal, tangent, etc., and we don't want all that.
	// (In fact, it screws up GL rendering by binding the wrong attributes.)
	IVertexDeclaration* const	pVertexDeclaration	= pRenderer->GetVertexDeclaration( VD_POSITIONS | VD_UVS );

	SMarker& Marker = m_Markers[ pEntity ];
	Marker.m_Mesh = pRenderer->GetMeshFactory()->CreateQuad( 1.0f, XY_PLANE, false );
	Marker.m_Mesh->m_Material	= m_Material;
	Marker.m_Mesh->SetVertexDeclaration( pVertexDeclaration );
	Marker.m_Mesh->SetMaterialFlags( MAT_OFFSCREEN_1 );
	Marker.m_Mesh->SetTexture( 0, pRenderer->GetTextureManager()->GetTexture( Texture.CStr() ) );
}

void WBCompEldMinimap::UpdateMarker( WBEntity* const pEntity, const Vector& Location, const Angles& Orientation, const bool MarkAsRoom )
{
	if( m_Markers.Search( pEntity ).IsNull() )
	{
		return;
	}

	EldritchWorld* const		pWorld		= GetWorld();
	ASSERT( pWorld );

	const int		RoomSizeX	= pWorld->GetRoomSizeX();
	const int		RoomSizeY	= pWorld->GetRoomSizeY();
	const int		RoomSizeZ	= pWorld->GetRoomSizeZ();
	const float		RoomScaleX	= 1.0f / static_cast<float>( RoomSizeX );
	const float		RoomScaleY	= 1.0f / static_cast<float>( RoomSizeY );
	const float		RoomScaleZ	= 1.0f / static_cast<float>( RoomSizeZ );
	const Vector	RoomScalar	= Vector( RoomScaleX, RoomScaleY, RoomScaleZ );
	const Vector	RoomOffset	= Vector( -1.0f, -1.0f, -1.0f );	// Offset for indestructible world bounds

	const int		MapSizeX	= pWorld->GetMapSizeX();
	const int		MapSizeY	= pWorld->GetMapSizeY();
	const int		MapSizeZ	= pWorld->GetMapSizeZ();
	const float		MapOffsetX	= -0.5f * static_cast<float>( MapSizeX );
	const float		MapOffsetY	= -0.5f * static_cast<float>( MapSizeY );
	const float		MapOffsetZ	= 0.5f * static_cast<float>( 1 - MapSizeZ );
	const Vector	MapOffset	= Vector( MapOffsetX, MapOffsetY, 0.0f );

	Vector			MapLocation	= ( ( Location + RoomOffset ) * RoomScalar ) + MapOffset;
	const float		fFloor		= Floor( MapLocation.z );
	MapLocation.z				= fFloor + MapOffsetZ;

	SMarker& Marker				= m_Markers[ pEntity ];
	Marker.m_MarkAsRoom			= MarkAsRoom;
	Marker.m_RoomIndex			= pWorld->GetRoomIndex( Location );
	Marker.m_Floor				= static_cast<uint>( fFloor );
	if( MarkAsRoom )
	{
		Vector QuantizedLocation = MapLocation;
		QuantizedLocation.x = Floor( QuantizedLocation.x ) + 0.5f;
		QuantizedLocation.y = Floor( QuantizedLocation.y ) + 0.5f;

		Marker.m_Mesh->m_Location = QuantizedLocation;
		Marker.m_Mesh->m_Rotation.Zero();
	}
	else
	{
		Marker.m_Mesh->m_Location = MapLocation;
		Marker.m_Mesh->m_Rotation = Angles( 0.0f, 0.0f, Orientation.Yaw );
	}
}

void WBCompEldMinimap::RemoveMarker( WBEntity* const pEntity )
{
	ASSERT( m_Markers.Search( pEntity ).IsValid() );

	m_Markers.Remove( pEntity );
}

/*virtual*/ void WBCompEldMinimap::Render()
{
	XTRACE_FUNCTION;

	EldritchFramework* const	pFramework	= GetFramework();
	ASSERT( pFramework );

	IRenderer* const			pRenderer	= pFramework->GetRenderer();
	ASSERT( pRenderer );

	const uint					NumFloors	= m_MinimapFloorMeshes.Size();

	// Render floors the player is not on first
	for( uint FloorIndex = 0; FloorIndex < NumFloors; ++FloorIndex )
	{
		if( FloorIndex == m_ActiveFloor )
		{
			continue;
		}

		const Array<Mesh*>& FloorMeshes = m_MinimapFloorMeshes[ FloorIndex ];
		FOR_EACH_ARRAY( MeshIter, FloorMeshes, Mesh* )
		{
			Mesh* const pMesh = MeshIter.GetValue();
			ASSERT( pMesh );

			pRenderer->AddMesh( pMesh );
		}
	}

	// Then render floor the player is on
	const Array<Mesh*>& FloorMeshes = m_MinimapFloorMeshes[ m_ActiveFloor ];
	FOR_EACH_ARRAY( MeshIter, FloorMeshes, Mesh* )
	{
		Mesh* const pMesh = MeshIter.GetValue();
		ASSERT( pMesh );

		pRenderer->AddMesh( pMesh );
	}

	// Render room markers for the floor the player is on
	FOR_EACH_MAP( MarkerIter, m_Markers, WBEntity*, SMarker )
	{
		const SMarker& Marker = MarkerIter.GetValue();

		if( Marker.m_Hidden ||
			!Marker.m_MarkAsRoom ||
			Marker.m_Floor != m_ActiveFloor )
		{
			continue;
		}

		const int RoomStatus = m_RoomStatus[ Marker.m_RoomIndex ];
		if( RoomStatus == ERS_Hidden && !m_ShowingAllMarkers )
		{
			continue;
		}

		pRenderer->AddMesh( Marker.m_Mesh );
	}

	// Render non-room markers for the floor the player is on
	FOR_EACH_MAP( MarkerIter, m_Markers, WBEntity*, SMarker )
	{
		const SMarker& Marker = MarkerIter.GetValue();

		if( Marker.m_Hidden ||
			Marker.m_MarkAsRoom ||
			Marker.m_Floor != m_ActiveFloor )
		{
			continue;
		}

		const int RoomStatus = m_RoomStatus[ Marker.m_RoomIndex ];
		if( RoomStatus == ERS_Hidden && !m_ShowingAllMarkers )
		{
			continue;
		}

		pRenderer->AddMesh( Marker.m_Mesh );
	}
}

#define VERSION_EMPTY				0
#define VERSION_ROOMSTATUS			1
#define VERSION_SHOWINGALLMARKERS	2
#define VERSION_ACTIVEROOMINDEX		3
#define VERSION_ACTIVEFLOOR			4
#define VERSION_CURRENT				4

uint WBCompEldMinimap::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;							// Version
	Size += 4;							// m_RoomStatus.Size()
	Size += m_RoomStatus.MemorySize();
	Size += 1;							// m_ShowingAllMarkers
	Size += 4;							// m_ActiveRoomIndex
	Size += 4;							// m_ActiveFloor

	return Size;
}

void WBCompEldMinimap::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteUInt32( m_RoomStatus.Size() );
	Stream.Write( m_RoomStatus.MemorySize(), m_RoomStatus.GetData() );

	Stream.WriteBool( m_ShowingAllMarkers );

	Stream.WriteInt32( m_ActiveRoomIndex );

	Stream.WriteUInt32( m_ActiveFloor );
}

void WBCompEldMinimap::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_ROOMSTATUS )
	{
		m_RoomStatus.Resize( Stream.ReadUInt32() );
		Stream.Read( m_RoomStatus.MemorySize(), m_RoomStatus.GetData() );
	}

	if( Version >= VERSION_SHOWINGALLMARKERS )
	{
		m_ShowingAllMarkers = Stream.ReadBool();
	}

	if( Version >= VERSION_ACTIVEROOMINDEX )
	{
		m_ActiveRoomIndex = Stream.ReadInt32();
	}

	if( Version >= VERSION_ACTIVEFLOOR )
	{
		m_ActiveFloor = Stream.ReadUInt32();
	}

	UpdateMinimapRoomMeshes();
}