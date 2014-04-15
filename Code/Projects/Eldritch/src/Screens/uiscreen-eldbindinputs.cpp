#include "core.h"
#include "uiscreen-eldbindinputs.h"
#include "configmanager.h"
#include "uifactory.h"
#include "inputsystem.h"
#include "eldritchframework.h"

UIScreenEldBindInputs::UIScreenEldBindInputs()
{
}

UIScreenEldBindInputs::~UIScreenEldBindInputs()
{
}

void UIScreenEldBindInputs::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	FlushWidgets();

	UIScreen::InitializeFromDefinition( DefinitionName );

	EldritchFramework* const	pFramework		= EldritchFramework::GetInstance();
	ASSERT( pFramework );

	InputSystem* const			pInputSystem	= pFramework->GetInputSystem();
	ASSERT( pInputSystem );

	const Array<SimpleString>&	ExposedInputs	= pInputSystem->GetExposedInputs();
	ASSERT( ExposedInputs.Size() );

	MAKEHASH( DefinitionName );

	CreateRulesDefinition();

	const uint NumExposedInputs = ExposedInputs.Size();
	for( m_ExposedInputIndex = 0; m_ExposedInputIndex < NumExposedInputs; ++m_ExposedInputIndex )
	{
		m_ExposedInput	= ExposedInputs[ m_ExposedInputIndex ];
		m_Y				= m_YBase + m_ExposedInputIndex * m_YStep;

		CreateLabelWidgetDefinition();
		CreateKeyboardWidgetDefinition();
		CreateMouseWidgetDefinition();
		CreateControllerWidgetDefinition();
		CreateBindingActionDefinition();
		CreateCompositeWidgetDefinition();
		CreateCompositeWidget();
	}

	UpdateRender();

	// Initialize focus to the first element (so that we're never unfocused,
	// because that doesn't make sense for controllers).
	if( m_FocusWidgets.Size() > 0 )
	{
		m_FocusedWidget = m_FocusWidgets[0];
		// NOTE: Intentionally not calling SetFocus here, so effects don't happen; we
		// just need an initial focus for controllers, it's not really *getting* focus
	}
}

void UIScreenEldBindInputs::CreateRulesDefinition()
{
	MAKEHASH( m_Name );

	STATICHASH( Rules );
	const SimpleString UsingRules = ConfigManager::GetString( sRules, "", sm_Name );

	MAKEHASH( UsingRules );

	STATICHASH( Archetype );
	m_ArchetypeName = ConfigManager::GetString( sArchetype, "", sUsingRules );

	STATICHASH( ControllerArchetype );
	m_ControllerArchetypeName = ConfigManager::GetString( sControllerArchetype, "", sUsingRules );

	STATICHASH( Parent );
	m_Parent = ConfigManager::GetString( sParent, "", sUsingRules );

	STATICHASH( PixelYBase );
	m_YBase = ConfigManager::GetFloat( sPixelYBase, 0.0f, sUsingRules );

	STATICHASH( PixelYStep );
	m_YStep = ConfigManager::GetFloat( sPixelYStep, 0.0f, sUsingRules );

	STATICHASH( Column0PixelX );
	m_Column0X = ConfigManager::GetFloat( sColumn0PixelX, 0.0f, sUsingRules );

	STATICHASH( Column1PixelX );
	m_Column1X = ConfigManager::GetFloat( sColumn1PixelX, 0.0f, sUsingRules );

	STATICHASH( Column2PixelX );
	m_Column2X = ConfigManager::GetFloat( sColumn2PixelX, 0.0f, sUsingRules );

	STATICHASH( Column3PixelX );
	m_Column3X = ConfigManager::GetFloat( sColumn3PixelX, 0.0f, sUsingRules );
}

void UIScreenEldBindInputs::CreateLabelWidgetDefinition()
{
	m_LabelWidgetDefinitionName		= SimpleString::PrintF( "_BindLabel%d", m_ExposedInputIndex );
	const SimpleString	LabelString	= SimpleString::PrintF( "Bind%s", m_ExposedInput.CStr() );

	MAKEHASH( m_LabelWidgetDefinitionName );

	STATICHASH( UIWidgetType );
	ConfigManager::SetString( sUIWidgetType, "Text", sm_LabelWidgetDefinitionName );

	STATICHASH( Archetype );
	ConfigManager::SetString( sArchetype, m_ArchetypeName.CStr(), sm_LabelWidgetDefinitionName );

	STATICHASH( Parent );
	ConfigManager::SetString( sParent, m_Parent.CStr(), sm_LabelWidgetDefinitionName );

	STATICHASH( String );
	ConfigManager::SetString( sString, LabelString.CStr(), sm_LabelWidgetDefinitionName );

	STATICHASH( PixelX );
	ConfigManager::SetFloat( sPixelX, m_Column0X, sm_LabelWidgetDefinitionName );

	STATICHASH( PixelY );
	ConfigManager::SetFloat( sPixelY, m_Y, sm_LabelWidgetDefinitionName );
}

void UIScreenEldBindInputs::CreateKeyboardWidgetDefinition()
{
	m_KeyboardWidgetDefinitionName	= SimpleString::PrintF( "_BindKeyboard%d", m_ExposedInputIndex );
	const SimpleString InputString	= SimpleString::PrintF( "#{s:EldritchKeyboard:%s}", m_ExposedInput.CStr() );

	MAKEHASH( m_KeyboardWidgetDefinitionName );

	STATICHASH( UIWidgetType );
	ConfigManager::SetString( sUIWidgetType, "Text", sm_KeyboardWidgetDefinitionName );

	STATICHASH( Archetype );
	ConfigManager::SetString( sArchetype, m_ArchetypeName.CStr(), sm_KeyboardWidgetDefinitionName );

	STATICHASH( Parent );
	ConfigManager::SetString( sParent, m_Parent.CStr(), sm_KeyboardWidgetDefinitionName );

	STATICHASH( IsLiteral );
	ConfigManager::SetBool( sIsLiteral, true, sm_KeyboardWidgetDefinitionName );

	STATICHASH( DynamicString );
	ConfigManager::SetString( sDynamicString, InputString.CStr(), sm_KeyboardWidgetDefinitionName );

	STATICHASH( PixelX );
	ConfigManager::SetFloat( sPixelX, m_Column1X, sm_KeyboardWidgetDefinitionName );

	STATICHASH( PixelY );
	ConfigManager::SetFloat( sPixelY, m_Y, sm_KeyboardWidgetDefinitionName );
}

void UIScreenEldBindInputs::CreateMouseWidgetDefinition()
{
	m_MouseWidgetDefinitionName	= SimpleString::PrintF( "_BindMouse%d", m_ExposedInputIndex );
	const SimpleString InputString	= SimpleString::PrintF( "#{l:EldritchMouse:%s}", m_ExposedInput.CStr() );

	MAKEHASH( m_MouseWidgetDefinitionName );

	STATICHASH( UIWidgetType );
	ConfigManager::SetString( sUIWidgetType, "Text", sm_MouseWidgetDefinitionName );

	STATICHASH( Archetype );
	ConfigManager::SetString( sArchetype, m_ArchetypeName.CStr(), sm_MouseWidgetDefinitionName );

	STATICHASH( Parent );
	ConfigManager::SetString( sParent, m_Parent.CStr(), sm_MouseWidgetDefinitionName );

	STATICHASH( IsLiteral );
	ConfigManager::SetBool( sIsLiteral, true, sm_MouseWidgetDefinitionName );

	STATICHASH( DynamicString );
	ConfigManager::SetString( sDynamicString, InputString.CStr(), sm_MouseWidgetDefinitionName );

	STATICHASH( PixelX );
	ConfigManager::SetFloat( sPixelX, m_Column2X, sm_MouseWidgetDefinitionName );

	STATICHASH( PixelY );
	ConfigManager::SetFloat( sPixelY, m_Y, sm_MouseWidgetDefinitionName );
}

void UIScreenEldBindInputs::CreateControllerWidgetDefinition()
{
	m_ControllerWidgetDefinitionName	= SimpleString::PrintF( "_BindController%d", m_ExposedInputIndex );
	const SimpleString InputString	= SimpleString::PrintF( "#{l:EldritchController:%s}", m_ExposedInput.CStr() );	// Using localization to map name to glyph

	MAKEHASH( m_ControllerWidgetDefinitionName );

	STATICHASH( UIWidgetType );
	ConfigManager::SetString( sUIWidgetType, "Text", sm_ControllerWidgetDefinitionName );

	STATICHASH( Archetype );
	ConfigManager::SetString( sArchetype, m_ControllerArchetypeName.CStr(), sm_ControllerWidgetDefinitionName );

	STATICHASH( Parent );
	ConfigManager::SetString( sParent, m_Parent.CStr(), sm_ControllerWidgetDefinitionName );

	STATICHASH( IsLiteral );
	ConfigManager::SetBool( sIsLiteral, true, sm_ControllerWidgetDefinitionName );

	STATICHASH( DynamicString );
	ConfigManager::SetString( sDynamicString, InputString.CStr(), sm_ControllerWidgetDefinitionName );

	STATICHASH( PixelX );
	ConfigManager::SetFloat( sPixelX, m_Column3X, sm_ControllerWidgetDefinitionName );

	STATICHASH( PixelY );
	ConfigManager::SetFloat( sPixelY, m_Y, sm_ControllerWidgetDefinitionName );
}

void UIScreenEldBindInputs::CreateBindingActionDefinition()
{
	m_BindActionDefinitionName = SimpleString::PrintF( "_BindAction%d", m_ExposedInputIndex );

	MAKEHASH( m_BindActionDefinitionName );

	STATICHASH( ActionType );
	ConfigManager::SetString( sActionType, "EldBindInput", sm_BindActionDefinitionName );

	STATICHASH( Input );
	ConfigManager::SetString( sInput, m_ExposedInput.CStr(), sm_BindActionDefinitionName );
}

void UIScreenEldBindInputs::CreateCompositeWidgetDefinition()
{
	m_CompositeWidgetDefinitionName = SimpleString::PrintF( "_BindComposite%d", m_ExposedInputIndex );

	MAKEHASH( m_CompositeWidgetDefinitionName );

	STATICHASH( UIWidgetType );
	ConfigManager::SetString( sUIWidgetType, "Composite", sm_CompositeWidgetDefinitionName );

	STATICHASH( Focus );
	ConfigManager::SetBool( sFocus, true, sm_CompositeWidgetDefinitionName );

	STATICHASH( NumChildren );
	ConfigManager::SetInt( sNumChildren, 4, sm_CompositeWidgetDefinitionName );

	STATICHASH( Child0 );
	ConfigManager::SetString( sChild0, m_LabelWidgetDefinitionName.CStr(), sm_CompositeWidgetDefinitionName );

	STATICHASH( Child1 );
	ConfigManager::SetString( sChild1, m_KeyboardWidgetDefinitionName.CStr(), sm_CompositeWidgetDefinitionName );

	STATICHASH( Child2 );
	ConfigManager::SetString( sChild2, m_MouseWidgetDefinitionName.CStr(), sm_CompositeWidgetDefinitionName );

	STATICHASH( Child3 );
	ConfigManager::SetString( sChild3, m_ControllerWidgetDefinitionName.CStr(), sm_CompositeWidgetDefinitionName );

	STATICHASH( NumActions );
	ConfigManager::SetInt( sNumActions, 1, sm_CompositeWidgetDefinitionName );

	STATICHASH( Action0 );
	ConfigManager::SetString( sAction0, m_BindActionDefinitionName.CStr(), sm_CompositeWidgetDefinitionName );
}

void UIScreenEldBindInputs::CreateCompositeWidget()
{
	UIWidget* const pCompositeWidget = UIFactory::CreateWidget( m_CompositeWidgetDefinitionName, this );
	ASSERT( pCompositeWidget );

	AddWidget( pCompositeWidget );
}