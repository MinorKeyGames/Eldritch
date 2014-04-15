#ifndef WBPARAMEVALUATOR_H
#define WBPARAMEVALUATOR_H

#include "simplestring.h"
#include "vector.h"
#include "wbentityref.h"
#include "wbevent.h"

class WBPE;

class WBParamEvaluator
{
public:
	WBParamEvaluator();
	~WBParamEvaluator();

	struct SPEContext
	{
		SPEContext();

		WBEntity*	m_Entity;
	};

	enum EParamType
	{
		EPT_None,
		EPT_Bool,
		EPT_Int,
		EPT_Float,
		EPT_String,
		EPT_Entity,
		EPT_Vector,
	};

	void			InitializeFromDefinition( const SimpleString& DefinitionName );
	void			Evaluate( const SPEContext& Context );

	bool			GetBool() const { return m_EvaluatedParam.GetBool(); }
	int				GetInt() const { return m_EvaluatedParam.GetInt(); }
	float			GetFloat() const { return m_EvaluatedParam.GetFloat(); }
	SimpleString	GetString() const { return m_EvaluatedParam.GetString(); }
	WBEntity*		GetEntity() const { return m_EvaluatedParam.GetEntity(); }
	Vector			GetVector() const { return m_EvaluatedParam.GetVector(); }

	EParamType		GetType() const { return m_EvaluatedParam.m_Type; };

	bool			HasRoot() const { return m_RootEvaluator != NULL; }

	struct SEvaluatedParam
	{
		SEvaluatedParam();

		EParamType		m_Type;

		// WBTODO: If I were concerned about memory, I could union these values (like WBEvents do)
		bool			m_Bool;
		int				m_Int;
		float			m_Float;
		SimpleString	m_String;
		WBEntityRef		m_Entity;
		Vector			m_Vector;

		bool			GetBool() const;
		int				GetInt() const;
		float			GetFloat() const;
		SimpleString	GetString() const;
		WBEntity*		GetEntity() const;
		Vector			GetVector() const;

		// Urgh.
		void			Set( const WBEvent::SParameter* const pParameter );
	};

private:
	WBPE*			m_RootEvaluator;
	SEvaluatedParam	m_EvaluatedParam;
};

#endif // WBPARAMEVALUATOR_H