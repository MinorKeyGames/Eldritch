#ifndef UIFACTORY_H
#define UIFACTORY_H

#include "uiscreen.h"
#include "uiwidget.h"

class SimpleString;

namespace UIFactory
{
	void	RegisterUIScreenFactory( const HashedString& TypeName, UIScreenFactoryFunc Factory );
	void	RegisterUIWidgetFactory( const HashedString& TypeName, UIWidgetFactoryFunc Factory );

	void	InitializeBaseFactories();
	void	ShutDown();

	UIScreen*	CreateScreen( const SimpleString& DefinitionName );
	UIWidget*	CreateWidget( const SimpleString& DefinitionName, UIScreen* const pOwnerScreen );
};

#endif // UIFACTORY_H