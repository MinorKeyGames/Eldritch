#ifndef WBCOMPRODINKNOWLEDGE_H
#define WBCOMPRODINKNOWLEDGE_H

#include "wbcomponent.h"
#include "wbevent.h"

class WBParamEvaluator;

class WBCompRodinKnowledge : public WBComponent
{
public:
	typedef WBEvent TKnowledge;
	typedef Map<WBEntityRef, TKnowledge> TKnowledgeMap;

	WBCompRodinKnowledge();
	virtual ~WBCompRodinKnowledge();

	DEFINE_WBCOMP( RodinKnowledge, WBComponent );

	virtual void			Tick( float DeltaTime );

	virtual uint			GetSerializationSize();
	virtual void			Save( const IDataStream& Stream );
	virtual void			Load( const IDataStream& Stream );

	const TKnowledge*		GetKnowledge( const WBEntityRef& Entity ) const;
	TKnowledge*				GetKnowledge( const WBEntityRef& Entity );

	// Primary interface for sensors
	TKnowledge&				UpdateEntity( const WBEntityRef& Entity, const float UpdateTime = 0.0f, const float ExpireTimeBonus = 0.0f );

	// Primary interface for thinkers
	const TKnowledgeMap&	GetKnowledgeMap() const { return m_KnowledgeMap; }

	// Helper functions
	bool					GetLastKnownLocationFor( const WBEntity* const pEntity, Vector& OutLocation ) const;
	bool					IsCurrentlyVisible( const WBEntity* const pEntity ) const;

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	// TODO: It's worth some consideration as to whether it's a good idea
	// to strongly associate knowledge with an entity. Doing so precludes
	// the possibility of false information, which has interesting connotations.
	TKnowledgeMap	m_KnowledgeMap;		// Transient; map from entity to project-specific knowledge about it
	float			m_ExpireTime;		// Config
	float			m_StaleSeenTime;	// Config
};

#endif // WBCOMPRODINKNOWLEDGE_H