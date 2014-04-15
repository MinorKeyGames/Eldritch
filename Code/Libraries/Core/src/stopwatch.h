#ifndef STOPWATCH_H
#define STOPWATCH_H

#include "clock.h"

class Stopwatch
{
public:
	Stopwatch();

	float	GetTime();
	void	TogglePause();
	void	Reset();

private:
	Clock						m_Clock;
	Clock::MultiplierRequest*	m_PauseRequest;
};

#endif // STOPWATCH_H