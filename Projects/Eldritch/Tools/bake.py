import os
import sys

VERBOSE = True
REBUILD = False

# The first value in each entry is the description that will be
# shown to the user if they try an invalid ruleset.

BAKE_RULES = {	'all': ( 'Bake everything.', 'blender_meshes', 'fonts', 'font_textures', 'textures', 'shaders', 'meshes', 'config', 'audio', 'rooms', 'exe', 'etc' ),
				'meshes': ( 'Export and bake Blender meshes.', 'blender_meshes', 'meshes' ),
				'compilemeshes': ( 'Bake intermediate meshes.', 'meshes' ),
				'fonts': ( 'Generate font textures and bake definitions.', 'fonts', 'font_textures' ),
				'textures': ( 'Bake all textures.', 'textures', 'font_textures' ),
				'shaders': ( 'Bake shaders.', 'shaders' ),
				'config': ( 'Bake configuration files.', 'config' ),
				'audio': ( 'Bake all audio.', 'audio' ),
				'rooms': ( 'Bake room files.', 'rooms' ),
				'exe': ( 'Bake executables.', 'exe' ),
				'etc': ( 'Bake everything else.', 'etc' ),
				'clean': ( 'Clean Intermediate and Baked folders.', 'clean' ) }

# Asset folders
RAW					= '../Raw/'
BAKED				= '../Baked/'
INTERMEDIATE		= '../Intermediate/'
RAW_TEXTURES		= RAW + 'Textures/'
INTER_TEXTURES		= INTERMEDIATE + 'Textures/'
INTER_FONT_TEXTURES	= INTER_TEXTURES + 'Fonts/'
RAW_MESHES			= RAW + 'Meshes/'
INTER_MESHES		= INTERMEDIATE + 'Meshes/'
FONTS				= RAW + 'Fonts/'
SHADERS				= RAW + 'Shaders/'
CONFIG				= RAW + 'Config/'
AUDIO				= RAW + 'Audio/'
ROOMS				= RAW + 'Rooms/'
MISC				= RAW + 'Misc/'

# Tools folders
CYGWIN_BIN = 'C:/cygwin/bin/'
TOOLS_DIR = './'
BLENDER_DIR = 'C:/Program Files (x86)/Blender Foundation/Blender/'
DXUTIL_DIR = 'C:/Program Files (x86)/Microsoft DirectX SDK (June 2008)/Utilities/Bin/x86/'

# Tools
COPY = CYGWIN_BIN + 'cp'
MOVE = CYGWIN_BIN + 'mv'
MESH_COMPILER = TOOLS_DIR + 'MeshCompiler.exe'
CONFIG_COMPILER = TOOLS_DIR + 'ConfigCompiler.exe'
FONT_GENERATOR = TOOLS_DIR + 'FontGenerator.exe'
MESH_EXPORT = TOOLS_DIR + 'xmlmeshexport.py'
BLENDER = BLENDER_DIR + 'blender.exe'
DXTEX = TOOLS_DIR + 'DxTex.exe'	# Custom version to do normal hack
FXC = TOOLS_DIR + 'fxc.exe'		# Moved to work around Program Files weirdness or something

NUMBAKED = 0
NUMSKIPPED = 0

#-----------------------------------------------------
def bake( rules ):
	if 'blender_meshes' in rules:
		blenddir( MESH_EXPORT, RAW_MESHES, RAW, INTERMEDIATE, '.mesh' )

	if 'fonts' in rules:
		generatefonts( FONT_GENERATOR, FONTS )
	if 'textures' in rules:
		converttextures( DXTEX, RAW_TEXTURES, RAW, BAKED );
		bakedir( COPY, RAW_TEXTURES, RAW, BAKED, '.bmp', '.bmp' )
	if 'font_textures' in rules:
		converttextures( DXTEX, INTER_TEXTURES, INTERMEDIATE, BAKED );
	if 'shaders' in rules:
		compileshaders( FXC, COPY, SHADERS );

	if 'config' in rules:
		bakedir( CONFIG_COMPILER, CONFIG, RAW, BAKED, '.config', '.ccf' )
		bakedir( CONFIG_COMPILER, CONFIG, RAW, BAKED, '.syncerconfig', '.pcf' )
		bakedir( COPY, CONFIG, RAW, BAKED, '.cfg', '.cfg' )
	if 'meshes' in rules:
		bakedir( MESH_COMPILER, INTER_MESHES, INTERMEDIATE, BAKED, '.mesh', '.cms' )
	if 'audio' in rules:
		bakedir( COPY, AUDIO, RAW, BAKED, '.mp3', '.mp3' )
		bakedir( COPY, AUDIO, RAW, BAKED, '.ogg', '.ogg' )
		bakedir( COPY, AUDIO, RAW, BAKED, '.wav', '.wav' )
	if 'rooms' in rules:
		bakedir( COPY, ROOMS, RAW, BAKED, '.eldritchroom', '.eldritchroom' )

	if 'exe' in rules:
		bakedir( COPY, RAW, RAW, BAKED, '.exe', '.exe', False )
		bakedir( COPY, RAW, RAW, BAKED, '.dll', '.dll', False )

	if 'etc' in rules:
		bakedir( COPY, RAW, RAW, BAKED, '.txt', '.txt', False )
		bakedir( COPY, RAW, RAW, BAKED, '.html', '.html', False )
		bakedir( COPY, MISC, RAW, BAKED, '.bmp', '.bmp', False )

#-----------------------------------------------------
def runtool( args ):
	if VERBOSE:
		for arg in args:
			print arg,
		print
	os.spawnv( os.P_WAIT, args[0], args )

#-----------------------------------------------------
# Timestamp checking for simple improvement on build time
def shouldbuild( rawfile, bakedfiles ):
	global NUMBAKED
	global NUMSKIPPED

	if REBUILD:
		NUMBAKED = NUMBAKED + 1
		return True

	rawstat = os.stat( rawfile )
	for bakedfile in bakedfiles:
		bakedfileexists = os.access( bakedfile, os.F_OK )
		if bakedfileexists:
			bakedstat = os.stat( bakedfile )
			if bakedstat.st_mtime < rawstat.st_mtime:
				NUMBAKED = NUMBAKED + 1
				return True
		else:
			NUMBAKED = NUMBAKED + 1
			return True

	if VERBOSE:
		print 'Skipping ' + rawfile

	NUMSKIPPED = NUMSKIPPED + 1
	return False

#-----------------------------------------------------
# If rawext is specified, only files matching that extension are baked
# If rawext isn't specified, all files in the folder are baked
# This will recurse into any subfolders of the given path
def bakedir( tool, rawdir, rawpath, bakedpath, rawext = '', bakedext = '', recursive = True ):
	for path, dirs, files in os.walk( rawdir ):
		if '.svn' in dirs:
			dirs.remove( '.svn' )	# Ignore SVN folders
		if not recursive:
			del dirs[:]				# Empty the list so we just examine this path
		if not os.path.exists( path.replace( rawpath, bakedpath ) ):
			os.mkdir( path.replace( rawpath, bakedpath ) )
		for file in files:
			if( ( not rawext ) or ( rawext in file ) ):
				infile = os.path.join( path, file )
				outfile = infile.replace( rawpath, bakedpath ).replace( rawext, bakedext )
				if shouldbuild( infile, [ outfile ] ):
					runtool( [ tool, infile, outfile ] )

#-----------------------------------------------------
# Create font property and image files
def generatefonts( tool, rawdir ):
	for path, dirs, files in os.walk( rawdir ):
		if '.svn' in dirs:
			dirs.remove( '.svn' )	# Ignore SVN folders
		if not os.path.exists( path.replace( FONTS, INTER_FONT_TEXTURES ) ):
			os.mkdir( path.replace( FONTS, INTER_FONT_TEXTURES ) )
		if not os.path.exists( path.replace( RAW, BAKED ) ):
			os.mkdir( path.replace( RAW, BAKED ) )
		for file in files:
			if( '.font' in file ):
				infile = os.path.join( path, file )
				outfontfile = infile.replace( RAW, BAKED ).replace( '.font', '.fnp' )
				outimagefile = infile.replace( FONTS, INTER_FONT_TEXTURES ).replace( '.font', '.tga' )
				# Don't test shouldbuild(outimagefile), because it's unnecessary and the name will be modified by FontGenerator
				if shouldbuild( infile, [ outfontfile ] ):
					runtool( [ tool, infile, outfontfile, outimagefile ] )

#-----------------------------------------------------
# Create font property and image files
def converttextures( tool, rawdir, rawpath, bakedpath ):
	for path, dirs, files in os.walk( rawdir ):
		if '.svn' in dirs:
			dirs.remove( '.svn' )	# Ignore SVN folders
		if not os.path.exists( path.replace( rawpath, bakedpath ) ):
			os.mkdir( path.replace( rawpath, bakedpath ) )
		for file in files:
			if( '.tga' in file ):
				if( '_NODXT' in file ):
					infile = os.path.join( path, file )
					outfile = infile.replace( rawpath, bakedpath )
					if shouldbuild( infile, [ outfile ] ):
						runtool( [ COPY, infile, outfile ] )
				else:
					compression = 'DXT1'
					normalhack = ''
					if( '_DXT2' in file ):
						compression = 'DXT2'
					if( '_DXT3' in file ):
						compression = 'DXT3'
					if( '_DXT4' in file ):
						compression = 'DXT4'
					if( '_DXT5' in file ):
						compression = 'DXT5'
					if( '_NORMAL' in file ):
						compression = 'DXT5'
						normalhack = '-n'
					if( '_SPEC' in file ):
						compression = 'DXT5'
					infile = os.path.join( path, file )
					outfile = infile.replace( rawpath, bakedpath ).replace( '.tga', '.dds' )
					if shouldbuild( infile, [ outfile ] ):
						runtool( [ tool, infile, '-m', normalhack, compression, outfile ] )

#-----------------------------------------------------
# Create font property and image files
def compileshaders( compiler, copy, rawdir ):
	for path, dirs, files in os.walk( rawdir ):
		if '.svn' in dirs:
			dirs.remove( '.svn' )	# Ignore SVN folders
		if not os.path.exists( path.replace( RAW, BAKED ) ):
			os.mkdir( path.replace( RAW, BAKED ) )
		for file in files:

			if( file.endswith( '.fx' ) ):
				infile = os.path.join( path, file )
				outfile = infile.replace( RAW, BAKED ).replace( '.fx', '.cfx' )
				if shouldbuild( infile, [ outfile ] ):
					runtool( [ compiler, '/T', 'fx_2_0', '/Fo', outfile, infile ] )

			elif( file.endswith( '.hlsl_vs2' ) ):
				infile = os.path.join( path, file )
				outfile = infile.replace( RAW, BAKED ).replace( '.hlsl_vs2', '.chv2' )
				if shouldbuild( infile, [ outfile ] ):
					runtool( [ compiler, '/T', 'vs_2_0', '/E', 'Main', '/Fo', outfile, infile ] )

			elif( file.endswith( '.hlsl_ps2' ) ):
				infile = os.path.join( path, file )
				outfile = infile.replace( RAW, BAKED ).replace( '.hlsl_ps2', '.chp2' )
				if shouldbuild( infile, [ outfile ] ):
					runtool( [ compiler, '/T', 'ps_2_0', '/E', 'Main', '/Fo', outfile, infile ] )

			elif( file.endswith( '.glsl_vs120' ) ):
				infile = os.path.join( path, file )
				outfile = infile.replace( RAW, BAKED ).replace( '.glsl_vs120', '.gv12' )
				if shouldbuild( infile, [ outfile ] ):
					runtool( [ copy, infile, outfile ] )

			elif( file.endswith( '.glsl_fs120' ) ):
				infile = os.path.join( path, file )
				outfile = infile.replace( RAW, BAKED ).replace( '.glsl_fs120', '.gf12' )
				if shouldbuild( infile, [ outfile ] ):
					runtool( [ copy, infile, outfile ] )

#-----------------------------------------------------
# Call Blender in background mode with a given script for each blend file
def blenddir( script, sourcedir, rawpath, bakedpath, bakedext ):
	for path, dirs, files in os.walk( sourcedir ):
		if '.svn' in dirs:
			dirs.remove( '.svn' )	# Ignore SVN folders
		if not os.path.exists( path.replace( rawpath, bakedpath ) ):
			os.mkdir( path.replace( rawpath, bakedpath ) )
		for file in files:
			if( file.endswith('.blend') ): # Cull .blend1, etc.
				infile = os.path.join( path, file )
				exportedfile = infile.replace( '.blend', bakedext )
				outfile = exportedfile.replace( rawpath, bakedpath )
				# Because I can't pass another parameter to Blender, I have to assume the name and move it
				if shouldbuild( infile, [ outfile ] ):
					runtool( [ BLENDER, '-b', infile, '-P', script ] )
					runtool( [ MOVE, '-f', exportedfile, outfile ] )

#-----------------------------------------------------
# Recursively delete a directory
def clean( cleanpath ):
	if os.path.exists( cleanpath ):
		for path, dirs, files in os.walk( cleanpath, False ):
			for file in files:
				os.remove( path + '/' + file )
			os.rmdir( path )

#-----------------------------------------------------
# Ensure a directory exists by making it if it does not
def makeifnotexists( path ):
	while not os.path.exists( path ):
		try:
			os.mkdir( path )
		except:
			pass

#-----------------------------------------------------
# Entry point
#-----------------------------------------------------

# TODO: Add exception handling (that also catches bad tool exit codes)
# NOTE: Python uses try/except/raise instead of try/catch/throw

# Ruleset must be the first parameter, if it's used at all
if len( sys.argv ) > 1:
	if sys.argv[1] in BAKE_RULES:
		ruleset = sys.argv[1]
	else:
		ruleset = None
else:
	ruleset = 'all'

for arg in sys.argv:
	if arg == '-r':
		REBUILD = True

if not ruleset:
	print 'Specified ruleset not found. Valid rulesets are:'
	for k, v in BAKE_RULES.iteritems():
		print k + ': ' + v[0]
else:
	rules = BAKE_RULES[ ruleset ]
	
	# Only clean the Intermediate/Baked folders if we're rebuilding all
	if( ( ruleset == 'all' and REBUILD ) or 'clean' in rules ):
		print 'Deleting Intermediate and Baked folders...'
		clean( INTERMEDIATE )
		clean( BAKED )

	makeifnotexists( INTERMEDIATE )
	makeifnotexists( BAKED )
	
	print 'Baking assets...'
	
	try:
		bake( rules )
	except:
		print 'Exception while baking.'
		sys.exit(1)
	
	print 'Baking done!'
	print 'Baked %d files, skipped %d files.' % ( NUMBAKED, NUMSKIPPED )
	print 'Total %d files processed.' % ( NUMBAKED + NUMSKIPPED )