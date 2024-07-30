#pragma once

#include "Collision.h"

#include <print>

namespace nv {
	template<typename Rep, typename Period>
	struct PixelRate {
		int pixels = 0;
		chrono::duration<Rep, Period> denm;
	};

	template<RenderObject Object, typename XRateRep, typename XRatePeriod, typename YRateRep, typename YRatePeriod>
	class MoveScheduler {
	private:
		std::reference_wrapper<Object> m_obj;

		using XRate = PixelRate<XRateRep, XRatePeriod>;
		using YRate = PixelRate<YRateRep, YRatePeriod>;
		XRate m_xRate;
		YRate m_yRate;

		int m_xDist;
		int m_yDist;
		int m_xDistTravelled = 0;
		int m_yDistTravelled = 0;
		
		chrono::system_clock::time_point m_xStart;
		chrono::system_clock::time_point m_yStart;

		bool m_startedMoving = false;
	public:
		MoveScheduler(Object& obj, XRate xRate, YRate yRate, int xDist, int yDist) 
			: m_obj{ obj }, m_xRate { xRate }, m_yRate{ yRate }, m_xDist{ xDist }, m_yDist{ yDist }
		{
		}

		bool operator()() {
			std::println("Moving\n");
			if (!m_startedMoving) {
				m_xStart = chrono::system_clock::now();
				m_yStart = chrono::system_clock::now();
				m_startedMoving = true;
			}
			auto now = chrono::system_clock::now();
			auto xTimeElapsed = chrono::duration_cast<chrono::duration<XRateRep, XRatePeriod>>(now - m_xStart);
			auto yTimeElapsed = chrono::duration_cast<chrono::duration<YRateRep, YRatePeriod>>(now - m_yStart);

			auto getTravelDist = [&](auto timeElapsed, auto rate, auto& distTravelled, auto dist, auto& timeStart) {
				if (timeElapsed > rate.denm && distTravelled < dist) {
					auto timesMoved = static_cast<int>(timeElapsed / rate.denm);
					auto ret = (rate.pixels * timesMoved);
					distTravelled += (rate.pixels * timesMoved);
					timeStart = now;
					return ret;
				}
				return 0;
			};
			auto dx = getTravelDist(xTimeElapsed, m_xRate, m_xDistTravelled, m_xDist, m_xStart);
			auto dy = getTravelDist(yTimeElapsed, m_yRate, m_yDistTravelled, m_yDist, m_yStart);
			m_obj.get().move(dx, dy);
			return m_xDistTravelled >= m_xDist && m_yDistTravelled >= m_yDist;
		}
	};

	/*template<RenderObject Object, typename XRateRep, typename XRatePeriod, typename YRateRep, typename YRatePeriod>
	MoveScheduler(Object& obj, EventHandler& evtHandler,
		PixelRate<XRateRep, XRatePeriod> xRate,
		PixelRate<YRateRep, YRatePeriod> yRate, int xDist, int yDist) ->
		MoveScheduler<Object, XRateRep, XRatePeriod, YRateRep, YRatePeriod>;*/
}