#ifndef WBPEOWNER_H
#define WBPEOWNER_H

#include "wbpe.h"

class WBPEOwner : public WBPE
{
public:
	WBPEOwner();
	virtual ~WBPEOwner();

	DEFINE_WBPE_FACTORY( Owner );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	bool	m_Topmost;
};

#endif // WBPEOWNER_H
