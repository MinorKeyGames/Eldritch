#include "core.h"
#include "uiscreen-eldcredits.h"
#include "Widgets/uiwidget-text.h"
#include "configmanager.h"
#include "mesh.h"

UIScreenEldCredits::UIScreenEldCredits()
{
}

UIScreenEldCredits::~UIScreenEldCredits()
{
}

/*virtual*/ UIScreen::ETickReturn UIScreenEldCredits::Tick( float DeltaTime, bool HasFocus )
{
	XTRACE_FUNCTION;

	STATIC_HASHED_STRING( CreditsText );
	UIWidgetText* const pCreditsText = GetWidget<UIWidgetText>( sCreditsText );
	ASSERT( pCreditsText );

	const float Bottom = pCreditsText->m_TopLeft.y + pCreditsText->m_Mesh->m_AABB.m_Max.z - pCreditsText->m_Mesh->m_AABB.m_Min.z;
	if( Bottom < 0.0f )
	{
		STATICHASH( DisplayHeight );
		const float DisplayHeight	= ConfigManager::GetFloat( sDisplayHeight );
		const float ParentHeight	= pCreditsText->m_OriginParent ? pCreditsText->m_OriginParent->GetHeight() : DisplayHeight;
		pCreditsText->m_TopLeft.y	= ParentHeight;

		pCreditsText->UpdateRenderPosition();
	}

	return UIScreen::Tick( DeltaTime, HasFocus );
}

/*virtual*/ void UIScreenEldCredits::Pushed()
{
	UIScreen::Pushed();

	STATIC_HASHED_STRING( CreditsText );
	UIWidgetText* const pCreditsText = GetWidget<UIWidgetText>( sCreditsText );
	ASSERT( pCreditsText );

	STATICHASH( DisplayHeight );
	const float DisplayHeight	= ConfigManager::GetFloat( sDisplayHeight );
	const float ParentHeight	= pCreditsText->m_OriginParent ? pCreditsText->m_OriginParent->GetHeight() : DisplayHeight;
	pCreditsText->m_TopLeft.y	= ParentHeight;

	pCreditsText->UpdateRenderPosition();
}