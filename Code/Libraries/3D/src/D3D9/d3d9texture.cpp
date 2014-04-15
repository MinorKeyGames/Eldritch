#include "core.h"
#include "d3d9texture.h"
#include "configmanager.h"
#include "idatastream.h"
#include "mathcore.h"

#include <d3d9.h>
#include <ddraw.h>	// For DDSURFACEDESC2

D3D9Texture::D3D9Texture( IDirect3DDevice9* D3DDevice )
:	m_Texture( NULL )
,	m_D3DDevice( D3DDevice )
{
}

D3D9Texture::D3D9Texture( IDirect3DDevice9* D3DDevice, IDirect3DTexture9* Texture )
:	m_D3DDevice( D3DDevice )
,	m_Texture( Texture )
{
}

D3D9Texture::~D3D9Texture()
{
	SafeRelease( m_Texture );
}

void* D3D9Texture::GetHandle()
{
	return m_Texture;
}

/*virtual*/ void D3D9Texture::CreateTexture( byte* const ARGBImage )
{
	const int MipLevels = CountMipLevels();

	HRESULT hr = m_D3DDevice->CreateTexture( m_Width, m_Height, MipLevels, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_Texture, NULL );
	ASSERT( hr == D3D_OK );
	ASSERT( m_Texture );

	STATICHASH( DebugMips );
	const bool	DebugMips	= ConfigManager::GetBool( sDebugMips );

	// Now fill the texture (and mipmaps)
	D3DLOCKED_RECT LockedRect;
	int		Width		= m_Width;
	int		Height		= m_Height;
	int		MipWidth	= 0;
	int		MipHeight	= 0;
	byte*	ThisLevel	= ARGBImage;
	byte*	NextLevel	= NULL;
	for( int MipLevel = 0; MipLevel < MipLevels; ++MipLevel )
	{
		HRESULT hr = m_Texture->LockRect( MipLevel, &LockedRect, NULL, 0 );
		ASSERT( hr == D3D_OK );

		memcpy( LockedRect.pBits, ThisLevel, LockedRect.Pitch * Height );

		hr = m_Texture->UnlockRect( MipLevel );
		ASSERT( hr == D3D_OK );

		if( DebugMips )
		{
			NextLevel = MakeDebugMip( MipLevel + 1, Width, Height, MipWidth, MipHeight );
		}
		else
		{
			NextLevel = MakeMip( Width, Height, MipWidth, MipHeight, ThisLevel );
		}

		SafeDeleteArray( ThisLevel );
		ThisLevel = NextLevel;
		Width = MipWidth;
		Height = MipHeight;
	}
	SafeDeleteArray( NextLevel );
}

/*virtual*/ void D3D9Texture::CreateTextureFromDDS( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	DDSURFACEDESC2	SurfaceDesc = {0};
	uint			Magic;
	
	Magic = Stream.ReadUInt32();
	Stream.Read( sizeof( DDSURFACEDESC2 ), &SurfaceDesc );

	m_Width = SurfaceDesc.dwWidth;
	m_Height = SurfaceDesc.dwHeight;

	DEVASSERT( Magic == ' SDD' );

	DEVASSERT( SurfaceDesc.dwFlags & ( DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT ) );
	DEVASSERT( SurfaceDesc.ddsCaps.dwCaps & DDSCAPS_TEXTURE );

	// Assume compressed texture
	DEVASSERT( SurfaceDesc.ddpfPixelFormat.dwFlags & DDPF_FOURCC );

	// I'm also going to assume mipmaps for now
	DEVASSERT( SurfaceDesc.dwFlags & DDSD_MIPMAPCOUNT );
	DEVASSERT( SurfaceDesc.ddsCaps.dwCaps & ( DDSCAPS_COMPLEX | DDSCAPS_MIPMAP ) );

	D3DFORMAT Format = D3DFMT_UNKNOWN;
	// TODO: Break off into separate function if this ever grows
	if( SurfaceDesc.ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '1') )
	{
		Format = D3DFMT_DXT1;
	}
	else if( SurfaceDesc.ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '2') )
	{
		Format = D3DFMT_DXT2;
	}
	else if( SurfaceDesc.ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '3') )
	{
		Format = D3DFMT_DXT3;
	}
	else if( SurfaceDesc.ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '4') )
	{
		Format = D3DFMT_DXT4;
	}
	else if( SurfaceDesc.ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '5') )
	{
		Format = D3DFMT_DXT5;
	}
	DEVASSERT( Format != D3DFMT_UNKNOWN );

	ASSERT( m_D3DDevice );
	const HRESULT hr = m_D3DDevice->CreateTexture(
		SurfaceDesc.dwWidth,
		SurfaceDesc.dwHeight,
		SurfaceDesc.dwMipMapCount,
		0,
		Format,
		D3DPOOL_MANAGED,
		&m_Texture,
		NULL );

	ASSERT( m_Texture );
	ASSERT( hr == D3D_OK );
	Unused( hr );

	// For a compressed texture, the size of each mipmap level image is one-fourth
	// the size of the previous, with a minimum of 8 (DXT1) or 16 (DXT2-5) bytes
	// (for square textures). Use the following formula to calculate the size of
	// each level for a non-square texture:
	// max(1, width x 4) x max(1, height x 4) x 8(DXT1) or 16(DXT2-5)
	// DLP NOTE: That's from the DX docs, but I think it's supposed to be width and height DIVIDED BY 4

	// Still assuming compressed textures
	uint	FormatBytes	= ( Format == D3DFMT_DXT1 ? 8 : 16 );
	uint	BlocksWide	= Max( 1, SurfaceDesc.dwWidth >> 2 );
	uint	BlocksHigh	= Max( 1, SurfaceDesc.dwHeight >> 2 );
	uint	ReadBytes	= BlocksWide * BlocksHigh * FormatBytes;

	D3DLOCKED_RECT LockedRect;
	for( uint i = 0; i < SurfaceDesc.dwMipMapCount; ++i )
	{
		m_Texture->LockRect( i, &LockedRect, NULL, 0 );
		Stream.Read( ReadBytes, LockedRect.pBits );
		m_Texture->UnlockRect( i );
		ReadBytes = Max( FormatBytes, ReadBytes >> 2 );
	}
}