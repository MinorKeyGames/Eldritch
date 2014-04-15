#ifndef WBACTIONELDCHECKLINE_H
#define WBACTIONELDCHECKLINE_H

#include "wbaction.h"
#include "wbparamevaluator.h"

class WBActionEldCheckLine : public WBAction
{
public:
	WBActionEldCheckLine();
	virtual ~WBActionEldCheckLine();

	DEFINE_WBACTION_FACTORY( EldCheckLine );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	void			GetLineTransform( Vector& OutLocation, Angles& OutOrientation ) const;

	float			m_LineLength;
	HashedString	m_CheckTag;
};

#endif // WBACTIONELDCHECKLINE_H