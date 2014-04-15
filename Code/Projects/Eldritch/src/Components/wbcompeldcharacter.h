#ifndef WBCOMPELDCHARACTER_H
#define WBCOMPELDCHARACTER_H

#include "wbeldritchcomponent.h"
#include "array.h"
#include "simplestring.h"

// This manages character customization and stuff.

class WBCompEldCharacter : public WBEldritchComponent
{
public:
	WBCompEldCharacter();
	virtual ~WBCompEldCharacter();

	DEFINE_WBCOMP( EldCharacter, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	SimpleString	GetCurrentVoiceSet() const { return GetCurrentHeadOption().m_VoiceSet; }

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	struct SHeadOption
	{
		SimpleString	m_Mesh;
		SimpleString	m_Texture;
		SimpleString	m_VoiceSet;
	};

	struct SBodyOption
	{
		SimpleString	m_Mesh;
		SimpleString	m_Texture;

		SimpleString	m_LeftHandMesh;
		SimpleString	m_LeftHandTexture;

		SimpleString	m_RightHandMesh;
		SimpleString	m_RightHandTexture;
	};

	void				PushCharacterOptions() const;

	void				ChangeHead( int IndexStep );
	void				ChangeBody( int IndexStep );

	const SHeadOption&	GetCurrentHeadOption() const { return m_HeadOptions[ m_CurrentHeadIndex ]; }
	const SBodyOption&	GetCurrentBodyOption() const { return m_BodyOptions[ m_CurrentBodyIndex ]; }

	void				PushPersistence() const;
	void				PullPersistence();

	Array<SHeadOption>	m_HeadOptions;
	Array<SBodyOption>	m_BodyOptions;

	uint				m_CurrentHeadIndex;
	uint				m_CurrentBodyIndex;
};

#endif // WBCOMPELDCHARACTER_H