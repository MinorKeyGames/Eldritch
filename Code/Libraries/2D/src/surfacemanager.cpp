#include "core.h"
#include "surfacemanager.h"
#include "surface.h"
#include "packstream.h"

SurfaceManager* SurfaceManager::m_Instance = NULL;

SurfaceManager::SurfaceManager()
:	m_Surfaces()
{
}

SurfaceManager::~SurfaceManager()
{
	FOR_EACH_MAP( SurfaceIter, m_Surfaces, HashedString, Surface* )
	{
		SafeDelete( *SurfaceIter );
	}
	m_Surfaces.Clear();
}

SurfaceManager* SurfaceManager::GetInstance()
{
	if( !m_Instance )
	{
		m_Instance = new SurfaceManager;
	}
	return m_Instance;
}

void SurfaceManager::DeleteInstance()
{
	SafeDelete( m_Instance );
}

Surface* SurfaceManager::GetSurface( const char* Filename, Surface::ESurfaceFileType FileType )
{
	SurfaceMap::Iterator SurfaceIter = m_Surfaces.Search( Filename );

	if( SurfaceIter.IsNull() )
	{
		Surface* pNewSurface = new Surface( PackStream( Filename ), FileType );
		SurfaceIter = m_Surfaces.Insert( Filename, pNewSurface );
	}

	return *SurfaceIter;
}