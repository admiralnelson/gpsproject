#pragma once
#include "NMEAParser.h"
#include "GPSService.h"
#include "memory"
#include "array"
#include <GPSFix.h>
#include <queue>

#define BUF_SIZE (1024)

class Gps {
public:
	static Gps& Get();
	std::array<char, BUF_SIZE> Buffer;

	void Update(size_t length);
	double GetLon();
	double GetLat();
	double GetAltitute();
	long GetTime();
	bool IsFix();
	bool IsDebug();
	void ShowDebug(bool bIsShowing);

private:
	bool bSerialIsActive = false;
	bool bShowDebug = false;

	void SetupSerial();
	void SetupGps();

	std::unique_ptr<class nmea::NMEAParser>nmeaParser;
	std::unique_ptr<class nmea::GPSService>gpsService;

	std::unique_ptr<class MovingAverageFilter> longitudeFilter;
	std::unique_ptr<class MovingAverageFilter> latitudeFilter;
	std::unique_ptr<class MovingAverageFilter> altitudeFilter;

	Gps(Gps const&) = delete;
	void operator=(Gps const&) = delete;


	Gps();
};
