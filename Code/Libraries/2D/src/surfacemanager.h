#ifndef SURFACEMANAGER_H
#define SURFACEMANAGER_H

#include "map.h"
#include "surface.h"

class SurfaceManager
{
public:
	static SurfaceManager* GetInstance();
	static void DeleteInstance();

	Surface* GetSurface( const char* Filename, Surface::ESurfaceFileType FileType );

private:
	SurfaceManager();
	~SurfaceManager();

	static SurfaceManager*	m_Instance;

	typedef Map< HashedString, Surface* > SurfaceMap;
	SurfaceMap	m_Surfaces;
};

#endif // SURFACEMANAGER_H