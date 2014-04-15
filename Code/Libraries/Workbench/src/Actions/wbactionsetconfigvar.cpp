#include "core.h"
#include "wbactionsetconfigvar.h"
#include "configmanager.h"

WBActionSetConfigVar::WBActionSetConfigVar()
:	m_VarContext()
,	m_VarName()
,	m_ValuePE()
{
}

WBActionSetConfigVar::~WBActionSetConfigVar()
{
}

/*virtual*/ void WBActionSetConfigVar::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( VarContext );
	m_VarContext = ConfigManager::GetString( sVarContext, "", sDefinitionName );

	STATICHASH( VarName );
	m_VarName = ConfigManager::GetString( sVarName, "", sDefinitionName );

	STATICHASH( ValuePE );
	m_ValuePE.InitializeFromDefinition( ConfigManager::GetString( sValuePE, "", sDefinitionName ) );
}

/*virtual*/ void WBActionSetConfigVar::Execute()
{
	WBAction::Execute();

	WBParamEvaluator::SPEContext Context;
	Context.m_Entity = GetEntity();
	m_ValuePE.Evaluate( Context );

	MAKEHASH( m_VarContext );
	MAKEHASH( m_VarName );

	if( m_ValuePE.GetType() == WBParamEvaluator::EPT_Bool )
	{
		ConfigManager::SetBool( sm_VarName, m_ValuePE.GetBool(), sm_VarContext );
	}
	else if( m_ValuePE.GetType() == WBParamEvaluator::EPT_Int )
	{
		ConfigManager::SetInt( sm_VarName, m_ValuePE.GetInt(), sm_VarContext );
	}
	else if( m_ValuePE.GetType() == WBParamEvaluator::EPT_Float )
	{
		ConfigManager::SetFloat( sm_VarName, m_ValuePE.GetFloat(), sm_VarContext );
	}
	else if( m_ValuePE.GetType() == WBParamEvaluator::EPT_String )
	{
		ConfigManager::SetString( sm_VarName, m_ValuePE.GetString().CStr(), sm_VarContext );
	}
}