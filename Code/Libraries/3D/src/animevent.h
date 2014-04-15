#ifndef ANIMEVENT_H
#define ANIMEVENT_H

// AnimEvents can be used in two ways: an event that can handle itself
// can use Call(). An event that should be handled by an animation
// listener will be sent to a listener callback and its type can be
// determined with GetEventName() (veeery simple form of RTTI).

typedef class AnimEvent* ( *AnimEventFactoryFunc )( void );

class SimpleString;
class Mesh;
class Animation;

#define DEFINE_ANIMEVENT_FACTORY( type ) static class AnimEvent* Factory() { return new AnimEvent##type; }

#define DEFINE_ANIMEVENT_STATICNAME( type ) static const char* GetStaticEventName() { static const char* sEventName = #type; return sEventName; }
#define DEFINE_ANIMEVENT_NAME virtual const char* GetEventName() { return GetStaticEventName(); }

#define DEFINE_ANIMEVENT( type )		\
	DEFINE_ANIMEVENT_FACTORY( type )	\
	DEFINE_ANIMEVENT_STATICNAME( type )	\
	DEFINE_ANIMEVENT_NAME

class AnimEvent
{
public:
	AnimEvent();
	virtual ~AnimEvent();

	virtual const char*	GetEventName() = 0;

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void		Call( Mesh* pMesh, Animation* pAnimation );

	float	m_Time;
};

#endif // ANIMEVENT_H