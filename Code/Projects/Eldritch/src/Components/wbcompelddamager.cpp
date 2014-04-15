#include "core.h"
#include "wbcompelddamager.h"
#include "configmanager.h"

WBCompEldDamager::WBCompEldDamager()
:	m_DamagerName()
{
}

WBCompEldDamager::~WBCompEldDamager()
{
}

/*virtual*/ void WBCompEldDamager::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( DamagerName );
	m_DamagerName = ConfigManager::GetInheritedString( sDamagerName, "", sDefinitionName );
}