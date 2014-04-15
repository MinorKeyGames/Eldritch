#include "core.h"
#include "wbcompeldmarkup.h"
#include "configmanager.h"

WBCompEldMarkup::WBCompEldMarkup()
{
}

WBCompEldMarkup::~WBCompEldMarkup()
{
}

/*virtual*/ void WBCompEldMarkup::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Markup );
	m_Markup = ConfigManager::GetInheritedHash( sMarkup, HashedString::NullString, sDefinitionName );
}