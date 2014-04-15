#ifndef WBPEELDGETCHARACTERVO_H
#define WBPEELDGETCHARACTERVO_H

#include "wbpe.h"

class WBPEEldGetCharacterVO : public WBPE
{
public:
	WBPEEldGetCharacterVO();
	virtual ~WBPEEldGetCharacterVO();

	DEFINE_WBPE_FACTORY( EldGetCharacterVO );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

protected:
	WBPE*			m_EntityPE;
	SimpleString	m_VO;
};

#endif // WBPEELDGETCHARACTERVO_H
