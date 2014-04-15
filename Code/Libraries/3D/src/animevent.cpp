#include "core.h"
#include "animevent.h"
#include "simplestring.h"

AnimEvent::AnimEvent()
:	m_Time( 0.0f )
{
}

AnimEvent::~AnimEvent()
{
}

void AnimEvent::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Unused( DefinitionName );
}

void AnimEvent::Call( Mesh* pMesh, Animation* pAnimation )
{
	Unused( pMesh );
	Unused( pAnimation );
}