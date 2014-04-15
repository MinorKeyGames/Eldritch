#include "core.h"
#include "frameworkutil.h"
#include "configmanager.h"
#include "packstream.h"
#include "filestream.h"
#include "file.h"

void InternalLoadConfigFiles();

void FrameworkUtil::LoadConfigFiles()
{
	STATICHASH( PackageFile );
	STATICHASH( NumConfigFiles );

	// Must load defaults and initialize the package before we can access any other files
	if( !FileUtil::Exists( "Config/default.cfg" ) )
	{
		return;
	}

	ConfigManager::Load( FileStream( "Config/default.cfg", FileStream::EFM_Read ) );

	// Load user variables over the defaults if possible
	if( FileUtil::Exists( "Config/user.cfg" ) )
	{
		ConfigManager::Load( FileStream( "Config/user.cfg", FileStream::EFM_Read ) );
	}

	// Initialize the packstream from whatever the default.cfg specifies.
	PackStream::StaticAddPackageFile( ConfigManager::GetString( sPackageFile ) );

	InternalLoadConfigFiles();
}

void FrameworkUtil::MinimalLoadConfigFiles( const char* RootFilename )
{
	ConfigManager::Load( PackStream( RootFilename ) );

	InternalLoadConfigFiles();
}

void InternalLoadConfigFiles()
{
	STATICHASH( NumConfigFiles );

	int NumConfigFiles = ConfigManager::GetInt( sNumConfigFiles );
	for( int i = 0; i < NumConfigFiles; ++i )
	{
		const char* ConfigFile = ConfigManager::GetSequenceString( "ConfigFile%d", i );
		if( ConfigFile )
		{
			ConfigManager::Load( PackStream( ConfigFile ) );
		}
	}
}