#ifndef EXCEPTIONTRACE_H
#define EXCEPTIONTRACE_H

// Can be compiled out for perf if desired.
#define XAUTOTRACES	( 1 && BUILD_WINDOWS )

#if XAUTOTRACES

#define XTRACE_INTERNAL( tracename, autotrace )	ExceptionTrace::SAutoTrace autotrace( tracename )

#define XTRACE_BEGIN( tracename )				{ XTRACE_INTERNAL( #tracename, tracename##AutoTrace )
#define XTRACE_END								}
#define XTRACE_NAMED( tracename )				XTRACE_INTERNAL( #tracename, tracename##AutoTrace )
#define XTRACE_FUNCTION							XTRACE_INTERNAL( __FUNCSIG__, FunctionAutoTrace )

#else

#define XTRACE_BEGIN( tracename )	{
#define XTRACE_END					}
#define XTRACE_NAMED( tracename )	DoNothing
#define XTRACE_FUNCTION				DoNothing

#endif

class SimpleString;

namespace ExceptionTrace
{
	void	Enable();
	void	ShutDown();

	void	Push( const char* const TraceName );
	void	Pop();

	void	PrintTrace();

	struct SAutoTrace
	{
		SAutoTrace( const char* const TraceName )
		{
			ExceptionTrace::Push( TraceName );
		}

		~SAutoTrace()
		{
			ExceptionTrace::Pop();
		}
	};
}

#endif // EXCEPTIONTRACE_H