#ifndef CLOCK_H
#define CLOCK_H

#include "list.h"

#if BUILD_SDL
#include "SDL2/SDL.h"
#endif

#if BUILD_WINDOWS_NO_SDL
#define CLOCK_T	__int64
#elif BUILD_SDL
#define CLOCK_T	Uint64
#endif

// Returns the total running time, current time, or delta time
// for either the game (subject to slow/stop effects) or the
// machine (independent of effects), or the physical clock (the
// integral counters, not in seconds).

// This class is not intended to be a singleton; while the engine
// should primarily use a single clock, everything works fine if
// more than one is used (and the profiler depends on it, because
// it uses its own clock).

class SimpleString;

class Clock
{
public:
	struct MultiplierRequest
	{
		float	m_Duration;		// In machine time; <= 0.0 implies infinite request
		float	m_Multiplier;
	};

	Clock();
	~Clock();

	// Clocks are automatically initialized and shut down on construction/destruction;
	// these just exist to allow reinitialization or deletion of memory when needed.
	void	Initialize();
	void	ShutDown();
	void	Tick( bool GamePaused = false, const bool DoGameTick = true );

	// A bit of a hack; my needs for this class have changed a lot in 6 years.
	void	GameTick( const bool GamePaused, const float DeltaTime );

	float	GetGameCurrentTime() const;
	float	GetGameDeltaTime() const;

	float	GetMachineCurrentTime() const;
	float	GetMachineDeltaTime() const;

	CLOCK_T	GetPhysicalCurrentTime() const;
	CLOCK_T	GetPhysicalDeltaTime() const;

	// Used by profiler, could probably be compiled out for Release
	double	GetResolution() const;

	uint	GetTickCount() const;

	void	Report() const;

	MultiplierRequest*	AddMultiplierRequest( const SimpleString& DefinitionName );
	MultiplierRequest*	AddMultiplierRequest( float Duration, float Multiplier );	// Set Duration to 0.0f to make it infinite
	void				RemoveMultiplierRequest( MultiplierRequest** pMultiplierRequest );
	void				ClearMultiplierRequests();

	// Used to clamp the game delta time to a reasonable maximum so
	// that hitches don't interfere with the sim and cause chaos.
	// Set to zero to disable the limit.
	void	SetGameDeltaTimeMax( float Limit );

	void	UseFixedMachineDeltaTime( float FixedDeltaTime );
	void	UseActualMachineDeltaTime();

private:
	float	CounterToSeconds( const CLOCK_T Counter ) const;
	float	GetMultiplier() const;
	void	TickMultiplierRequests( float DeltaTime );

	// NOTE: It might be a bit redundant, but I should probably precompute and store all
	// the various things this class can return, for two reasons: 1) to prevent recomputing
	// things I don't need to, and 2) so there's no way I could possibly get two different
	// results in a given frame (e.g., by changing the multiplier mid-frame).
	CLOCK_T	m_PhysicalBaseTime;
	CLOCK_T	m_PhysicalCurrentTime;
	CLOCK_T	m_PhysicalDeltaTime;

	float	m_MachineCurrentTime;
	float	m_MachineDeltaTime;

	float	m_GameCurrentTime;
	float	m_GameDeltaTime;

	uint	m_TickCount;

	// Used by profiler, could probably be compiled out for Release
	double	m_Resolution;

	List< MultiplierRequest* >	m_MultiplierRequests;
	float	m_GameDeltaTimeMax;

	// Hack to make FRAPS captures smoother
	bool	m_UseFixedMachineDeltaTime;
	float	m_FixedMachineDeltaTime;
};

#endif // CLOCK_H