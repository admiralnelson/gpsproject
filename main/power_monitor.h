#pragma once

class PowerMonitor {
public:

	static PowerMonitor& Get();

	float Voltage();
	float Current();

private:
	PowerMonitor(PowerMonitor const&) = delete;
	void operator=(PowerMonitor const&) = delete;

	PowerMonitor();
};