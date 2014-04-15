#ifndef WBCOMPELDTHINKER_H
#define WBCOMPELDTHINKER_H

#include "wbeldritchcomponent.h"

class WBCompEldThinker : public WBEldritchComponent
{
public:
	WBCompEldThinker();
	virtual ~WBCompEldThinker();

	virtual void	HandleEvent( const WBEvent& Event );
};

#endif // WBCOMPELDTHINKER_H