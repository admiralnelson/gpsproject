#pragma once
#include "string"
class Logger {
public:
	static void Start();
	static void Stop();
	static void Clear();
	static std::string GetPowerLogFilename();
	static std::string GetLdrLogFilename();

};