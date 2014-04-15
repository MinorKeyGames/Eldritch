#ifndef WBCOMPELDMINIMAP_H
#define WBCOMPELDMINIMAP_H

#include "wbeldritchcomponent.h"
#include "array.h"
#include "map.h"
#include "material.h"

class Mesh;
class ITexture;
class Vector;
class Angles;

class WBCompEldMinimap : public WBEldritchComponent
{
public:
	WBCompEldMinimap();
	virtual ~WBCompEldMinimap();

	DEFINE_WBCOMP( EldMinimap, WBEldritchComponent );

	virtual void	Tick( float DeltaTime );
	virtual int		GetTickOrder() { return ETO_TickDefault; }	// Needs to tick after transform

	virtual void	HandleEvent( const WBEvent& Event );

	virtual bool	IsRenderable() { return true; }
	virtual void	Render();

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			UpdateMinimapRoomMeshes();
	void			CreateMinimapRoomMeshes();
	void			DeleteMinimapRoomMeshes();

	void			AddMarker( WBEntity* const pEntity, const SimpleString& Texture );
	void			UpdateMarker( WBEntity* const pEntity, const Vector& Location, const Angles& Orientation, const bool MarkAsRoom );
	void			RemoveMarker( WBEntity* const pEntity );

	uint			GetAtlasIndex( const uint RoomExits, const uint RoomStatus, const bool ActiveFloor ) const;
	Mesh*			CreateQuad( const uint AtlasIndex, Mesh* const pMesh ) const;

	struct SMarker
	{
		SMarker()
		:	m_Mesh( NULL )
		,	m_MarkAsRoom( false )
		,	m_Hidden( false )
		,	m_RoomIndex( 0 )
		,	m_Floor( 0 )
		{
		}

		Mesh*	m_Mesh;
		bool	m_MarkAsRoom;
		bool	m_Hidden;
		uint	m_RoomIndex;
		uint	m_Floor;
	};

	enum ERoomStatus
	{
		ERS_Hidden,
		ERS_Inactive,
		ERS_Active,
	};

	Array<uint>				m_RoomStatus;			// Serialized
	bool					m_ShowingAllMarkers;	// Serialized

	Map<WBEntity*, SMarker>	m_Markers;				// Transient

	Array<Mesh*>			m_MinimapRoomMeshes;	// Transient
	Array<Array<Mesh*> >	m_MinimapFloorMeshes;	// Transient, same meshes as m_MinimapRoomMeshes but sorted by floor
	Array<uint>				m_LastAtlasIndices;		// Transient

	int						m_ActiveRoomIndex;		// Serialized
	uint					m_ActiveFloor;			// Serialized

	ITexture*				m_RoomAtlasTexture;		// Config
	Material				m_Material;			// Config
};

#endif // WBCOMPELDMINIMAP_H