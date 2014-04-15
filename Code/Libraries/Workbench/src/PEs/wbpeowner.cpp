#include "core.h"
#include "wbpeowner.h"
#include "Components/wbcompowner.h"
#include "configmanager.h"

WBPEOwner::WBPEOwner()
:	m_Topmost( false )
{
}

WBPEOwner::~WBPEOwner()
{
}

/*virtual*/ void WBPEOwner::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBPE::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Topmost );
	m_Topmost = ConfigManager::GetBool( sTopmost, false, sDefinitionName );
}

/*virtual*/ void WBPEOwner::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	if( Context.m_Entity )
	{
		if( m_Topmost )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Entity;
			EvaluatedParam.m_Entity	= WBCompOwner::GetTopmostOwner( Context.m_Entity );
		}
		else
		{
			WBCompOwner* const pOwnerComponent = GET_WBCOMP( Context.m_Entity, Owner );
			if( pOwnerComponent )
			{
				EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Entity;
				EvaluatedParam.m_Entity	= pOwnerComponent->GetOwner();
			}
		}
	}
}