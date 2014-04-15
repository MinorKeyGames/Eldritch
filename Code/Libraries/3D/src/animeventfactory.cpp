#include "core.h"
#include "animeventfactory.h"
#include "configmanager.h"

AnimEventFactory* AnimEventFactory::m_Instance = NULL;

AnimEventFactory::AnimEventFactory()
:	m_FactoryFuncMap()
{
}

AnimEventFactory::~AnimEventFactory()
{
}

AnimEventFactory* AnimEventFactory::GetInstance()
{
	if( !m_Instance )
	{
		m_Instance = new AnimEventFactory;
	}
	return m_Instance;
}

void AnimEventFactory::DeleteInstance()
{
	SafeDelete( m_Instance );
}

void AnimEventFactory::Register( const HashedString& ClassName, AnimEventFactoryFunc FactoryFunc )
{
	m_FactoryFuncMap.Insert( ClassName, FactoryFunc );
}

AnimEvent* AnimEventFactory::Create( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Type );
	const HashedString Type = ConfigManager::GetHash( sType, HashedString::NullString, sDefinitionName );

	return Create( Type, DefinitionName );
}

AnimEvent* AnimEventFactory::Create( const HashedString& ClassName, const SimpleString& DefinitionName )
{
	Map< HashedString, AnimEventFactoryFunc >::Iterator FactoryFuncIter = m_FactoryFuncMap.Search( ClassName );
	ASSERTDESC( !FactoryFuncIter.IsNull(), "Unknown AnimEvent class specified in config file." );

	AnimEvent* pNewAnimEvent = ( *FactoryFuncIter )();
	pNewAnimEvent->InitializeFromDefinition( DefinitionName );

	return pNewAnimEvent;
}