#include "logger.h"
#include "rolling_log.h"
#include "mutex"
#include "condition_variable"
#include "motor.h"
#include "solartrackingthread.h"
#include "gps.h"
#include "power_monitor.h"
#include "common.h"
#include "esp_log.h"

static const char* POWER = "power.txt";
static const char* POWER_HEADER = "Time;Current;Voltage;Power";

static const char* LDR = "ldr.txt";
static const char* LDR_HEADER = "Time;LDRN;LDRW;LDRS;LDRE";

static std::mutex mtx;
static std::condition_variable cv;
static bool run = false;

void ThreadUpdate()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [] {return run; });
		ESP_LOGI("logger", "logging");
		Delay(1000 * 60 * 5);
		std::string out;
		out = GetCurrentSystemDateTimeAsString() + ";" + std::to_string(PowerMonitor::Get().Current()) + ";" + std::to_string(PowerMonitor::Get().Voltage()) + ";" + std::to_string(PowerMonitor::Get().Power());
		RollingLog::Get().WriteLog(POWER, out);
		ESP_LOGI("logger", "%s", out.c_str());

		if (SolarTrackingClosedLoop::Get().IsRunning()) 
		{
			std::string out2;
			SolarTrackingClosedLoop::LdrArray LdrArrays = SolarTrackingClosedLoop::Get().GetArrayLdr();
			out2 = GetCurrentSystemDateTimeAsString() + ";" + std::to_string(LdrArrays.N) + ";" + std::to_string(LdrArrays.W) + ";" + std::to_string(LdrArrays.S) + ";" + std::to_string(LdrArrays.E);
			RollingLog::Get().WriteLog(LDR, out2);
			ESP_LOGI("logger", "%s", out2.c_str());
		}
	}
}

void Logger::Start()
{
	RollingLog::Get().Configure(POWER, POWER_HEADER, 10000);
	RollingLog::Get().Configure(LDR, LDR_HEADER, 10000);

	std::thread thread(ThreadUpdate);

	thread.detach();
	
	std::lock_guard<std::mutex> lock(mtx); 
	run = true; 
	cv.notify_one(); 
	ESP_LOGI("logger", "starting");
}

void Logger::Stop()
{
	std::lock_guard<std::mutex> lock(mtx);
	run = false;
}

void Logger::Clear()
{
	RollingLog::Get().ClearLog(POWER);
	RollingLog::Get().ClearLog(LDR);
}

std::string Logger::GetPowerLogFilename()
{
	bool found;
	auto metadata = RollingLog::Get().GetMetadata(POWER, found);
	if (!found) return "meta not found";
	return metadata.Path;
}

std::string Logger::GetLdrLogFilename()
{
	bool found;
	auto metadata = RollingLog::Get().GetMetadata(LDR, found);
	if (!found) return "meta not found";
	return metadata.Path;
}
