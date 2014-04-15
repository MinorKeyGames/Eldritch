#include "core.h"
#include "wbeventmanager.h"
#include "iwbeventobserver.h"
#include "clock.h"
#include "idatastream.h"

#if LOG_EVENTS
#include "reversehash.h"
#endif

#if BUILD_DEBUG

#define BEGIN_ITERATING_QUEUE do{ m_IteratingQueue = true; } while(0)
#define END_ITERATING_QUEUE do{ m_IteratingQueue = false; } while(0)
#define CHECK_ITERATING_QUEUE do{ DEBUGASSERT( !m_IteratingQueue ); } while(0)

#define BEGIN_ITERATING_OBSERVERS do{ m_IteratingObservers = true; } while(0)
#define END_ITERATING_OBSERVERS do{ m_IteratingObservers = false; } while(0)
#define CHECK_ITERATING_OBSERVERS do{ DEBUGASSERT( !m_IteratingObservers ); } while(0)

#else

#define BEGIN_ITERATING_QUEUE DoNothing
#define END_ITERATING_QUEUE DoNothing
#define CHECK_ITERATING_QUEUE DoNothing

#define BEGIN_ITERATING_OBSERVERS DoNothing
#define END_ITERATING_OBSERVERS DoNothing
#define CHECK_ITERATING_OBSERVERS DoNothing

#endif

WBEventManager::WBEventManager()
:	m_EventQueue()
,	m_EventQueueAdd()
,	m_EventObservers()
,	m_LastQueueUID( 0 )
,	m_InsideDispatch( false )
,	m_DestroyAfterDispatch( false )
#if BUILD_DEBUG
,	m_IteratingQueue( false )
,	m_IteratingObservers( false )
#endif
{
	m_EventQueue.SetDeflate( false );
	m_EventQueueAdd.SetDeflate( false );
}

WBEventManager::~WBEventManager()
{
}

void WBEventManager::Destroy()
{
	if( m_InsideDispatch )
	{
		m_DestroyAfterDispatch = true;
	}
	else
	{
		SafeDeleteNoNull( this );
	}
}

void WBEventManager::DispatchEvent( const WBEvent& Event, IWBEventObserver* const Recipient ) const
{
	InternalDispatchEvent( Event, Recipient );
}

TEventUID WBEventManager::QueueEvent( const WBEvent& Event, IWBEventObserver* const Recipient, const float DispatchDelay /*= 0.0f*/ )
{
	SQueuedEvent NewEvent;
	NewEvent.m_Event		= Event;
	NewEvent.m_UID			= ++m_LastQueueUID;
	NewEvent.m_Recipient	= Recipient;
	NewEvent.m_DispatchTime	= DispatchDelay + WBWorld::GetInstance()->GetTime();

	m_EventQueueAdd.PushBack( NewEvent );

	return m_LastQueueUID;
}

void WBEventManager::UnqueueEvent( const TEventUID& EventUID )
{
	for( uint QueuedEventIndex = 0; QueuedEventIndex < m_EventQueue.Size(); ++QueuedEventIndex )
	{
		SQueuedEvent& QueuedEvent = m_EventQueue[ QueuedEventIndex ];
		if( !QueuedEvent.m_Unqueue && QueuedEvent.m_UID == EventUID )
		{
			QueuedEvent.m_Unqueue = true;
		}
	}

	for( uint QueuedEventIndex = 0; QueuedEventIndex < m_EventQueueAdd.Size(); ++QueuedEventIndex )
	{
		SQueuedEvent& QueuedEvent = m_EventQueueAdd[ QueuedEventIndex ];
		if( !QueuedEvent.m_Unqueue && QueuedEvent.m_UID == EventUID )
		{
			QueuedEvent.m_Unqueue = true;
		}
	}
}

void WBEventManager::UnqueueEvents( IWBEventObserver* const Recipient )
{
	for( uint QueuedEventIndex = 0; QueuedEventIndex < m_EventQueue.Size(); ++QueuedEventIndex )
	{
		SQueuedEvent& QueuedEvent = m_EventQueue[ QueuedEventIndex ];
		if( !QueuedEvent.m_Unqueue && QueuedEvent.m_Recipient == Recipient )
		{
			QueuedEvent.m_Unqueue = true;
		}
	}

	for( uint QueuedEventIndex = 0; QueuedEventIndex < m_EventQueueAdd.Size(); ++QueuedEventIndex )
	{
		SQueuedEvent& QueuedEvent = m_EventQueueAdd[ QueuedEventIndex ];
		if( !QueuedEvent.m_Unqueue && QueuedEvent.m_Recipient == Recipient )
		{
			QueuedEvent.m_Unqueue = true;
		}
	}
}

void WBEventManager::Flush()
{
	CHECK_ITERATING_QUEUE;
	m_EventQueue.Clear();
}

void WBEventManager::AddObserver( const HashedString& EventName, IWBEventObserver* const Observer, IWBEventObserver* const Recipient /*= NULL*/ )
{
	CHECK_ITERATING_OBSERVERS;

	ASSERT( EventName );
	ASSERT( Observer );

	SObserver ObserverEntry;
	ObserverEntry.m_Observer = Observer;
	ObserverEntry.m_Recipient = Recipient;

	m_EventObservers.Insert( EventName, ObserverEntry );
}

void WBEventManager::RemoveObserver( const HashedString& EventName, IWBEventObserver* const Observer, IWBEventObserver* const Recipient /*= NULL*/ )
{
	CHECK_ITERATING_OBSERVERS;

	ASSERT( EventName );
	ASSERT( Observer );

	SObserver ObserverEntry;
	ObserverEntry.m_Observer = Observer;
	ObserverEntry.m_Recipient = Recipient;

	m_EventObservers.Remove( EventName, ObserverEntry );
}

void WBEventManager::RemoveObserver( IWBEventObserver* const Observer )
{
	CHECK_ITERATING_OBSERVERS;

	ASSERT( Observer );

	FOR_EACH_MULTIMAP_NOINCR( ObserverIter, m_EventObservers, HashedString, SObserver )
	{
		const SObserver& ObserverEntry = ObserverIter.GetValue();
		if( ObserverEntry.m_Observer == Observer )
		{
			m_EventObservers.Remove( ObserverIter );
		}
		else
		{
			++ObserverIter;
		}
	}
}

void WBEventManager::PushAddQueueEvents()
{
	XTRACE_FUNCTION;

	// Array::Append doesn't work because it does a shallow copy.
	FOR_EACH_ARRAY( AddEventIter, m_EventQueueAdd, SQueuedEvent )
	{
		const SQueuedEvent& Event = AddEventIter.GetValue();
		if( Event.m_Unqueue )
		{
			// Ignore
		}
		else
		{
			m_EventQueue.PushBack( Event );
		}
	}

	m_EventQueueAdd.Clear();
}

void WBEventManager::Tick()
{
	XTRACE_FUNCTION;

	const float CurrentTime = WBWorld::GetInstance()->GetTime();

	// Make sure we have all new queued events before ticking.
	PushAddQueueEvents();

	// Dispatch events whose delay has elapsed.
	XTRACE_BEGIN( DispatchEvents );
		BEGIN_ITERATING_QUEUE;
		for( uint QueuedEventIndex = 0; QueuedEventIndex < m_EventQueue.Size(); ++QueuedEventIndex )
		{
			SQueuedEvent& QueuedEvent = m_EventQueue[ QueuedEventIndex ];
			if( !QueuedEvent.m_Unqueue && QueuedEvent.m_DispatchTime <= CurrentTime )
			{
				QueuedEvent.m_Unqueue = true;
				InternalDispatchEvent( QueuedEvent.m_Event, QueuedEvent.m_Recipient );
			}
		}
		END_ITERATING_QUEUE;
	XTRACE_END;

	// Remove dispatched or otherwise unqueued events.
	XTRACE_BEGIN( RemoveEvents );
		for( int QueuedEventIndex = m_EventQueue.Size() - 1; QueuedEventIndex >= 0; --QueuedEventIndex )
		{
			const SQueuedEvent& QueuedEvent = m_EventQueue[ QueuedEventIndex ];
			if( QueuedEvent.m_Unqueue )
			{
				// Slow remove to maintain order.
				m_EventQueue.Remove( QueuedEventIndex );
			}
		}
	XTRACE_END;

	// And make sure we have all new queued events after ticking.
	PushAddQueueEvents();
}

void WBEventManager::InternalDispatchEvent( const WBEvent& Event, IWBEventObserver* const Recipient ) const
{
	XTRACE_FUNCTION;

	m_InsideDispatch = true;

#if LOG_EVENTS
	STATIC_HASHED_STRING( WBEvent_LogEvent );
	const bool LogEvent = Event.GetBool( sWBEvent_LogEvent );
	if( LogEvent )
	{
		const SimpleString		EventName		= ReverseHash::ReversedHash( Event.GetEventName() );

		STATIC_HASHED_STRING( EventOwner );
		const WBEntity* const	pEventOwner		= Event.GetEntity( sEventOwner );
		const SimpleString		EventOwnerName	= pEventOwner ? pEventOwner->GetUniqueName() : "None";

		const WBEntityRef		RecipientEntity	= Recipient ? Recipient->GetEntityUID() : 0;
		const WBEntity* const	pRecipient		= RecipientEntity.Get();
		const SimpleString		RecipientName	= pRecipient ? pRecipient->GetUniqueName() : "None";

		const float				CurrentTime		= WBWorld::GetInstance()->GetTime();

		if( EventOwnerName == RecipientName )
		{
			PRINTF( "WBE %.2f: %s Own: %s\n", CurrentTime, EventName.CStr(), EventOwnerName.CStr() );
		}
		else
		{
			PRINTF( "WBE %.2f: %s Own: %s Rec: %s\n", CurrentTime, EventName.CStr(), EventOwnerName.CStr(), RecipientName.CStr() );
		}
	}
#endif

	if( Recipient )
	{
		Recipient->HandleEvent( Event );
	}

	BEGIN_ITERATING_OBSERVERS;
	FOR_EACH_MULTIMAP_SEARCH( ObserverIter, m_EventObservers, HashedString, SObserver, Event.GetEventName() )
	{
		const SObserver& ObserverEntry = ObserverIter.GetValue();

		IWBEventObserver* const Observer = ObserverEntry.m_Observer;
		ASSERT( Observer );
		if( Observer == Recipient )
		{
			// Don't let recipient handle it twice if it's registered as an observer.
			continue;
		}

		IWBEventObserver* const ObserverRecipient = ObserverEntry.m_Recipient;
		if( ObserverRecipient && ObserverRecipient != Recipient )
		{
			// Ignore if observer isn't listening for this recipient (and isn't a global observer).
			continue;
		}

		Observer->HandleEvent( Event );
	}
	END_ITERATING_OBSERVERS;

	m_InsideDispatch = false;
	if( m_DestroyAfterDispatch )
	{
		SafeDeleteNoNull( this );
	}
}

// Observers are not serialized. Registered observers will remain registered, and new observers should reregister.

#define VERSION_EMPTY		0
#define VERSION_UIDS		1
#define VERSION_EVENTUIDS	2
#define VERSION_CURRENT		2

void WBEventManager::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const float CurrentTime = WBWorld::GetInstance()->GetTime();

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_UIDS )
	{
		m_LastQueueUID = Stream.ReadUInt32();
	}

	const uint NumQueuedEvents = Stream.ReadUInt32();
	for( uint QueuedEventIndex = 0; QueuedEventIndex < NumQueuedEvents; ++QueuedEventIndex )
	{
		const bool Unqueued = Stream.ReadBool();
		if( Unqueued )
		{
			continue;
		}

		const float RemainingTime = Stream.ReadFloat();

		const uint PackedEventSize = Stream.ReadUInt32();
		WBPackedEvent PackedEvent;
		PackedEvent.Reinit( NULL, PackedEventSize );
		Stream.Read( PackedEventSize, PackedEvent.GetData() );

		// NOTE: Entities are the only recipients that can be serialized currently.
		const uint EntityUID = Stream.ReadUInt32();
		IWBEventObserver* pRecipient = WBWorld::GetInstance()->GetEntity( EntityUID );

		const TEventUID EventUID = ( Version >= VERSION_EVENTUIDS ) ? Stream.ReadUInt32() : 0;

		SQueuedEvent NewEvent;
		NewEvent.m_Event.Unpack( PackedEvent );
		NewEvent.m_Recipient = pRecipient;
		NewEvent.m_DispatchTime = RemainingTime + CurrentTime;
		NewEvent.m_UID = EventUID;

		m_EventQueue.PushBack( NewEvent );
	}
}

void WBEventManager::Save( const IDataStream& Stream ) const
{
	XTRACE_FUNCTION;

	const float CurrentTime = WBWorld::GetInstance()->GetTime();

	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteUInt32( m_LastQueueUID );

	Stream.WriteUInt32( m_EventQueue.Size() );
	for( uint QueuedEventIndex = 0; QueuedEventIndex < m_EventQueue.Size(); ++QueuedEventIndex )
	{
		const SQueuedEvent& QueuedEvent = m_EventQueue[ QueuedEventIndex ];

		Stream.WriteBool( QueuedEvent.m_Unqueue );
		if( QueuedEvent.m_Unqueue )
		{
			continue;
		}

		const float RemainingTime = QueuedEvent.m_DispatchTime - CurrentTime;
		Stream.WriteFloat( RemainingTime );

		WBPackedEvent PackedEvent;
		QueuedEvent.m_Event.Pack( PackedEvent );

		Stream.WriteUInt32( PackedEvent.GetSize() );
		Stream.Write( PackedEvent.GetSize(), PackedEvent.GetData() );

		Stream.WriteUInt32( QueuedEvent.m_Recipient ? QueuedEvent.m_Recipient->GetEntityUID() : 0 );
		Stream.WriteUInt32( QueuedEvent.m_UID );
	}
}