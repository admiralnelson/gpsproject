#include "solartrackingthread.h"
#include "esp_log.h"
#include "common.h"
#include "gps.h"
#include <cmath>
#include "motor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SOLAR_OPEN_LOOP_TASK_STACK_SIZE (2048*5)

const char* SOLAR_TRACKER_THREAD_TAG = "solartracker";

static void OpenLoopThread(void* thisPtr)
{
	SolarTrackingOpenLoop* solarTrackingOpenLoopMgr = static_cast<SolarTrackingOpenLoop*>(thisPtr);
	ESP_LOGI(SOLAR_TRACKER_THREAD_TAG, "starting openloopthread");
	while (true)
	{
		if (solarTrackingOpenLoopMgr->IsRunning())
		{
			solarTrackingOpenLoopMgr->Update();
			Delay(1000);
			continue;
		}
		Delay(1000);
	}

}


SolarTrackingOpenLoop& SolarTrackingOpenLoop::Get()
{
	static SolarTrackingOpenLoop instance;
	return instance;
}

SolarTrackingOpenLoop::SolarTrackingOpenLoop()
{
	this->blsRunning = false;
}

bool SolarTrackingOpenLoop::Start()
{
	if (!this->IsRunning())
	{
		this->blsRunning = true;
		ESP_LOGI(SOLAR_TRACKER_THREAD_TAG, "starting solar tracking open loop thread");
		xTaskCreate(OpenLoopThread, "solar_tracking_open_loop", SOLAR_OPEN_LOOP_TASK_STACK_SIZE, this, 10, NULL);
	}
	else
	{
		ESP_LOGI(SOLAR_TRACKER_THREAD_TAG, "it's already started");
	}
	return true;
}

bool SolarTrackingOpenLoop::Stop()
{
	if (this->IsRunning())
	{
		this->blsRunning = false;
	}
	ESP_LOGI(SOLAR_TRACKER_THREAD_TAG, "stopping solar tracking open loop thread");
	return true;
}

bool SolarTrackingOpenLoop::IsRunning()
{
	return this->blsRunning;
}

void SolarTrackingOpenLoop::Update()
{
	if (!Gps::Get().IsFix())
	{
		return;
	}

	DateTime date = GetCurrentSystemDateTimeStruct();

	//if (!IsBetween(date.hour, 9, 16))
	//{
	//	ESP_LOGW(SOLAR_TRACKER_THREAD_TAG, "time is not between 9 and 16. current time %s", GetCurrentSystemDateTimeAsString().c_str());
	//	MotorController::Get().SetAToDeg(0);
	//	MotorController::Get().SetBToDeg(0);
	//	return;
	//}

	double longitude = Gps::Get().GetLon();
	double latitude  = Gps::Get().GetLat();
	double altitute  = Gps::Get().GetAltitute();

	double azimuth = 0;	
	double elevation = 0;

	time_t utcTime = time(NULL) - 7 * 60 * 60;

	SolarAzEl(utcTime, latitude, longitude, altitute, &azimuth, &elevation);

	azimuth = rad2deg(sin(deg2rad(azimuth)));

	ESP_LOGI(SOLAR_TRACKER_THREAD_TAG, "current longitute %f latitude %f altitude %f", longitude, latitude, altitute);
	ESP_LOGI(SOLAR_TRACKER_THREAD_TAG, "moving the motor azimuth %f elevation %f", azimuth, elevation);

	MotorController::Get().SetAToDeg((int)ceil(azimuth));
	MotorController::Get().SetBToDeg((int)ceil(elevation));
}


