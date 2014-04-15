#ifndef ELDRITCHROOM_H
#define ELDRITCHROOM_H

// Small (hopefully) class that contains a random room definition.

#include "eldritchworld.h"	// For voxel types

class IDataStream;

class EldritchRoom
{
public:
	EldritchRoom();
	~EldritchRoom();

	struct SSpawner
	{
		SSpawner()
		:	m_SpawnerDef()
		,	m_Location( 0 )
		,	m_Orientation( 0 )
		{
		}

		SimpleString	m_SpawnerDef;
		vidx_t			m_Location;
		uint8			m_Orientation;	// See ESpawnerOrientation
	};

	void	Save( const IDataStream& Stream );
	void	Load( const IDataStream& Stream );

	void					GetRoomScalars( uint& X, uint& Y, uint& Z ) const;
	const Array<vval_t>&	GetVoxelMap() const { return m_VoxelMap; }
	const Array<SSpawner>&	GetSpawners() const { return m_Spawners; }

private:
	friend class EldritchTools;

	uint			m_RoomScalarX;
	uint			m_RoomScalarY;
	uint			m_RoomScalarZ;
	Array<vval_t>	m_VoxelMap;

	Array<SSpawner>	m_Spawners;
};

#endif // ELDRITCHROOM_H