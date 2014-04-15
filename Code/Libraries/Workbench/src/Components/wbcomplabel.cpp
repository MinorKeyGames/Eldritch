#include "core.h"
#include "wbcomplabel.h"
#include "configmanager.h"

WBCompLabel::WBCompLabel()
:	m_Label()
{
}

WBCompLabel::~WBCompLabel()
{
}

/*virtual*/ void WBCompLabel::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Label );
	m_Label = ConfigManager::GetInheritedHash( sLabel, HashedString::NullString, sDefinitionName );
}