#ifndef TIMER_H
#define TIMER_H

class Timer
{
public:
	Timer();
	~Timer();

	void	Start( float Time );
	void	Stop();
	void	AddTime( float Time );
	bool	Tick( float DeltaTime );	// Returns true on the tick that the timer expires
	bool	HasExpired();				// Simply means there's no time left on it, doesn't imply that the timer was ever running
	float	GetTimeRemaining();

private:
	float	m_TimeRemaining;
};

#endif // TIMER_H