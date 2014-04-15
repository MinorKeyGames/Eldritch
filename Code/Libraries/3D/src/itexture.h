#ifndef ITEXTURE_H
#define ITEXTURE_H

class ITexture
{
public:
	virtual ~ITexture() {}

	virtual void* GetHandle() = 0;
	virtual uint GetWidth() const = 0;
	virtual uint GetHeight() const = 0;
};

#endif // ITEXTURE_H