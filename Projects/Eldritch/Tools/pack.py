import os
import sys

VERBOSE = True
PACKTESTASSETS = False

# Assets
BAKED		= '../Baked/'
TEXTURES	= 'Textures/'
MESHES		= 'Meshes/'
FONTS		= 'Fonts/'
SHADERS		= 'Shaders/'
CONFIG		= 'Config/'
AUDIO		= 'Audio/'
ROOMS		= 'Rooms/'
MISC		= 'Misc/'

BASEPACKFILE		= 'eldritch-base.cpk'
TEXTURESPACKFILE	= 'eldritch-textures.cpk'
MESHESPACKFILE		= 'eldritch-meshes.cpk'
AUDIOPACKFILE		= 'eldritch-audio.cpk'
WORLDPACKFILE		= 'eldritch-world.cpk'
SYNCERPACKFILE		= 'syncer.cpk'

# Map from folder names to package files
DLCPACKFILES = {	'DLC1' : 'dlc1.cpk',
					'DLC2' : 'dlc2.cpk' }

# Tools folders
TOOLS_DIR = '../Tools/'	# Relative to Baked directory

# Tools
FILE_PACKER = TOOLS_DIR + 'FilePacker.exe'

#-----------------------------------------------------
def pack():
	packdir( CONFIG,	True, '.ccf',			BASEPACKFILE )
	packdir( CONFIG,	True, '.pcf',			BASEPACKFILE )
	packdir( FONTS,		True, '.fnp',			BASEPACKFILE )
	packdir( MESHES,	True, '.cms',			MESHESPACKFILE )
	packdir( SHADERS,	True, '.cfx',			BASEPACKFILE )
	packdir( SHADERS,	True, '.chv2',			BASEPACKFILE )
	packdir( SHADERS,	True, '.chp2',			BASEPACKFILE )
	packdir( SHADERS,	True, '.gv12',			BASEPACKFILE )
	packdir( SHADERS,	True, '.gf12',			BASEPACKFILE )
	packdir( TEXTURES,	True, '.dds',			TEXTURESPACKFILE )
	packdir( TEXTURES,	True, '.bmp',			TEXTURESPACKFILE )
	packdir( TEXTURES,	True, '.tga',			TEXTURESPACKFILE )
	packdir( ROOMS,		True, '.eldritchroom',	WORLDPACKFILE )
	packdir( AUDIO,		True, '.wav',			AUDIOPACKFILE )
	#packdir( AUDIO,	True, '.ogg',			AUDIOPACKFILE )	# Compress .ogg files because Audiere can't stream from pack file anyway.
	packdir( AUDIO,		False, '.ogg',			AUDIOPACKFILE )	# Don't compress .ogg files because FMOD can stream from pack file.
	packdir( MISC,		True, '.bmp',			BASEPACKFILE )

	packdir( CONFIG,	True, '.pcf',			SYNCERPACKFILE )

#-----------------------------------------------------
def runtool( args ):
	if VERBOSE:
		for arg in args:
			print arg,
		print
	os.spawnv( os.P_WAIT, args[0], args )

#-----------------------------------------------------
# If ext is specified, only files matching that extension are baked
# If ext isn't specified, all files in the folder are baked
# This will recurse into any subfolders of the given path
def packdir( dir, compress, ext, packfile ):
	for path, dirs, files in os.walk( dir ):

		# Ignore source control and test content
		if '.svn' in dirs:
			dirs.remove( '.svn' )

		# Ignore test content if we're not building a test package
		if( ( not PACKTESTASSETS ) and ( 'Test' in dirs ) ):
			dirs.remove( 'Test' )

		usepackfile = packfile

		# Override package file for DLC
		for dlcdir, dlcpackfile in DLCPACKFILES.iteritems():
			if dlcdir in path:
				usepackfile = dlcpackfile

		for file in files:
			if( ( not ext ) or ( ext in file ) ):
				infile = os.path.join( path, file )
				compressflag = ''
				if compress:
					compressflag = '-c'
				runtool( [ FILE_PACKER, infile, usepackfile, compressflag ] )

#-----------------------------------------------------
# Entry point
#-----------------------------------------------------

for arg in sys.argv:
	if arg == '-t':
		PACKTESTASSETS = True
		print 'ALERT: Packing test assets!'

print 'Deleting pack files...'

os.chdir( BAKED )

if os.path.exists( BASEPACKFILE ):
	os.remove( BASEPACKFILE )

if os.path.exists( TEXTURESPACKFILE ):
	os.remove( TEXTURESPACKFILE )

if os.path.exists( MESHESPACKFILE ):
	os.remove( MESHESPACKFILE )

if os.path.exists( AUDIOPACKFILE ):
	os.remove( AUDIOPACKFILE )

if os.path.exists( WORLDPACKFILE ):
	os.remove( WORLDPACKFILE )

if os.path.exists( SYNCERPACKFILE ):
	os.remove( SYNCERPACKFILE )

for dlcdir, dlcpackfile in DLCPACKFILES.iteritems():
	if os.path.exists( dlcpackfile ):
		os.remove( dlcpackfile )

print 'Packing assets...'

try:
	pack()
except:
	sys.exit(1)
	
print 'Packing done!'