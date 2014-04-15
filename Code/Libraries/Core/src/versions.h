#ifndef VERSIONS_H
#define VERSIONS_H

#define BUILD_WINDOWS	1
#define BUILD_MAC		0
#define BUILD_LINUX		0
#define BUILD_SDL		0
#define BUILD_STEAM		0

#define BUILD_WINDOWS_NO_SDL	BUILD_WINDOWS && !BUILD_SDL

#endif // VERSIONS_H