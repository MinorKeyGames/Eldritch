#ifndef INPUTCONTEXT_H
#define INPUTCONTEXT_H

#include "map.h"
#include "hashedstring.h"

class SimpleString;
class InputSystem;

class InputContext
{
public:
	InputContext();
	~InputContext();

	void	Initialize( const SimpleString& DefinitionName, InputSystem* const pInputSystem );

	bool			Suppresses() const { return m_SuppressUnredirectedInputs; }
	bool			HasRedirect( const HashedString& Input ) const;
	HashedString	GetRedirect( const HashedString& Input ) const;

private:
	bool							m_SuppressUnredirectedInputs;	// If false, inputs are passed through to next context in stack
	Map<HashedString, HashedString>	m_InputRedirects;
};

#endif // INPUTCONTEXT_H