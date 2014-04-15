#ifndef UI_H
#define UI_H

class UIManager;
class UIScreen;
class UIWidget;

typedef void ( *UICallback )( void* /*widget/screen*/, void* /*metadata*/ );

#if BUILD_DEBUG
#if BUILD_WINDOWS
typedef	__int64		debug_int64_t;
#elif BUILD_LINUX
typedef	int64_t		debug_int64_t;
#elif BUILD_MAC
typedef __int64_t	debug_int64_t;
#endif
#endif

struct SUICallback
{
	SUICallback()
	:	m_Callback( NULL )
	,	m_Void( NULL )
	{
	}

	SUICallback( UICallback Callback, void* pVoid )
	:	m_Callback( Callback )
	,	m_Void( pVoid )
	{
	}

	UICallback	m_Callback;
	void*		m_Void;
};

#endif // UI_H