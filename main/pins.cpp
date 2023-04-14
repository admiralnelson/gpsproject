#include "pins.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "common.h"
#include "vector"
#include "map"
#include "esp_log.h"

const char* TAG = "PIN MANAGER:";


bool PinFunctions::SetPin(uint32_t pinNr, bool enableOrNot)
{
    ESP_LOGI(TAG, "pinNr %d is set to enable? %d", pinNr, enableOrNot);
	return gpio_set_level(pinNr, enableOrNot) == ESP_OK;
}

bool PinFunctions::EnablePin(uint32_t pinNr)
{
	return SetPin(pinNr, true);
}

bool PinFunctions::DisablePin(uint32_t pinNr)
{
	return SetPin(pinNr, false);
}

bool PinFunctions::GetPinStatus(uint32_t pinNr)
{
	return gpio_get_level(pinNr);
}


static std::map<ledc_channel_t, bool> occupiedChannel = {
    {LEDC_CHANNEL_0 ,false},
    {LEDC_CHANNEL_1 ,false},
    {LEDC_CHANNEL_2 ,false},
    {LEDC_CHANNEL_3 ,false},
    {LEDC_CHANNEL_4 ,false},
    {LEDC_CHANNEL_5 ,false},
};

static std::map<ledc_timer_t, bool> occupiedTimer = {
    {LEDC_TIMER_0, false},
    {LEDC_TIMER_1, false},
    {LEDC_TIMER_2, false},
    {LEDC_TIMER_3, false},
};

ledc_channel_t FindUnusedChannel() {
    for (const auto& [channel, occupiedOrNot] : occupiedChannel) {
        if (occupiedOrNot == false) return channel;
    }

    ESP_LOGE(TAG, "all channels are used!");
    throw "all channel are used!";
}

ledc_timer_t FindUnusedTimer() {
    for (const auto& [timer, occupiedOrNot] : occupiedTimer) {
        if (occupiedOrNot == false) return timer;
    }

    ESP_LOGE(TAG, "all timer are used!");
    throw "all timer are used!";
}

PWMPin::PWMPin(uint32_t pinNr, uint32_t freqInHz, uint32_t resolution)
{
    ESP_LOGI(TAG, "finding unused channel");
    ledc_channel_t channel = FindUnusedChannel();
    ESP_LOGI(TAG, "finding unused timer");
    ledc_timer_t timer = FindUnusedTimer();
    ESP_LOGI(TAG, "found unused channel %d & unused timer %d", channel, timer);

    this->freqInHz = freqInHz;
    this->pinNr = pinNr;
    this->resolution = resolution;
    this->timerNr = timer;
    this->channelNr = channel;
    this->dutyCycleInPercentage = 0;

    ledc_timer_config_t timer_conf;
    timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    timer_conf.timer_num = (ledc_timer_t) this->timerNr;
    timer_conf.freq_hz = this->freqInHz;
    ledc_timer_bit_t resolutionEnum = (ledc_timer_bit_t) clamp<uint32_t>(this->resolution, 0, 20);
    timer_conf.duty_resolution = resolutionEnum;
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t ledc_conf;
    ledc_conf.gpio_num = this->pinNr;
    ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_conf.channel = (ledc_channel_t) this->channelNr;
    ledc_conf.intr_type = LEDC_INTR_DISABLE;
    ledc_conf.timer_sel = (ledc_timer_t) this->timerNr;
    ledc_conf.duty = this->dutyCycleInPercentage;
    ledc_channel_config(&ledc_conf);

    occupiedTimer[timer] = true;
    occupiedChannel[channel] = true;

    ESP_LOGI(TAG, "creating PWM pinNr %d at freq %d at resolution %d", this->pinNr, this->freqInHz, this->resolution);
}

void PWMPin::SetFrequency(uint32_t freqInHz)
{
    this->freqInHz = freqInHz;
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, (ledc_timer_t)timerNr, this->freqInHz);
    ESP_LOGI(TAG, "setting frequency for pinNr %d frequency %d hz", this->pinNr, this->freqInHz);
}

void PWMPin::SetDutyCycle(uint32_t percentage)
{
    this->dutyCycleInPercentage = percentage;
    uint32_t dutyCycle = (uint32_t)((this->dutyCycleInPercentage * ((1 << (ledc_timer_bit_t) this->resolution) - 1)) / 100);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)channelNr, dutyCycle);
    ESP_LOGI(TAG, "setting duty cycle for pinNr %d duty cycle in %d%% duty cycle in integer %d", this->pinNr, this->dutyCycleInPercentage, dutyCycle);
}

void PWMPin::Pause(bool pauseOrResume)
{
    ledc_channel_t channel = (ledc_channel_t)this->channelNr;
    uint32_t dutyCycle = (uint32_t)((this->dutyCycleInPercentage * ((1 << (ledc_timer_bit_t) this->resolution) - 1)) / 100);
    
    if (pauseOrResume)
    {
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, 0);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
    }
    else
    {
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, dutyCycle);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
    }
}

uint32_t PWMPin::GetDutyCycleInPercentage()
{
    return this->dutyCycleInPercentage;
}

PWMPin::~PWMPin()
{
    occupiedTimer[(ledc_timer_t)this->timerNr] = false;
    occupiedChannel[(ledc_channel_t)this->channelNr] = false;
    ESP_LOGI(TAG, "PWM pin destroyed pinNr %d", this->pinNr);
    ESP_LOGI(TAG, "removing occupied timer %d and channelNr", this->timerNr, this->channelNr);

}
