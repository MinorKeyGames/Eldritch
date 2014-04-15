#ifndef WBACTIONSTACK_H
#define WBACTIONSTACK_H

// Stack frame for the currently executed WBAction.
// To simplify a bunch of code, this just reuses WBEvents as the stack frame.

class WBEvent;

namespace WBActionStack
{
	void Initialize();
	void ShutDown();

	void Push( const WBEvent& Event );
	void Pop();

	const WBEvent& Top();
}

#endif // WBACTIONSTACK_H