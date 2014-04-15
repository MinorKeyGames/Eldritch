#ifndef TARGETMANAGER_H
#define TARGETMANAGER_H

class IRenderer;
class IRenderTarget;

class TargetManager
{
public:
	TargetManager( IRenderer* pRenderer, uint Width, uint Height );
	~TargetManager();

	void	CreateTargets( uint Width, uint Height );
	void	ReleaseTargets();

	void	GetBloomDimensions( uint& Width, uint& Height );

	IRenderer*			m_Renderer;
	IRenderTarget*		m_OriginalRenderTarget;
	IRenderTarget*		m_DepthRenderTarget;
	IRenderTarget*		m_DepthAlphaRenderTarget;
	IRenderTarget*		m_PrimaryRenderTarget;
	IRenderTarget*		m_BloomSmallTarget1;
	IRenderTarget*		m_BloomSmallTarget2;
	uint				m_BloomWidth;
	uint				m_BloomHeight;
};

#endif // TARGETMANAGER_H