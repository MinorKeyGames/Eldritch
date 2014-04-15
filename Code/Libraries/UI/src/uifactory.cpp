#include "core.h"
#include "uifactory.h"
#include "configmanager.h"
#include "uiscreens.h"
#include "uiwidgets.h"

static Map<HashedString, UIScreenFactoryFunc>	sScreenFactoryFuncMap;
static Map<HashedString, UIWidgetFactoryFunc>	sWidgetFactoryFuncMap;

void UIFactory::RegisterUIScreenFactory( const HashedString& TypeName, UIScreenFactoryFunc Factory )
{
	ASSERT( sScreenFactoryFuncMap.Search( TypeName ).IsNull() );
	ASSERT( Factory );
	sScreenFactoryFuncMap[ TypeName ] = Factory;
}

void UIFactory::RegisterUIWidgetFactory( const HashedString& TypeName, UIWidgetFactoryFunc Factory )
{
	ASSERT( sWidgetFactoryFuncMap.Search( TypeName ).IsNull() );
	ASSERT( Factory );
	sWidgetFactoryFuncMap[ TypeName ] = Factory;
}

void UIFactory::InitializeBaseFactories()
{
#define ADDUISCREENFACTORY( type ) UIFactory::RegisterUIScreenFactory( #type, UIScreen##type::Factory );
#include "uiscreens.h"
#undef ADDUISCREENFACTORY
#define ADDUIWIDGETFACTORY( type ) UIFactory::RegisterUIWidgetFactory( #type, UIWidget##type::Factory );
#include "uiwidgets.h"
#undef ADDUIWIDGETFACTORY
}

void UIFactory::ShutDown()
{
	sScreenFactoryFuncMap.Clear();
	sWidgetFactoryFuncMap.Clear();
}

UIScreen* UIFactory::CreateScreen( const SimpleString& DefinitionName )
{
	STATICHASH( UIScreenType );
	MAKEHASH( DefinitionName );
	HashedString UIScreenType = ConfigManager::GetHash( sUIScreenType, "", sDefinitionName );

	Map<HashedString, UIScreenFactoryFunc>::Iterator FactoryIter = sScreenFactoryFuncMap.Search( UIScreenType );
	if( FactoryIter.IsNull() )
	{
		PRINTF( "Invalid type requested for UIScreen %s.\n", DefinitionName.CStr() );
		WARNDESC( "Invalid UIScreen type requested." );
		return NULL;
	}

	UIScreenFactoryFunc pFactory = ( *FactoryIter );
	ASSERT( pFactory );

	UIScreen* const pUIScreen = pFactory();
	ASSERT( pUIScreen );

	pUIScreen->InitializeFromDefinition( DefinitionName );

	return pUIScreen;
}

UIWidget* UIFactory::CreateWidget( const SimpleString& DefinitionName, UIScreen* const pOwnerScreen )
{
	if( DefinitionName == "" )
	{
		return NULL;
	}

	MAKEHASH( DefinitionName );

#if BUILD_FINAL
	STATICHASH( DevOnly );
	const bool DevOnly = ConfigManager::GetBool( sDevOnly, false, sDefinitionName );
	if( DevOnly )
	{
		return NULL;
	}
#endif

	STATICHASH( UIWidgetType );
	HashedString UIWidgetType = ConfigManager::GetHash( sUIWidgetType, "", sDefinitionName );

	Map<HashedString, UIWidgetFactoryFunc>::Iterator FactoryIter = sWidgetFactoryFuncMap.Search( UIWidgetType );
	if( FactoryIter.IsNull() )
	{
		PRINTF( "Invalid type requested for UIWidget %s.\n", DefinitionName.CStr() );
		WARNDESC( "Invalid UIWidget type requested." );
		return NULL;
	}

	UIWidgetFactoryFunc pFactory = ( *FactoryIter );
	ASSERT( pFactory );

	UIWidget* const pUIWidget = pFactory();
	ASSERT( pUIWidget );

	pUIWidget->SetOwnerScreen( pOwnerScreen );
	pUIWidget->InitializeFromDefinition( DefinitionName );

	return pUIWidget;
}