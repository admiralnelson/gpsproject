#include "solartrackingthread.h"
#include "esp_log.h"
#include "common.h"
#include "gps.h"
#include <cmath>
#include "motor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SOLAR_OPEN_LOOP_TASK_STACK_SIZE (2048*5)
#define SOLAR_CLOSED_LOOP_TASK_STACK_SIZE (2048*5)


const char* SOLAR_TRACKER_THREAD_TAG = "solartracker open";
const char* SOLAR_TRACKER_THREAD_CLOSED_TAG = "solartracker closed";


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
	this->bIsRunning = false;
}

bool SolarTrackingOpenLoop::Start()
{
	if (!this->IsRunning())
	{
		this->bIsRunning = true;
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
		this->bIsRunning = false;
	}
	ESP_LOGI(SOLAR_TRACKER_THREAD_TAG, "stopping solar tracking open loop thread");
	return true;
}

bool SolarTrackingOpenLoop::IsRunning()
{
	return this->bIsRunning;
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

static void ClosedLoopThread(void* thisPtr)
{
	SolarTrackingClosedLoop* solarTrackingClosedLoopMgr = static_cast<SolarTrackingClosedLoop*>(thisPtr);
	ESP_LOGI(SOLAR_TRACKER_THREAD_CLOSED_TAG, "starting closedloopthread");
	while (true)
	{
		if (solarTrackingClosedLoopMgr->IsRunning())
		{
			solarTrackingClosedLoopMgr->Update();
			Delay(1000);
			continue;
		}
		Delay(1000);
	}

}

SolarTrackingClosedLoop& SolarTrackingClosedLoop::Get()
{
	static SolarTrackingClosedLoop instance;
	return instance;
}

bool SolarTrackingClosedLoop::Start()
{
	if (!this->IsRunning())
	{
		this->bIsRunning = true;
		ESP_LOGI(SOLAR_TRACKER_THREAD_CLOSED_TAG, "starting solar tracking closed loop thread");
		xTaskCreate(ClosedLoopThread, "solar_tracking_closed_loop", SOLAR_CLOSED_LOOP_TASK_STACK_SIZE, this, 15, NULL);
	}
	else
	{
		ESP_LOGI(SOLAR_TRACKER_THREAD_CLOSED_TAG, "it's already started");
	}
	return true;
}

bool SolarTrackingClosedLoop::Stop()
{
	if (this->IsRunning())
	{
		this->bIsRunning = false;
	}
	ESP_LOGI(SOLAR_TRACKER_THREAD_CLOSED_TAG, "stopping solar tracking closed loop thread");
	return true;
}

bool SolarTrackingClosedLoop::IsRunning()
{
	return this->bIsRunning;
}

void SolarTrackingClosedLoop::Update()
{
	this->UpdateLdr();
	this->UpdateHorizontal();
	this->UpdateVertical();
}

bool SolarTrackingClosedLoop::IsBetweenTolerance(float value, float min, float max)
{
	return value >= min && value <= max;
}

float SolarTrackingClosedLoop::Map(int value, int minMilliVolt, int maxMilliVolt, float min, float max)
{
	return map((float)value, (float)minMilliVolt, (float)maxMilliVolt, min, max);
}

void SolarTrackingClosedLoop::UpdateLdr()
{
	const int Ldr1 = PinFunctions::ReadAnalog2(LDR_1_PIN);
	const int Ldr2 = PinFunctions::ReadAnalog2(LDR_2_PIN);
	const int Ldr3 = PinFunctions::ReadAnalog1(LDR_3_PIN);
	const int Ldr4 = PinFunctions::ReadAnalog1(LDR_4_PIN);

	this->ldrArray.N = this->Map(Ldr2, ADC2_RANGE.Min, ADC2_RANGE.Max, 0.0F, 1.0F);
	this->ldrArray.W = this->Map(Ldr1, ADC2_RANGE.Min, ADC2_RANGE.Max, 0.0F, 1.0F);
	this->ldrArray.E = this->Map(Ldr3, ADC1_RANGE.Min, ADC1_RANGE.Max, 0.0F, 1.0F);
	this->ldrArray.S = this->Map(Ldr4, ADC1_RANGE.Min, ADC1_RANGE.Max, 0.0F, 1.0F);

	ESP_LOGI(SOLAR_TRACKER_THREAD_CLOSED_TAG, "updating sensors ldr1 W %d mapped %f", Ldr1, this->ldrArray.W);
	ESP_LOGI(SOLAR_TRACKER_THREAD_CLOSED_TAG, "updating sensors ldr2 S %d mapped %f", Ldr2, this->ldrArray.S);
	ESP_LOGI(SOLAR_TRACKER_THREAD_CLOSED_TAG, "updating sensors ldr3 E %d mapped %f", Ldr3, this->ldrArray.E);
	ESP_LOGI(SOLAR_TRACKER_THREAD_CLOSED_TAG, "updating sensors ldr4 N %d mapped %f", Ldr4, this->ldrArray.N);
}

void SolarTrackingClosedLoop::UpdateHorizontal()
{
	ESP_LOGI(SOLAR_TRACKER_THREAD_CLOSED_TAG, "difference W - E = %f", std::abs(this->ldrArray.W - this->ldrArray.E));
	if (this->IsBetweenTolerance(std::abs(this->ldrArray.W - this->ldrArray.E), this->treshold.Horizontal.Min, this->treshold.Horizontal.Max))
	{
		if (this->ldrArray.W > this->ldrArray.E)
		{
			MotorController::Get().StepAMinusOneDeg();
		}
		if (this->ldrArray.W < this->ldrArray.E)
		{
			MotorController::Get().StepAOneDeg();
		}
	}

}

void SolarTrackingClosedLoop::UpdateVertical()
{
	ESP_LOGI(SOLAR_TRACKER_THREAD_CLOSED_TAG, "difference N - S = %f", std::abs(this->ldrArray.N - this->ldrArray.S));
	if (this->IsBetweenTolerance(std::abs(this->ldrArray.N - this->ldrArray.S), this->treshold.Vertical.Min, this->treshold.Vertical.Max))
	{
		if (this->ldrArray.N > this->ldrArray.S)
		{
			MotorController::Get().StepBOneDeg();
		}
		if (this->ldrArray.N < this->ldrArray.S)
		{
			MotorController::Get().StepBMinusOneDeg();
		}
	}
}

SolarTrackingClosedLoop::SolarTrackingClosedLoop()
{
	this->bIsRunning = false;
	
}
