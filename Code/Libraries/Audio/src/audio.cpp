#include "core.h"
#include "audio.h"
#include "fmodaudiosystem.h"

IAudioSystem* CreateFMODAudioSystem()
{
	return new FMODAudioSystem;
}