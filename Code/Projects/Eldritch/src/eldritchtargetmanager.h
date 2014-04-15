#ifndef ELDRITCHTARGETMANAGER_H
#define ELDRITCHTARGETMANAGER_H

class IRenderer;
class IRenderTarget;

class EldritchTargetManager
{
public:
	EldritchTargetManager( IRenderer* const pRenderer );
	~EldritchTargetManager();

	void	CreateTargets( const uint DisplayWidth, const uint DisplayHeight );
	void	ReleaseTargets();

	IRenderTarget*	GetOriginalRenderTarget() const { return m_OriginalRenderTarget; }
	IRenderTarget*	GetPrimaryRenderTarget() const { return m_PrimaryRenderTarget; }
	IRenderTarget*	GetMirrorRenderTarget() const { return m_MirrorRenderTarget; }
	IRenderTarget*	GetMinimapRenderTarget() const { return m_MinimapRenderTarget; }

private:
	IRenderer*		m_Renderer;
	IRenderTarget*	m_OriginalRenderTarget;	// What is presented to the screen
	IRenderTarget*	m_PrimaryRenderTarget;	// What is rendered onto
	IRenderTarget*	m_MirrorRenderTarget;	// The character creation mirror image
	IRenderTarget*	m_MinimapRenderTarget;	// Target for minimap UI texture
};

#endif // ELDRITCHTARGETMANAGER_H