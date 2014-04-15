#ifndef UISCREENELDSETRES_H
#define UISCREENELDSETRES_H

#include "uiscreen.h"
#include "map.h"
#include "hashedstring.h"
#include "display.h"

class UIScreenEldSetRes : public UIScreen
{
public:
	UIScreenEldSetRes();
	virtual ~UIScreenEldSetRes();

	DEFINE_UISCREEN_FACTORY( EldSetRes );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Pushed();

	void			SetUICallback( const SUICallback& Callback );

	SDisplayMode	GetRes( const HashedString& Name );

protected:
	Map<HashedString, SDisplayMode>	m_ResMap;
	SUICallback						m_Callback;
};

#endif // UISCREENELDSETRES_H