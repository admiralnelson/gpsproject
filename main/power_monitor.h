#pragma once

class PowerMonitor {
public:

	static PowerMonitor& Get();

	float Voltage();

private:
	PowerMonitor(PowerMonitor const&) = delete;
	void operator=(PowerMonitor const&) = delete;

	PowerMonitor();
};