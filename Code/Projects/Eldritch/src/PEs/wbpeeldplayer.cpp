#include "core.h"
#include "wbpeeldplayer.h"
#include "eldritchgame.h"

WBPEEldPlayer::WBPEEldPlayer()
{
}

WBPEEldPlayer::~WBPEEldPlayer()
{
}

/*virtual*/ void WBPEEldPlayer::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Entity;
	EvaluatedParam.m_Entity	= EldritchGame::GetPlayer();
}