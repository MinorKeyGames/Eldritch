#ifndef ELDRITCHMESH_H
#define ELDRITCHMESH_H

#include "mesh.h"
#include "eldritchirradiance.h"
#include "wbentity.h"

class EldritchMesh : public Mesh
{
public:
	EldritchMesh();
	virtual ~EldritchMesh();

	void					SetIrradianceCube( const SVoxelIrradiance& Irradiance );
	const SVoxelIrradiance&	GetIrradianceCube() const;

	void					SetEntity( WBEntity* const pEntity ) { m_Entity = pEntity; }
	WBEntity*				GetEntity() const { return m_Entity; }

private:
	SVoxelIrradiance	m_IrradianceCube;
	WBEntity*			m_Entity;
};

#endif // ELDRITCHMESH_H