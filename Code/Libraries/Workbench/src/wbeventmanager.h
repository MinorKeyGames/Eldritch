#ifndef WBEVENTMANAGER_H
#define WBEVENTMANAGER_H

#include "array.h"
#include "multimap.h"
#include "wbevent.h"
#include "clock.h"

class IWBEventObserver;
class IDataStream;

// For use with auto events created by WB_MAKE_EVENT macro.
#define WB_DISPATCH_EVENT( mgr, event, rec ) ( mgr )->DispatchEvent( ( event##AutoEvent ), ( rec ) )
#define WB_QUEUE_EVENT( mgr, event, rec ) ( mgr )->QueueEvent( ( event##AutoEvent ), ( rec ) )
#define WB_QUEUE_EVENT_DELAY( mgr, event, rec, delay ) ( mgr )->QueueEvent( ( event##AutoEvent ), ( rec ), ( delay ) )

typedef uint TEventUID;

class WBEventManager
{
public:
	WBEventManager();
	~WBEventManager();

	void		DispatchEvent( const WBEvent& Event, IWBEventObserver* const Recipient ) const;
	TEventUID	QueueEvent( const WBEvent& Event, IWBEventObserver* const Recipient, const float DispatchDelay = 0.0f );

	void		UnqueueEvent( const TEventUID& EventUID );
	void		UnqueueEvents( IWBEventObserver* const Recipient );

	void		Flush();

	void		AddObserver( const HashedString& EventName, IWBEventObserver* const Observer, IWBEventObserver* const Recipient = NULL );
	void		RemoveObserver( const HashedString& EventName, IWBEventObserver* const Observer, IWBEventObserver* const Recipient = NULL );
	void		RemoveObserver( IWBEventObserver* const Observer );

	void		Tick();

	void		Load( const IDataStream& Stream );
	void		Save( const IDataStream& Stream ) const;

	void		Destroy();

private:
	void		PushAddQueueEvents();
	void		InternalDispatchEvent( const WBEvent& Event, IWBEventObserver* const Recipient ) const;

	struct SQueuedEvent
	{
		SQueuedEvent()
		:	m_Event()
		,	m_UID( 0 )
		,	m_Recipient( NULL )
		,	m_DispatchTime( 0.0f )
		,	m_Unqueue( false )
		{
		}

		WBEvent				m_Event;
		TEventUID			m_UID;
		IWBEventObserver*	m_Recipient;
		float				m_DispatchTime;
		bool				m_Unqueue;
	};

	struct SObserver
	{
		SObserver()
		:	m_Observer( NULL )
		,	m_Recipient( NULL )
		{
		}

		bool operator==( const SObserver& Other )
		{
			return m_Observer == Other.m_Observer && m_Recipient == Other.m_Recipient;
		}

		IWBEventObserver*	m_Observer;
		IWBEventObserver*	m_Recipient;	// Optional, only observe this recipient's events
	};

	Array<SQueuedEvent>					m_EventQueue;
	Array<SQueuedEvent>					m_EventQueueAdd;
	Multimap<HashedString, SObserver>	m_EventObservers;
	TEventUID							m_LastQueueUID;

	mutable bool						m_InsideDispatch;
	mutable bool						m_DestroyAfterDispatch;

#if BUILD_DEBUG
	mutable bool						m_IteratingQueue;
	mutable bool						m_IteratingObservers;
#endif
};

#endif // WBEVENTMANAGER_H