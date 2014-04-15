#ifndef ANIMEVENTELDEXECUTEACTION_H
#define ANIMEVENTELDEXECUTEACTION_H

#include "animevent.h"
#include "wbaction.h"
#include "array.h"

class AnimEventEldExecuteAction : public AnimEvent
{
public:
	AnimEventEldExecuteAction();
	virtual ~AnimEventEldExecuteAction();

	DEFINE_ANIMEVENT( EldExecuteAction );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Call( Mesh* pMesh, Animation* pAnimation );

private:
	Array<WBAction*>	m_Actions;
};

#endif // ANIMEVENTELDEXECUTEACTION_H