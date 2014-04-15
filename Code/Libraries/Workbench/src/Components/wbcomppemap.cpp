#include "core.h"
#include "wbcomppemap.h"
#include "configmanager.h"
#include "wbparamevaluatorfactory.h"

WBCompPEMap::WBCompPEMap()
:	m_PEMap()
{
}

WBCompPEMap::~WBCompPEMap()
{
	FOR_EACH_MAP( PEIter, m_PEMap, HashedString, WBPE* )
	{
		WBPE* PE = PEIter.GetValue();
		SafeDelete( PE );
	}
}

/*virtual*/ void WBCompPEMap::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( NumPEs );
	const uint NumPEs = ConfigManager::GetInheritedInt( sNumPEs, 0, sDefinitionName );
	for( uint PEIndex = 0; PEIndex < NumPEs; ++PEIndex )
	{
		const HashedString PEKey = ConfigManager::GetInheritedSequenceHash( "PE%dKey", PEIndex, HashedString::NullString, sDefinitionName );
		const SimpleString PEName = ConfigManager::GetInheritedSequenceString( "PE%dDef", PEIndex, "", sDefinitionName );

		WBPE* const PE = WBParamEvaluatorFactory::Create( PEName );
		ASSERT( PE );

		m_PEMap.Insert( PEKey, PE );
	}
}

WBPE* WBCompPEMap::GetPE( const HashedString& Name ) const
{
	Map<HashedString, WBPE*>::Iterator PEIter = m_PEMap.Search( Name );
	return PEIter.IsValid() ? PEIter.GetValue() : NULL;
}