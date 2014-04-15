#ifndef WBACTION_H
#define WBACTION_H

class SimpleString;
class WBEntity;

#define DEFINE_WBACTION_FACTORY( type ) static class WBAction* Factory() { return new WBAction##type; }
typedef class WBAction* ( *WBActionFactoryFunc )( void );

class WBAction
{
public:
	WBAction();
	virtual ~WBAction();

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

	WBEntity*		GetEntity() const;
	WBEntity*		GetOwner() const;
};

#endif // WBACTION_H