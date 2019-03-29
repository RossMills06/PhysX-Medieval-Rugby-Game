#pragma once
#ifndef aTimerFILE
#define aTimerFILE
#include <iostream>
#include <chrono>
//Written by Olivier Szymanezyk oszymanezyk@lincoln.ac.uk
class HighResTimer
{
private:
	typedef std::chrono::high_resolution_clock Clock;
	std::chrono::steady_clock::time_point startChrono;

public:
	HighResTimer();
	void resetChronoTimer();
	float getChronoTime();
};
#endif