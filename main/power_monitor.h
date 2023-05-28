#pragma once
#include "pins.h"
#include "common.h"
#include "memory"
#include "cjsoncpp.h"
#include "ina219.h"


class PowerMonitor {
public:
	struct CalibrationData : JsonSerialisable {
		float maxCurrent = 0;
		float shuntResistance = 0;

		virtual cjsonpp::JSONObject ToJsonObject() const override;
		virtual bool FromJsonObject(const cjsonpp::JSONObject& Object) noexcept override;
	};
public:

	static PowerMonitor& Get();
	bool CalibrateDevice(const CalibrationData& values);
	float Voltage();
	float Current();
	float Power();
	CalibrationData GetCalibration();
	void ResetCalibration();

private:
	void InitIna219Device();

private:
	CalibrationData calibrationValues;
	ina219_handle_t ina219Device;
	bool bInited = false;

	PowerMonitor(PowerMonitor const&) = delete;
	void operator=(PowerMonitor const&) = delete;
	PowerMonitor();


};
