#pragma once
#include "memory"
#include "array"

#define BUF_SIZE (1024)

class Gps {
public:
	static Gps& Get();
	std::array<char, BUF_SIZE> Buffer;

private:
	bool bSerialIsActive = false;

	void SetupSerial();
	void SetupGps();

	Gps(Gps const&) = delete;
	void operator=(Gps const&) = delete;


	Gps();
};