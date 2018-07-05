#include <iostream>
#include <cassert>
#include "utilsDebug.hpp"

LongerRunTimeDetector::LongerRunTimeDetector(std::string name): _name{name}, _averageRunTime{0}, _sampleCount{0}
{
}

LongerRunTimeDetector::AutoStopWatch::AutoStopWatch(LongerRunTimeDetector& parent):
	_parent{parent}, _startTime{std::chrono::system_clock::now()}
{
}

LongerRunTimeDetector::AutoStopWatch::~AutoStopWatch()
{
	_parent.addTimeOfRun(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now()-_startTime).count());
}

LongerRunTimeDetector::AutoStopWatch LongerRunTimeDetector::getAutoStopWatch()
{
	return AutoStopWatch(*this);
}

void LongerRunTimeDetector::addTimeOfRun(unsigned long long us)
{
	if(_sampleCount > 100) {
		assert(_sampleCount > 0);
		double perc = double(us)/_averageRunTime;
		if(perc > 1.5)
			std::cout << "Longer run time detected (" << _name << "): " << us << " us, " << perc*100 << "\% of avg (average is " << _averageRunTime << " us, " << _sampleCount << " samples)\n";
	}
	_averageRunTime = (_sampleCount*_averageRunTime + (double)us)/(_sampleCount+1);
	++_sampleCount;
}
