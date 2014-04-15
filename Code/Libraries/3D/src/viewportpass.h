#ifndef VIEWPORTPASS_H
#define VIEWPORTPASS_H

#include "map.h"
#include "3d.h"

class View;

class ViewportPass
{
public:
	ViewportPass();
	~ViewportPass();

	View*			GetView( View* const pView ) const;
	const SRect&	GetBounds() const { return m_ViewportBounds; }

	void			SetBounds( const SRect Bounds ) { m_ViewportBounds = Bounds; }
	void			SetViewOverride( View* const pOriginalView, View* const pNewView ) { m_ViewMap[ pOriginalView ] = pNewView; }

private:
	SRect				m_ViewportBounds;
	Map<View*, View*>	m_ViewMap;
};

#endif // VIEWPORTPASS_H