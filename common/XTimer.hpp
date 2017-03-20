#pragma once
#include <windows.h>
#include <map>
#include <string>
#include <vector>

namespace XR
{
	class Timer
	{
	public:
		static Timer& getTimer()
		{
			if (!instance)
				instance = new Timer();
			return *instance;
		}

		void setClock(const std::string& name)
		{
			timeStamp[name] = getNow();
		}

		size_t setClock()
		{
			timeStamp2.push_back(getNow());
			return timeStamp2.size() - 1;
		}

		float milliseconds(int id) const
		{
			return _evalms(getNow(), timeStamp2[id]);
		}

		float milliseconds(const std::string& name) const
		{
			return _evalms(getNow(), timeStamp.at(name));
		}

		float millisecondsAndReset(int id)
		{
			auto now = getNow();
			float res = _evalms(now, timeStamp2[id]);
			timeStamp2[id] = now;
			return res;
		}

		float millisecondsAndReset(const std::string& name)
		{
			auto now = getNow();
			float res = _evalms(now, timeStamp.at(name));
			timeStamp[name] = now;
			return res;
		}

	private:
		Timer() { QueryPerformanceFrequency(&freq); }
		float _evalms(const LARGE_INTEGER& a, const LARGE_INTEGER& b) const
		{
			return (a.QuadPart - b.QuadPart) * 1000.0f / freq.QuadPart;
		}

		static Timer *instance;

		LARGE_INTEGER getNow() const {
			LARGE_INTEGER now;
			QueryPerformanceCounter(&now);
			return now;
		}

		std::map<std::string, LARGE_INTEGER> timeStamp;
		std::vector<LARGE_INTEGER> timeStamp2;
		LARGE_INTEGER freq;
	};
}

#ifdef XTIMER_INSTANCE
XR::Timer * XR::Timer::instance;
#endif

#define XTIMER_OBJ (XR::Timer::getTimer())

// deprecated
#define XTIMER_HELPER(func) (XR::Timer::getTimer().##func##)