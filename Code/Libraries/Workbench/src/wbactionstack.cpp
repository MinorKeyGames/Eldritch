#include "core.h"
#include "wbactionstack.h"
#include "array.h"
#include "wbevent.h"

static Array<const WBEvent*>* sStack;

void WBActionStack::Initialize()
{
	ASSERT( !sStack );
	sStack = new Array<const WBEvent*>;
	sStack->SetDeflate( false );
}

void WBActionStack::ShutDown()
{
	SafeDelete( sStack );
}

void WBActionStack::Push( const WBEvent& Event )
{
	ASSERT( sStack );
	sStack->PushBack( &Event );
}

void WBActionStack::Pop()
{
	ASSERT( sStack );
	sStack->PopBack();
}

const WBEvent& WBActionStack::Top()
{
	ASSERT( sStack );
	ASSERT( sStack->Size() );
	return *(sStack->Last());
}