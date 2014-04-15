#ifndef IRODINRESOURCEUSER_H
#define IRODINRESOURCEUSER_H

class HashedString;

class IRodinResourceUser
{
public:
	virtual ~IRodinResourceUser() {}

	// Return true if this user should remain on the stack and false otherwise.
	virtual bool OnResourceStolen( const HashedString& Resource ) { Unused( Resource ); return false; }
	virtual void OnResourceReturned( const HashedString& Resource ) { Unused( Resource ); }
};

#endif // IRODINRESOURCEUSER_H