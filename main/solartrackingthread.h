#include "memory"
#include "common.h"
#include "sunpath.h"
#include "pins.h"

class SolarTrackingOpenLoop
{
public:
	static SolarTrackingOpenLoop& Get();
	bool Start();
	bool Stop();
	bool IsRunning();

	void Update();

private:
	bool bIsRunning = false;

	SolarTrackingOpenLoop();
	SolarTrackingOpenLoop(SolarTrackingOpenLoop const&) = delete;

};

const AdcChannel2 LDR_1_PIN = ADC2_CHANNEL_0;
const AdcChannel2 LDR_2_PIN = ADC2_CHANNEL_3;
const AdcChannel1 LDR_3_PIN = ADC1_CHANNEL_3;
const AdcChannel1 LDR_4_PIN = ADC1_CHANNEL_0;

const MinMax<int> ADC1_RANGE = {
	.Min = 75,
	.Max = 1045
};

const MinMax<int> ADC2_RANGE = {
	.Min = 73,
	.Max = 1030
};

class SolarTrackingClosedLoop
{
public:
	static SolarTrackingClosedLoop& Get();
	bool Start();
	bool Stop();
	bool IsRunning();

	void Update();

private:
	bool IsBetweenTolerance(float value, float min, float max);
	float Map(int value, int minMilliVolt, int maxMilliVolt, float min, float max);


	void UpdateLdr();
	void UpdateHorizontal();
	void UpdateVertical();

private:
	struct LdrArray 
	{
		float N = 0;
		float W = 0;
		float S = 0;
		float E = 0;
	};

	struct Threshold 
	{
		const struct MinMax<float> Horizontal = {
			.Min = 0.1,
			.Max = 1
		};
		const struct MinMax<float> Vertical = {
			.Min = 0.1,
			.Max = 1
		};
	};


	LdrArray ldrArray;
	Threshold treshold;
	bool bIsRunning = false;

	SolarTrackingClosedLoop();
	SolarTrackingClosedLoop(SolarTrackingClosedLoop const&) = delete;

};

