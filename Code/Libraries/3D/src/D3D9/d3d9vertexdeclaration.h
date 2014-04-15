#ifndef D3D9VERTEXDECLARATION_H
#define D3D9VERTEXDECLARATION_H

#include "ivertexdeclaration.h"

struct IDirect3DDevice9;
struct IDirect3DVertexDeclaration9;

class D3D9VertexDeclaration : public IVertexDeclaration
{
public:
	D3D9VertexDeclaration( IDirect3DDevice9* pD3DDevice );
	virtual ~D3D9VertexDeclaration();

	virtual void	Initialize( uint VertexSignature );
	virtual void*	GetDeclaration();
	virtual uint	GetSignature();

private:
	IDirect3DDevice9*				m_D3DDevice;
	IDirect3DVertexDeclaration9*	m_VertexDeclaration;
	uint							m_VertexSignature;
};

#endif // D3D9VERTEXDECLARATION_H