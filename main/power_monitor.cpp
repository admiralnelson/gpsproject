#include "power_monitor.h"

PowerMonitor::PowerMonitor()
{
}

PowerMonitor& PowerMonitor::Get()
{
	static PowerMonitor instance;
	return instance;
}

float PowerMonitor::Voltage()
{
	return 12.0f;
}

