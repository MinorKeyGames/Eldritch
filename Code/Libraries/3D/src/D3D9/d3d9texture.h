#ifndef D3D9TEXTURE_H
#define D3D9TEXTURE_H

#include "texturecommon.h"

struct IDirect3DDevice9;
struct IDirect3DTexture9;

class D3D9Texture : public TextureCommon
{
public:
	D3D9Texture( IDirect3DDevice9* D3DDevice );
	D3D9Texture( IDirect3DDevice9* D3DDevice, IDirect3DTexture9* Texture );
	virtual ~D3D9Texture();

	virtual void*	GetHandle();

private:
	virtual void	CreateTexture( byte* const ARGBImage );
	virtual void	CreateTextureFromDDS( const IDataStream& Stream );

	IDirect3DTexture9*	m_Texture;
	IDirect3DDevice9*	m_D3DDevice;
};

#endif // D3D9TEXTURE_H