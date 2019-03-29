#include "Timer.h"
SecTimer::SecTimer()
{
}
void SecTimer::resetChronoTimer()
{
	startChrono = Clock::now();
}
float SecTimer::getChronoTime()
{
	std::chrono::steady_clock::time_point now = Clock::now();
	//auto timeDiff = std::chrono::duration_cast<std::chrono::nanoseconds>(now - startChrono).count();
	auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(now - startChrono).count();
	return (float)timeDiff;
}
