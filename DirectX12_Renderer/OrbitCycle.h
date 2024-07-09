#pragma once

#include "DirectionalLight.h"
#include <chrono>

using namespace std::chrono;

static const double DEG_PER_MILLI = 360.0 / 86400000.0; // the number of degrees per millisecond, assuming you rotate 360 degrees in 24 hours.

class OrbitCycle
{
public:
	OrbitCycle(UINT period);
	~OrbitCycle();

	void Update();

	LightSource GetLight() { return m_Sun.GetLight(); }

private:
	UINT						m_period;	// the number of game milliseconds that each real time millisecond should count as.
	DirectionalLight			m_Sun;		// light source representing the sun. 
	DirectionalLight			m_Moon;	// light source representing the moon.
	time_point<system_clock>	m_Last;		// the last point in time that we updated our cycle.
};