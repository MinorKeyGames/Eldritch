#include "core.h"
#include "3d.h"
#include "dynamicmeshmanager.h"
#include "mesh.h"
#include "irenderer.h"
#include "packstream.h"
#include "meshfactory.h"
#include "file.h"

#include <memory.h>

DynamicMeshManager* DynamicMeshManager::m_Instance = NULL;

DynamicMeshManager::DynamicMeshManager()
:	m_Meshes() {}

DynamicMeshManager::~DynamicMeshManager()
{
	FreeMeshes();
}

DynamicMeshManager* DynamicMeshManager::GetInstance()
{
	if( !m_Instance )
	{
		m_Instance = new DynamicMeshManager;
	}
	return m_Instance;
}

void DynamicMeshManager::DeleteInstance()
{
	SafeDelete( m_Instance );
}

void DynamicMeshManager::FreeMeshes()
{
	FOR_EACH_MAP( Iter, m_Meshes, HashedString, Mesh* )
	{
		SafeDelete( *Iter );
	}
	m_Meshes.Clear();
}

Mesh* DynamicMeshManager::GetMesh( MeshFactory* pFactory, const char* Filename )
{
	HashedString HashedFilename( Filename );

	if( !m_Meshes[ HashedFilename ] )
	{
		m_Meshes[ HashedFilename ] = pFactory->Read( PackStream( Filename ), Filename, NULL, NULL );
	}

	return m_Meshes[ HashedFilename ];
}