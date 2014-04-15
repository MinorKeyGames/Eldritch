#ifndef WBPESELECTOR_H
#define WBPESELECTOR_H

#include "wbpe.h"
#include "array.h"

class WBPESelector : public WBPE
{
public:
	WBPESelector();
	virtual ~WBPESelector();

	DEFINE_WBPE_FACTORY( Selector );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	struct SSelection
	{
		SSelection()
		:	m_ConditionPE( NULL )
		,	m_ValuePE( NULL )
		{
		}

		WBPE*	m_ConditionPE;
		WBPE*	m_ValuePE;
	};

	Array<SSelection>	m_Selections;
};

#endif // WBPESELECTOR_H