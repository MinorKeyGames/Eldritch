#ifndef WBCOMPELDDROPSHADOW_H
#define WBCOMPELDDROPSHADOW_H

#include "wbeldritchcomponent.h"
#include "aabb.h"

class Mesh;

class WBCompEldDropShadow : public WBEldritchComponent
{
public:
	WBCompEldDropShadow();
	virtual ~WBCompEldDropShadow();

	DEFINE_WBCOMP( EldDropShadow, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual bool	IsRenderable() { return true; }
	virtual void	Render();

#if BUILD_DEV
	virtual void	DebugRender() const;
#endif

	virtual void	HandleEvent( const WBEvent& Event );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	bool			GetUseMeshCenter() { return m_UseMeshCenter; }

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			Update();
	bool			ShouldBeHidden() const;

	Mesh*	m_Mesh;
	AABB	m_MeshOriginalAABB;
	float	m_VoxelCheckOffsetZ;
	float	m_ShadowFloatZ;
	bool	m_Hidden;
	bool	m_ScriptHidden;
	bool	m_UseMeshCenter;
};

#endif // WBCOMPELDDROPSHADOW_H