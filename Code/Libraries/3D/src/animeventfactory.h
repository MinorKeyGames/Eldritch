#ifndef ANIMEVENTFACTORY_H
#define ANIMEVENTFACTORY_H

#include "animevent.h"
#include "hashedstring.h"
#include "map.h"

class SimpleString;

class AnimEventFactory
{
public:
	static AnimEventFactory*	GetInstance();
	static void					DeleteInstance();

	void		Register( const HashedString& ClassName, AnimEventFactoryFunc FactoryFunc );
	AnimEvent*	Create( const HashedString& ClassName, const SimpleString& DefinitionName );
	AnimEvent*	Create( const SimpleString& DefinitionName );

private:
	AnimEventFactory();
	~AnimEventFactory();

	Map<HashedString, AnimEventFactoryFunc>	m_FactoryFuncMap;

	static AnimEventFactory*	m_Instance;
};

#endif // ANIMEVENTFACTORY_H