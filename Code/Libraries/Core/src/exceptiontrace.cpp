#include "core.h"
#include "exceptiontrace.h"
#include "array.h"
#include "simplestring.h"

static bool					gIsEnabled	= false;
static Array<const char*>*	gStack		= NULL;

void ExceptionTrace::Enable()
{
	ASSERT( !gIsEnabled );

	gIsEnabled	= true;
	gStack		= new Array<const char*>;
	gStack->SetDeflate( false );
}

void ExceptionTrace::ShutDown()
{
	ASSERT( gIsEnabled );

	gIsEnabled	= false;
	SafeDelete( gStack );
}

void ExceptionTrace::Push( const char* const TraceName )
{
	if( gIsEnabled )
	{
		gStack->PushBack( TraceName );
	}
}

void ExceptionTrace::Pop()
{
	if( gIsEnabled )
	{
		gStack->PopBack();
	}
}

void ExceptionTrace::PrintTrace()
{
	if( !gIsEnabled )
	{
		return;
	}

	PRINTF( "Exception trace:\n" );
	const Array<const char*>& StackArray = *gStack;
	FOR_EACH_ARRAY_REVERSE( StackIter, StackArray, const char* )
	{
		const char* const TraceName = StackIter.GetValue();
		PRINTF( "\t%s\n", TraceName );
	}
}