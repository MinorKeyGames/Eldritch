#include "core.h"
#include "wbpeconditional.h"
#include "configmanager.h"

WBPEConditional::WBPEConditional()
:	m_ConditionalOp( ECO_None )
{
}

WBPEConditional::~WBPEConditional()
{
	SafeDelete( m_InputA );
	SafeDelete( m_InputB );
}

WBPEConditional::EConditionalOp WBPEConditional::GetConditionalOp( const HashedString& ConditionalOp ) const
{
	STATIC_HASHED_STRING( Equals );
	STATIC_HASHED_STRING( E );
	STATIC_HASHED_STRING( NotEquals );
	STATIC_HASHED_STRING( NE );
	STATIC_HASHED_STRING( LessThan );
	STATIC_HASHED_STRING( LT );
	STATIC_HASHED_STRING( LessThanOrEqual );
	STATIC_HASHED_STRING( LTE );
	STATIC_HASHED_STRING( GreaterThan );
	STATIC_HASHED_STRING( GT );
	STATIC_HASHED_STRING( GreaterThanOrEqual );
	STATIC_HASHED_STRING( GTE );

	if( ConditionalOp == sEquals				|| ConditionalOp == sE )	{ return ECO_Equals; }
	if( ConditionalOp == sNotEquals				|| ConditionalOp == sNE )	{ return ECO_NotEquals; }
	if( ConditionalOp == sLessThan				|| ConditionalOp == sLT )	{ return ECO_LessThan; }
	if( ConditionalOp == sLessThanOrEqual		|| ConditionalOp == sLTE )	{ return ECO_LessThanOrEqual; }
	if( ConditionalOp == sGreaterThan			|| ConditionalOp == sGT )	{ return ECO_GreaterThan; }
	if( ConditionalOp == sGreaterThanOrEqual	|| ConditionalOp == sGTE )	{ return ECO_GreaterThanOrEqual; }

	WARN;
	return ECO_None;
}

void WBPEConditional::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBPEBinaryOp::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Op );
	const HashedString ConditionalOp = ConfigManager::GetHash( sOp, HashedString::NullString, sDefinitionName );
	m_ConditionalOp = GetConditionalOp( ConditionalOp );
}

/*virtual*/ void WBPEConditional::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam ValueA;
	m_InputA->Evaluate( Context, ValueA );

	WBParamEvaluator::SEvaluatedParam ValueB;
	m_InputB->Evaluate( Context, ValueB );

	if( m_ConditionalOp == ECO_Equals )
	{
		if( ValueA.m_Type == WBParamEvaluator::EPT_Int && ValueB.m_Type == WBParamEvaluator::EPT_Int )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.m_Int == ValueB.m_Int;
		}
		else if( ValueA.m_Type == WBParamEvaluator::EPT_Float || ValueB.m_Type == WBParamEvaluator::EPT_Float )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.GetFloat() == ValueB.GetFloat();
		}
		else if( ValueA.m_Type == WBParamEvaluator::EPT_String || ValueA.m_Type == WBParamEvaluator::EPT_String )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.GetString() == ValueB.GetString();
		}
		else if( ValueA.m_Type == WBParamEvaluator::EPT_Entity || ValueA.m_Type == WBParamEvaluator::EPT_Entity )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.GetEntity() == ValueB.GetEntity();
		}
		else if( ValueA.m_Type == WBParamEvaluator::EPT_Vector || ValueA.m_Type == WBParamEvaluator::EPT_Vector )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.GetVector() == ValueB.GetVector();
		}
	}
	else if( m_ConditionalOp == ECO_NotEquals )
	{
		if( ValueA.m_Type == WBParamEvaluator::EPT_Int && ValueB.m_Type == WBParamEvaluator::EPT_Int )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.m_Int != ValueB.m_Int;
		}
		else if( ValueA.m_Type == WBParamEvaluator::EPT_Float || ValueB.m_Type == WBParamEvaluator::EPT_Float )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.GetFloat() != ValueB.GetFloat();
		}
		else if( ValueA.m_Type == WBParamEvaluator::EPT_String || ValueA.m_Type == WBParamEvaluator::EPT_String )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.GetString() != ValueB.GetString();
		}
		else if( ValueA.m_Type == WBParamEvaluator::EPT_Entity || ValueA.m_Type == WBParamEvaluator::EPT_Entity )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.GetEntity() != ValueB.GetEntity();
		}
		else if( ValueA.m_Type == WBParamEvaluator::EPT_Vector || ValueA.m_Type == WBParamEvaluator::EPT_Vector )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.GetVector() != ValueB.GetVector();
		}
	}
	else if( m_ConditionalOp == ECO_LessThan )
	{
		if( ValueA.m_Type == WBParamEvaluator::EPT_Int && ValueB.m_Type == WBParamEvaluator::EPT_Int )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.m_Int < ValueB.m_Int;
		}
		else if( ValueA.m_Type == WBParamEvaluator::EPT_Float || ValueB.m_Type == WBParamEvaluator::EPT_Float )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.GetFloat() < ValueB.GetFloat();
		}
	}
	else if( m_ConditionalOp == ECO_LessThanOrEqual )
	{
		if( ValueA.m_Type == WBParamEvaluator::EPT_Int && ValueB.m_Type == WBParamEvaluator::EPT_Int )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.m_Int <= ValueB.m_Int;
		}
		else if( ValueA.m_Type == WBParamEvaluator::EPT_Float || ValueB.m_Type == WBParamEvaluator::EPT_Float )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.GetFloat() <= ValueB.GetFloat();
		}
	}
	else if( m_ConditionalOp == ECO_GreaterThan )
	{
		if( ValueA.m_Type == WBParamEvaluator::EPT_Int && ValueB.m_Type == WBParamEvaluator::EPT_Int )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.m_Int > ValueB.m_Int;
		}
		else if( ValueA.m_Type == WBParamEvaluator::EPT_Float || ValueB.m_Type == WBParamEvaluator::EPT_Float )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.GetFloat() > ValueB.GetFloat();
		}
	}
	else if( m_ConditionalOp == ECO_GreaterThanOrEqual )
	{
		if( ValueA.m_Type == WBParamEvaluator::EPT_Int && ValueB.m_Type == WBParamEvaluator::EPT_Int )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.m_Int >= ValueB.m_Int;
		}
		else if( ValueA.m_Type == WBParamEvaluator::EPT_Float || ValueB.m_Type == WBParamEvaluator::EPT_Float )
		{
			EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Bool;
			EvaluatedParam.m_Bool	= ValueA.GetFloat() >= ValueB.GetFloat();
		}
	}
}