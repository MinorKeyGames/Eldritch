#ifndef WBPECONDITIONAL_H
#define WBPECONDITIONAL_H

#include "wbpebinaryop.h"

class WBPEConditional : public WBPEBinaryOp
{
public:
	WBPEConditional();
	virtual ~WBPEConditional();

	DEFINE_WBPE_FACTORY( Conditional );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	enum EConditionalOp
	{
		ECO_None,
		ECO_Equals,
		ECO_NotEquals,
		ECO_LessThan,
		ECO_LessThanOrEqual,
		ECO_GreaterThan,
		ECO_GreaterThanOrEqual,
	};

	EConditionalOp	GetConditionalOp( const HashedString& ConditionalOp ) const;

	EConditionalOp	m_ConditionalOp;
};

#endif // WBPECONDITIONAL_H
