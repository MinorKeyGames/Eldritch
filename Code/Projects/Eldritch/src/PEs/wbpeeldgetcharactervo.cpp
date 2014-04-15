#include "core.h"
#include "wbpeeldgetcharactervo.h"
#include "Components/wbcompeldcharacter.h"
#include "configmanager.h"
#include "wbparamevaluatorfactory.h"

WBPEEldGetCharacterVO::WBPEEldGetCharacterVO()
:	m_EntityPE( NULL )
,	m_VO()
{
}

WBPEEldGetCharacterVO::~WBPEEldGetCharacterVO()
{
	SafeDelete( m_EntityPE );
}

/*virtual*/ void WBPEEldGetCharacterVO::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( EntityPE );
	m_EntityPE = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sEntityPE, "", sDefinitionName ) );

	STATICHASH( VO );
	m_VO = ConfigManager::GetString( sVO, "", sDefinitionName );
}

/*virtual*/ void WBPEEldGetCharacterVO::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam EntityValue;
	m_EntityPE->Evaluate( Context, EntityValue );

	WBEntity* const pEntity = EntityValue.GetEntity();
	if( !pEntity )
	{
		return;
	}

	WBCompEldCharacter* const pCharacter = GET_WBCOMP( pEntity, EldCharacter );
	if( !pCharacter )
	{
		return;
	}

	const SimpleString VoiceSet = pCharacter->GetCurrentVoiceSet();
	MAKEHASH( VoiceSet );
	MAKEHASH( m_VO );

	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_String;
	EvaluatedParam.m_String	= ConfigManager::GetString( sm_VO, "", sVoiceSet );
}