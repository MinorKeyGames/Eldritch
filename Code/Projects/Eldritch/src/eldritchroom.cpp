#include "core.h"
#include "eldritchroom.h"
#include "idatastream.h"

EldritchRoom::EldritchRoom()
:	m_RoomScalarX( 0 )
,	m_RoomScalarY( 0 )
,	m_RoomScalarZ( 0 )
,	m_VoxelMap()
,	m_Spawners()
{
}

EldritchRoom::~EldritchRoom()
{
}

void EldritchRoom::GetRoomScalars( uint& X, uint& Y, uint& Z ) const
{
	X = m_RoomScalarX;
	Y = m_RoomScalarY;
	Z = m_RoomScalarZ;
}

#define VERSION_EMPTY				0
#define VERSION_VOXELS				1
#define VERSION_ROOMSCALARS			2
#define VERSION_SPAWNERS			3
#define VERSION_SPAWNER_ORIENTATION	4
#define VERSION_CURRENT				4

void EldritchRoom::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteUInt32( m_RoomScalarX );
	Stream.WriteUInt32( m_RoomScalarY );
	Stream.WriteUInt32( m_RoomScalarZ );

	Stream.WriteUInt32( m_VoxelMap.Size() );
	Stream.Write( m_VoxelMap.MemorySize(), m_VoxelMap.GetData() );

	Stream.WriteUInt32( m_Spawners.Size() );
	for( uint SpawnerIndex = 0; SpawnerIndex < m_Spawners.Size(); ++SpawnerIndex )
	{
		const SSpawner& Spawner = m_Spawners[ SpawnerIndex ];
		Stream.WriteString( Spawner.m_SpawnerDef );
		Stream.WriteInt32( Spawner.m_Location );
		Stream.WriteUInt8( Spawner.m_Orientation );
	}
}

void EldritchRoom::Load( const IDataStream& Stream )
{
	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_ROOMSCALARS )
	{
		m_RoomScalarX = Stream.ReadUInt32();
		m_RoomScalarY = Stream.ReadUInt32();
		m_RoomScalarZ = Stream.ReadUInt32();
	}

	if( Version >= VERSION_VOXELS )
	{
		const uint NumVoxels = Stream.ReadUInt32();
		m_VoxelMap.Resize( NumVoxels );
		Stream.Read( m_VoxelMap.MemorySize(), m_VoxelMap.GetData() );
	}

	if( Version >= VERSION_SPAWNERS )
	{
		const uint NumSpawners = Stream.ReadUInt32();
		m_Spawners.Clear();
		m_Spawners.Reserve( NumSpawners );
		for( uint SpawnerIndex = 0; SpawnerIndex < NumSpawners; ++SpawnerIndex )
		{
			SSpawner& Spawner = m_Spawners.PushBack();
			Spawner.m_SpawnerDef = Stream.ReadString();
			Spawner.m_Location = Stream.ReadInt32();

			if( Version >= VERSION_SPAWNER_ORIENTATION )
			{
				Spawner.m_Orientation = Stream.ReadUInt8();
			}
		}
	}
}