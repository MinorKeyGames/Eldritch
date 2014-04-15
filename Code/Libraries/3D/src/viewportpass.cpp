#include "core.h"
#include "viewportpass.h"

ViewportPass::ViewportPass()
:	m_ViewportBounds()
,	m_ViewMap()
{
}

ViewportPass::~ViewportPass()
{
}

View* ViewportPass::GetView( View* const pView ) const
{
	Map<View*, View*>::Iterator ViewIterator = m_ViewMap.Search( pView );
	return ViewIterator.IsValid() ? ViewIterator.GetValue() : pView;
}