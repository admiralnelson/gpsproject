#pragma once
#include "NMEAParser.h"
#include "GPSService.h"
#include "memory"
#include "array"
#include <GPSFix.h>


#define BUF_SIZE (1024)

class Gps {
public:
	static Gps& Get();
	std::array<char, BUF_SIZE> Buffer;

	void Update(size_t length);

private:
	bool bSerialIsActive = false;

	void SetupSerial();
	void SetupGps();

	std::unique_ptr<nmea::NMEAParser>nmeaParser;
	std::unique_ptr<nmea::GPSService>gpsService;

	Gps(Gps const&) = delete;
	void operator=(Gps const&) = delete;


	Gps();
};