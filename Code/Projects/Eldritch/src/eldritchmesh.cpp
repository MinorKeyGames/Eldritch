#include "core.h"
#include "eldritchmesh.h"
#include "eldritchworld.h"
#include "eldritchframework.h"

EldritchMesh::EldritchMesh()
:	m_IrradianceCube()
{
}

EldritchMesh::~EldritchMesh()
{
}

void EldritchMesh::SetIrradianceCube( const SVoxelIrradiance& Irradiance )
{
	m_IrradianceCube = Irradiance + EldritchFramework::GetInstance()->GetWorld()->GetGlobalLight();
}

const SVoxelIrradiance& EldritchMesh::GetIrradianceCube() const
{
	return m_IrradianceCube;
}