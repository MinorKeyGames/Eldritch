#include "GL/glew.h"
#ifdef WIN32
#include "GL/wglew.h"
#endif

#if BUILD_DEBUG
#define GLERRORCHECK do { const GLenum Error = glGetError(); if( Error == GL_NO_ERROR ) { break; } else { PRINTF( "GL error: 0x%04X\n", Error ); WARNDESC( "GL check" ); } } while(1)
#else
#define GLERRORCHECK DoNothing
#endif

#if BUILD_DEBUG
#define GLPARANOIDERRORCHECK GLERRORCHECK
#else
#define GLPARANOIDERRORCHECK DoNothing
#endif