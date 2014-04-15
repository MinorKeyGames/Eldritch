#include "core.h"
#include "uiscreen-fade.h"
#include "Widgets/uiwidget-image.h"

UIScreenFade::UIScreenFade()
:	m_FadeInterpolator()
,	m_FadeFinishedCallback()
{
}

UIScreenFade::UIScreenFade( const SimpleString& DefinitionName )
:	m_FadeInterpolator()
,	m_FadeFinishedCallback()
{
	InitializeFromDefinition( DefinitionName );
}

UIScreenFade::~UIScreenFade() {}

UIScreen::ETickReturn UIScreenFade::Tick( float DeltaTime, bool HasFocus )
{
	XTRACE_FUNCTION;

	ETickReturn RetVal = UIScreen::Tick( DeltaTime, HasFocus );

	static const HashedString sFadeImage( "FadeImage" );
	UIWidgetImage* pFadeImage = GetWidget<UIWidgetImage>( sFadeImage );

	if( pFadeImage )
	{
		const bool WasFinished = m_FadeInterpolator.IsFinished();
		m_FadeInterpolator.Tick( DeltaTime );
		const bool IsFinished = m_FadeInterpolator.IsFinished();

		pFadeImage->m_Color = m_FadeInterpolator.GetValue();

		if( !WasFinished && IsFinished )
		{
			if( m_FadeFinishedCallback.m_Callback )
			{
				m_FadeFinishedCallback.m_Callback( this, m_FadeFinishedCallback.m_Void );
			}
		}
	}

	return RetVal;
}

void UIScreenFade::Fade( const Vector4& StartColor, const Vector4& EndColor, const float Duration )
{
	m_FadeInterpolator.Reset( Interpolator<Vector4>::EIT_Linear, StartColor, EndColor, Duration );

	static const HashedString sFadeImage( "FadeImage" );
	UIWidgetImage* pFadeImage = GetWidget<UIWidgetImage>( sFadeImage );

	if( pFadeImage )
	{
		pFadeImage->m_Color = m_FadeInterpolator.GetValue();
	}
}

void UIScreenFade::SetFadeCallback( const SUICallback& FadeCallback )
{
	m_FadeFinishedCallback = FadeCallback;
}