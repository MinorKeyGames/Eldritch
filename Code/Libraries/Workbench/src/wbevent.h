#ifndef WBEVENT_H
#define WBEVENT_H

// NOTE: I've removed Strings from events, because managing their memory is tricky,
// they wouldn't transmit across a network properly without special consideration,
// and I haven't absolutely needed them yet.

#include "map.h"
#include "hashedstring.h"
#include "vector.h"
#include "angles.h"
#include "wbentityref.h"

class WBParamEvaluator;
class IDataStream;

// I can turn this back on if needed, but I'm not really sure how useful it will be right now.
#define LOG_EVENTS 0

// Create an event and add context parameters.
// Event is assigned an automatic variable name.
#define WB_MAKE_EVENT( name, owner )								\
	WBEvent name##AutoEvent;										\
	STATIC_HASHED_STRING( name );									\
	name##AutoEvent.SetEventName( s##name );						\
	WBEntity* const p##name##AutoOwner = ( owner );					\
	if( p##name##AutoOwner )										\
	{																\
		p##name##AutoOwner->AddContextToEvent( name##AutoEvent );	\
	}

#define WB_AUTO_EVENT( name ) name##AutoEvent

// For use with auto events created by WB_MAKE_EVENT macro.
#define WB_SET_AUTO( event, type, name, value )				\
	STATIC_HASHED_STRING( name );							\
	( event##AutoEvent ).Set##type( s##name, ( value ) )

#define WB_SET_AUTO_PE( event, name, pe )	\
	STATIC_HASHED_STRING( name );			\
	( event##AutoEvent ).Set( s##name, pe )

// For use with any (non-auto) event, but mainly for adding context parameters.
#define WB_SET_CONTEXT( event, type, name, value )	\
	STATIC_HASHED_STRING( name );					\
	( event ).Set##type( s##name, ( value ) )

#if LOG_EVENTS

#define WB_LOG_EVENT( event )								\
	STATIC_HASHED_STRING( WBEvent_LogEvent );				\
	( event##AutoEvent ).SetBool( sWBEvent_LogEvent, true )

#else

#define WB_LOG_EVENT( event ) DoNothing

#endif

class WBPackedEvent
{
public:
	WBPackedEvent()
	:	m_PackedEvent()
	{
		m_PackedEvent.SetDeflate( false );
	}

	WBPackedEvent( const void* const pData, const uint Size )
	:	m_PackedEvent()
	{
		m_PackedEvent.SetDeflate( false );
		Reinit( pData, Size );
	}

	void		Reinit( const void* const pData, const uint Size )
	{
		m_PackedEvent.Resize( Size );
		if( pData )
		{
			memcpy( m_PackedEvent.GetData(), pData, Size );
		}
	}

	void*		GetData() const { return m_PackedEvent.GetData(); }
	uint		GetSize() const { return m_PackedEvent.Size(); }

	WBEvent		Unpack() const;

private:
	Array<byte>	m_PackedEvent;

	friend class WBEvent;	// So WBEvent can access m_PackedEvent
};

class WBEvent
{
public:
	WBEvent();
	~WBEvent();

	enum EType
	{
		EWBEPT_None,	// Size 0
		EWBEPT_Bool,	// Size 1
		EWBEPT_Int,		// Size 4
		EWBEPT_Float,	// Size 4
		EWBEPT_Hash,	// Size 4
		EWBEPT_Vector,	// Size 12
		EWBEPT_Angles,	// Size 12
		EWBEPT_Entity,	// Size 8
		EWBEPT_Pointer,	// Size 4
	};

	struct SParameter
	{
		SParameter()
		:	m_Type( EWBEPT_None )
		,	m_Data1( 0 )
		,	m_Data2( 0 )
		,	m_Data3( 0 )
		{
		}

		EType	m_Type;
		uint	m_Data1;
		uint	m_Data2;
		uint	m_Data3;

		void	SetBool( const bool Value )				{ m_Type = EWBEPT_Bool;		*( bool* )( &m_Data1 ) = Value;			}
		void	SetInt( const int Value )				{ m_Type = EWBEPT_Int;		*( int* )( &m_Data1 ) = Value;			}
		void	SetFloat( const float Value )			{ m_Type = EWBEPT_Float;	*( float* )( &m_Data1 ) = Value;		}
		void	SetHash( const HashedString& Value )	{ m_Type = EWBEPT_Hash;		*( HashedString* )( &m_Data1 ) = Value;	}
		void	SetVector( const Vector& Value )		{ m_Type = EWBEPT_Vector;	*( Vector* )( &m_Data1 ) = Value;		}
		void	SetAngles( const Angles& Value )		{ m_Type = EWBEPT_Angles;	*( Angles* )( &m_Data1 ) = Value;		}
		void	SetEntity( const WBEntityRef& Value )	{ m_Type = EWBEPT_Entity;	*( WBEntityRef* )( &m_Data1 ) = Value;	}
		void	SetPointer( void* const Value )			{ m_Type = EWBEPT_Pointer;	*( void** )( &m_Data1 ) = Value;		}

		EType			GetType() const { return m_Type; }

		bool			GetBool() const		{ DEVASSERT( m_Type == EWBEPT_Bool );		return *( bool* )( &m_Data1 );			}
		int				GetInt() const		{ DEVASSERT( m_Type == EWBEPT_Int );		return *( int* )( &m_Data1 );			}
		float			GetFloat() const	{ DEVASSERT( m_Type == EWBEPT_Float );		return *( float* )( &m_Data1 );			}
		HashedString&	GetHash() const		{ DEVASSERT( m_Type == EWBEPT_Hash );		return *( HashedString* )( &m_Data1 );	}
		Vector&			GetVector() const	{ DEVASSERT( m_Type == EWBEPT_Vector );		return *( Vector* )( &m_Data1 );		}
		Angles&			GetAngles() const	{ DEVASSERT( m_Type == EWBEPT_Angles );		return *( Angles* )( &m_Data1 );		}
		WBEntityRef&	GetEntity() const	{ DEVASSERT( m_Type == EWBEPT_Entity );		return *( WBEntityRef* )( &m_Data1 );	}
		void*			GetPointer() const	{ DEVASSERT( m_Type == EWBEPT_Pointer );	return *( void** )( &m_Data1 );			}

		bool			CoerceBool() const;
		int				CoerceInt() const;
		float			CoerceFloat() const;
		HashedString	CoerceHash() const;
		Vector			CoerceVector() const;
		Angles			CoerceAngles() const;
		WBEntity*		CoerceEntity() const;
		void*			CoercePointer() const;

		// Even though string isn't a supported type, it's useful for debugging.
		SimpleString	CoerceString() const;
	};

	typedef Map<HashedString, SParameter> TParameterMap;

	void	SetBool( const HashedString& Name, const bool Value )			{ m_Parameters[ Name ].SetBool( Value ); }
	void	SetInt( const HashedString& Name, const int Value )				{ m_Parameters[ Name ].SetInt( Value ); }
	void	SetFloat( const HashedString& Name, const float Value )			{ m_Parameters[ Name ].SetFloat( Value ); }
	void	SetHash( const HashedString& Name, const HashedString& Value )	{ m_Parameters[ Name ].SetHash( Value ); }
	void	SetVector( const HashedString& Name, const Vector& Value )		{ m_Parameters[ Name ].SetVector( Value ); }
	void	SetAngles( const HashedString& Name, const Angles& Value )		{ m_Parameters[ Name ].SetAngles( Value ); }
	void	SetEntity( const HashedString& Name, const WBEntityRef& Value )	{ m_Parameters[ Name ].SetEntity( Value ); }
	void	SetPointer( const HashedString& Name, void* const Value )		{ m_Parameters[ Name ].SetPointer( Value ); }

	void	Set( const HashedString& Name, const SParameter* const pParameter );
	void	Set( const HashedString& Name, const WBParamEvaluator& PE );

	uint	Size() const	{ return m_Parameters.Size(); }
	void	Clear()			{ m_Parameters.Clear(); }

	void			SetEventName( const HashedString& Value );
	HashedString	GetEventName() const;

	const TParameterMap&	GetParameters() const { return m_Parameters; }
	const SParameter*		GetParameter( const HashedString& Name ) const;
	EType					GetType( const HashedString& Name ) const;

	bool					GetBool( const HashedString& Name ) const;
	int						GetInt( const HashedString& Name ) const;
	float					GetFloat( const HashedString& Name ) const;
	HashedString			GetHash( const HashedString& Name ) const;
	Vector					GetVector( const HashedString& Name ) const;
	Angles					GetAngles( const HashedString& Name ) const;
	WBEntity*				GetEntity( const HashedString& Name ) const;
	void*					GetPointer( const HashedString& Name ) const;
	template<class C> C*	GetPointer( const HashedString& Name ) const { return static_cast<C*>( GetPointer( Name ) ); }

	// Even though string isn't a fully supported type, it's useful to coerce from other types.
	SimpleString	GetString( const HashedString& Name ) const;

	WBPackedEvent	Pack() const;
	void			Pack( WBPackedEvent& PackedEvent ) const;
	void			Unpack( const WBPackedEvent& PackedEvent );

	uint			GetSerializationSize() const;
	void			Save( const IDataStream& Stream ) const;
	void			Load( const IDataStream& Stream );

private:
	TParameterMap	m_Parameters;
};

#endif // WBEVENT_H