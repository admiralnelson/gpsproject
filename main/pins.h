#pragma once
#include "stdint.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

typedef adc1_channel_t AdcChannel1;
typedef adc2_channel_t AdcChannel2;

namespace PinFunctions {
	bool SetPin(uint32_t pinNr, bool enableOrNot);
	bool EnablePin(uint32_t pinNr);
	bool DisablePin(uint32_t pinNr);
	bool GetPinStatus(uint32_t pinNr);
	int ReadAnalog1(AdcChannel1 pinNr);
	int ReadAnalog2(AdcChannel2 pinNr);

}

/*
*
* PWMPin
* NOT THREAD SAFE
* 
*/
class PWMPin {
public:
	PWMPin(uint32_t pinNr, uint32_t freqInHz);
	void SetFrequency(uint32_t freqInHz);
	void SetDutyCycle(uint32_t percentage);
	void Pause(bool pauseOrResume);
	uint32_t GetDutyCycleInPercentage();
	~PWMPin();
private:
	uint32_t timerNr = 0;
	uint32_t channelNr = 0;
	uint32_t freqInHz = 0;
	uint32_t dutyCycleInPercentage = 0;
	uint32_t pinNr = 0;
	uint32_t resolution = 0;

};