#pragma once
#include "pins.h"
#include "common.h"
#include "memory"


class PowerMonitor {
public:

	static PowerMonitor& Get();

	float Voltage();
	float Current();
	float Power();

private:
	void InitIna219Device();
	void Update();

private:
	float current = 0;
	float voltage = 0;
	float power = 0;

	PowerMonitor(PowerMonitor const&) = delete;
	void operator=(PowerMonitor const&) = delete;
	PowerMonitor();

};
