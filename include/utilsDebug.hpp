#ifndef UTILSDEBUG_HPP_18_07_05_15_00_17
#define UTILSDEBUG_HPP_18_07_05_15_00_17 
#include <functional>
#include <fstream>
#include <chrono>

template <typename T>
class ValueLogger {
	public:
		using Outputter = std::function<void(double time, T value, std::ofstream& outFile)>;
		ValueLogger(std::string outFileName, Outputter outputter = [](double time, T value, std::ofstream& outFile){ outFile << time << ": " << value << std::endl;}): _outputter{outputter}, _time{0}
		{
			_outFile.open(outFileName, std::ofstream::out);
		}
		~ValueLogger()
		{
			_outFile.close();
		}
		void log(double timeDelta, T value)
		{
			_time += timeDelta;
			_outputter(_time, value, _outFile);
		}

	private:
		Outputter _outputter;
		double _time;
		std::ofstream _outFile;
};


class LongerRunTimeDetector {
	public:
		LongerRunTimeDetector(std::string name);

		class AutoStopWatch {
			public:
				AutoStopWatch(LongerRunTimeDetector& parent);
				~AutoStopWatch();
			private:
				LongerRunTimeDetector& _parent;
				std::chrono::time_point<std::chrono::system_clock> _startTime;
		};
		AutoStopWatch getAutoStopWatch();
		void addTimeOfRun(unsigned long long us);

	private:
		std::string _name;
		double _averageRunTime;
		unsigned long long _sampleCount;
};
#endif /* UTILSDEBUG_HPP_18_07_05_15_00_17 */
