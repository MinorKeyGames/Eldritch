#ifndef FRAMEWORKUTIL_H
#define FRAMEWORKUTIL_H

namespace FrameworkUtil
{
	// Game-style load, starts from a loose default.cfg. As a side effect, initializes the packstream source from config if it's not already.
	void LoadConfigFiles();

	// Smaller version, assumes packstream source has been initialized already and only loads from there.
	void MinimalLoadConfigFiles( const char* RootFilename );
}

#endif // FRAMEWORKUTIL_H