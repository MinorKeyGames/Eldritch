#include "core.h"
#include "3d.h"
#if BUILD_WINDOWS_NO_SDL
#include "D3D9/d3d9renderer.h"
#endif
#include "GL2/gl2renderer.h"

#if BUILD_WINDOWS_NO_SDL
IRenderer* CreateD3D9Renderer( HWND hWnd, bool Fullscreen )
{
	return new D3D9Renderer( hWnd, Fullscreen );
}
#endif

IRenderer* CreateGL2Renderer( Window* const pWindow )
{
	return new GL2Renderer( pWindow );
}