#ifndef GL2PIXELSHADER_H
#define GL2PIXELSHADER_H

#include "ipixelshader.h"
#include "gl2.h"

class IDataStream;

class GL2PixelShader : public IPixelShader
{
public:
	GL2PixelShader();
	virtual ~GL2PixelShader();

	virtual void*	GetHandle() { return &m_PixelShader; }
	virtual bool	GetRegister( const HashedString& Parameter, uint& Register ) const;

	void			Initialize( const IDataStream& Stream );

private:
	GLuint	m_PixelShader;
};

#endif // GL2PIXELSHADER_H