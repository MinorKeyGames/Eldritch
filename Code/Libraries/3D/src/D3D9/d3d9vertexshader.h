#ifndef D3D9VERTEXSHADER_H
#define D3D9VERTEXSHADER_H

#include "ivertexshader.h"
#include "map.h"

class IDataStream;

struct IDirect3DDevice9;
struct IDirect3DVertexShader9;

class D3D9VertexShader : public IVertexShader
{
public:
	D3D9VertexShader();
	virtual ~D3D9VertexShader();

	virtual void*	GetHandle() { return m_VertexShader; }
	virtual bool	GetRegister( const HashedString& Parameter, uint& Register ) const;

	void			Initialize( IDirect3DDevice9* const pD3DDevice, const IDataStream& Stream );

private:
	IDirect3DVertexShader9*	m_VertexShader;

	// Constant table for this shader, parsed from bytecode
	// Maps from parameter name to register number.
	Map<HashedString, uint>	m_ConstantTable;
};

#endif // D3D9VERTEXSHADER_H