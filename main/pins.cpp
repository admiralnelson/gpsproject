#include "pins.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "common.h"
#include "vector"
#include "map"
#include "esp_log.h"

const char* PINS_TAG = "PIN MANAGER:";

bool PinFunctions::SetPin(uint32_t pinNr, bool enableOrNot)
{
    ESP_LOGI(PINS_TAG, "pinNr %lu is set to enable? %d", pinNr, enableOrNot);
    gpio_set_direction((gpio_num_t)pinNr, GPIO_MODE_OUTPUT);
    return gpio_set_level((gpio_num_t)pinNr, enableOrNot) == ESP_OK;
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
	return gpio_get_level((gpio_num_t)pinNr);
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

    ESP_LOGE(PINS_TAG, "all channels are used!");
    throw "all channel are used!";
}

ledc_timer_t FindUnusedTimer() {
    for (const auto& [timer, occupiedOrNot] : occupiedTimer) {
        if (occupiedOrNot == false) return timer;
    }

    ESP_LOGE(PINS_TAG, "all timer are used!");
    throw "all timer are used!";
}

PWMPin::PWMPin(uint32_t pinNr, uint32_t freqInHz)
{
    ESP_LOGI(PINS_TAG, "finding unused channel");
    ledc_channel_t channel = FindUnusedChannel();
    ESP_LOGI(PINS_TAG, "finding unused timer");
    ledc_timer_t timer = FindUnusedTimer();
    ESP_LOGI(PINS_TAG, "found unused channel %d & unused timer %d", channel, timer);

    this->freqInHz = freqInHz;
    this->pinNr = pinNr;
    this->timerNr = timer;
    this->channelNr = channel;
    this->dutyCycleInPercentage = 0;


    ledc_channel_config_t ledc_conf = {};
    ledc_conf.channel = (ledc_channel_t)this->channelNr;//LEDC_CHANNEL_0;
    ledc_conf.gpio_num = (gpio_num_t)this->pinNr;
    ledc_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_conf.timer_sel = (ledc_timer_t)this->timerNr; //LEDC_TIMER_0;
    ledc_conf.intr_type = LEDC_INTR_DISABLE;
    ledc_conf.duty = 0;
    ledc_conf.hpoint = 0;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_conf));

    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = (ledc_timer_t)this->timerNr,//LEDC_TIMER_0,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    occupiedTimer[timer] = true;
    occupiedChannel[channel] = true;

    ESP_LOGI(PINS_TAG, "creating PWM pinNr %lu at freq %lu at resolution %d", this->pinNr, this->freqInHz, (int)LEDC_TIMER_13_BIT);
    this->Pause(true);
}

void PWMPin::SetFrequency(uint32_t freqInHz)
{
    this->freqInHz = freqInHz;
    ledc_set_freq(LEDC_LOW_SPEED_MODE, (ledc_timer_t)timerNr, this->freqInHz);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)this->channelNr);
    ESP_LOGI(PINS_TAG, "setting frequency for pinNr %lu frequency %lu hz", this->pinNr, this->freqInHz);
}

void PWMPin::SetDutyCycle(uint32_t percentage)
{
    this->dutyCycleInPercentage = percentage;
    uint32_t dutyCycle = map<uint32_t>(this->dutyCycleInPercentage, 0, 100, 0, 1023); 
    ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)this->channelNr, dutyCycle);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)this->channelNr);
    //ESP_LOGI(TAG, "setting duty cycle for pinNr %lu duty cycle in %lu%% duty cycle in integer %lu", this->pinNr, this->dutyCycleInPercentage, dutyCycle);
}

void PWMPin::Pause(bool pauseOrResume)
{
    ledc_channel_t channel = (ledc_channel_t)this->channelNr;
    uint32_t dutyCycle = map<uint32_t>(this->dutyCycleInPercentage, 0, 100, 0, 1023);
    
    if (pauseOrResume)
    {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
    }
    else
    {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, dutyCycle);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
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
    ESP_LOGI(PINS_TAG, "PWM pin destroyed pinNr %lu", this->pinNr);
    ESP_LOGI(PINS_TAG, "removing occupied timer %lu and channelNr %lu", this->timerNr, this->channelNr);

}
