#ifndef D3D9PIXELSHADER_H
#define D3D9PIXELSHADER_H

#include "ipixelshader.h"
#include "map.h"

class IDataStream;

struct IDirect3DDevice9;
struct IDirect3DPixelShader9;

class D3D9PixelShader : public IPixelShader
{
public:
	D3D9PixelShader();
	virtual ~D3D9PixelShader();

	virtual void*	GetHandle() { return m_PixelShader; }
	virtual bool	GetRegister( const HashedString& Parameter, uint& Register ) const;

	void			Initialize( IDirect3DDevice9* const pD3DDevice, const IDataStream& Stream );

private:
	IDirect3DPixelShader9*	m_PixelShader;

	// Constant table for this shader, parsed from bytecode
	// Maps from parameter name to register number.
	Map<HashedString, uint>	m_ConstantTable;
};

#endif // D3D9PIXELSHADER_H