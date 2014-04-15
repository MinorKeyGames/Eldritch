#include "core.h"
#include "uiwidget-composite.h"
#include "uimanager.h"
#include "3d.h"
#include "configmanager.h"
#include "mathcore.h"
#include "uifactory.h"

UIWidgetComposite::UIWidgetComposite()
:	m_Children()
{
}

UIWidgetComposite::~UIWidgetComposite()
{
	ClearChildren();
}

void UIWidgetComposite::ClearChildren()
{
	FOR_EACH_ARRAY( ChildIter, m_Children, UIWidget* )
	{
		UIWidget* pWidget = ChildIter.GetValue();
		SafeDelete( pWidget );
	}

	m_Children.Clear();
}

void UIWidgetComposite::UpdateRender()
{
	FOR_EACH_ARRAY( ChildIter, m_Children, UIWidget* )
	{
		UIWidget* const pWidget = ChildIter.GetValue();
		pWidget->UpdateRender();
	}
}

void UIWidgetComposite::Render( bool HasFocus )
{
	XTRACE_FUNCTION;

	FOR_EACH_ARRAY( ChildIter, m_Children, UIWidget* )
	{
		UIWidget* const pWidget = ChildIter.GetValue();
		pWidget->Render( HasFocus );
	}
}

void UIWidgetComposite::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	ClearChildren();

	UIWidget::InitializeFromDefinition( DefinitionName );

	MAKEHASH( m_Archetype );
	MAKEHASH( DefinitionName );

	STATICHASH( NumChildren );
	const uint NumChildren = ConfigManager::GetArchetypeInt( sNumChildren, sm_Archetype, 0, sDefinitionName );
	for( uint ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex )
	{
		const SimpleString	ChildDefinitionName	= ConfigManager::GetArchetypeSequenceString( "Child%d", ChildIndex, sm_Archetype, "", sDefinitionName );
		UIWidget* const		pChildWidget		= UIFactory::CreateWidget( ChildDefinitionName, GetOwnerScreen() );
		ASSERT( pChildWidget );
		m_Children.PushBack( pChildWidget );
	}
}

void UIWidgetComposite::GetBounds( SRect& OutBounds )
{
	ASSERT( m_Children.Size() > 0 );
	m_Children[0]->GetBounds( OutBounds );

	FOR_EACH_ARRAY( ChildIter, m_Children, UIWidget* )
	{
		UIWidget* const pWidget = ChildIter.GetValue();

		SRect WidgetBounds;
		pWidget->GetBounds( WidgetBounds );

		OutBounds.m_Left	= Min( WidgetBounds.m_Left,		OutBounds.m_Left );
		OutBounds.m_Top		= Min( WidgetBounds.m_Top,		OutBounds.m_Top );
		OutBounds.m_Right	= Max( WidgetBounds.m_Right,	OutBounds.m_Right );
		OutBounds.m_Bottom	= Max( WidgetBounds.m_Bottom,	OutBounds.m_Bottom );
	}
}

void UIWidgetComposite::Refresh()
{
	UIWidget::Refresh();

	FOR_EACH_ARRAY( ChildIter, m_Children, UIWidget* )
	{
		UIWidget* const pWidget = ChildIter.GetValue();
		pWidget->Refresh();
	}
}