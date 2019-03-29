#include "HighResTimer.h"
HighResTimer::HighResTimer()
{
}
void HighResTimer::resetChronoTimer()
{
	startChrono = Clock::now();
}
float HighResTimer::getChronoTime()
{
	std::chrono::steady_clock::time_point now = Clock::now();
	//auto timeDiff = std::chrono::duration_cast<std::chrono::nanoseconds>(now - startChrono).count();
	//auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(now - startChrono).count();
	auto timeDiff = std::chrono::duration_cast<std::chrono::microseconds>(now - startChrono).count();
	return (float)timeDiff;
}
