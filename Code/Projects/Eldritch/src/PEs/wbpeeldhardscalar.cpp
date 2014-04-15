#include "core.h"
#include "wbpeeldhardscalar.h"
#include "configmanager.h"
#include "wbparamevaluatorfactory.h"
#include "eldritchgame.h"
#include "Components/wbcompeldhard.h"

WBPEEldHardScalar::WBPEEldHardScalar()
:	m_Scalar( 0.0f )
{
}

WBPEEldHardScalar::~WBPEEldHardScalar()
{
}

void WBPEEldHardScalar::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBPEUnaryOp::InitializeFromDefinition( DefinitionName );

	STATICHASH( Scalar );
	MAKEHASH( DefinitionName );

	m_Scalar = ConfigManager::GetFloat( sScalar, 0.0f, sDefinitionName );
}

void WBPEEldHardScalar::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBEntity* const pPlayer = EldritchGame::GetPlayer();
	ASSERT( pPlayer );

	WBCompEldHard* const pHard = SAFE_GET_WBCOMP( pPlayer, EldHard );
	ASSERT( pHard );

	if( pHard && pHard->IsHard() )
	{
		WBParamEvaluator::SEvaluatedParam Value;
		m_Input->Evaluate( Context, Value );

		ASSERT( Value.m_Type == WBParamEvaluator::EPT_Int || Value.m_Type == WBParamEvaluator::EPT_Float );

		if( Value.m_Type == WBParamEvaluator::EPT_Int )
		{
			EvaluatedParam.m_Type = WBParamEvaluator::EPT_Int;
			EvaluatedParam.m_Int = static_cast<int>( Value.m_Int * m_Scalar );
		}
		else
		{
			EvaluatedParam.m_Type = WBParamEvaluator::EPT_Float;
			EvaluatedParam.m_Float = Value.GetFloat() * m_Scalar;
		}
	}
	else
	{
		m_Input->Evaluate( Context, EvaluatedParam );
	}
}