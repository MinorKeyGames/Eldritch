#ifndef IVERTEXSHADER_H
#define IVERTEXSHADER_H

class HashedString;

class IVertexShader
{
public:
	virtual ~IVertexShader() {}

	virtual void*	GetHandle() = 0;
	virtual bool	GetRegister( const HashedString& Parameter, uint& Register ) const = 0;
};

#endif // IVERTEXSHADER_H