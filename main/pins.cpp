#include "pins.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "common.h"
#include "vector"
#include "map"
#include "esp_log.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

static const adc_atten_t atten = ADC_ATTEN_DB_0;

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

static bool bCheckEFuse = false;

void CheckEfuse()
{
    if (bCheckEFuse) return;
    bCheckEFuse = true;
    
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) 
    {
        ESP_LOGI(PINS_TAG, "eFuse Two Point: Supported");
    }
    else 
    {
        ESP_LOGW(PINS_TAG, "eFuse Two Point: NOT supported");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) 
    {
        ESP_LOGI(PINS_TAG, "eFuse Vref: Supported");
    }
    else 
    {
        ESP_LOGW(PINS_TAG, "eFuse Vref: NOT supported");
    }
}

void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    }
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    }
    else {
        printf("Characterized using Default Vref\n");
    }
}

int PinFunctions::ReadAnalog1(AdcChannel1 pinNr)
{
    CheckEfuse();

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(pinNr, atten);

    esp_adc_cal_characteristics_t adcChars;

    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adcChars);
    //print_char_val_type(val_type);

    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) 
    {
        int read = adc1_get_raw(pinNr);
        if (read == -1)
        {
            ESP_LOGW(PINS_TAG, "%s : read failed channel adc 1 nr %d ", __func__, pinNr);
            return -1;
        }
        adc_reading += read;
    }
    adc_reading /= NO_OF_SAMPLES;

    //Convert adc_reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adcChars);

    return voltage;
}

int PinFunctions::ReadAnalog2(AdcChannel2 pinNr)
{
    CheckEfuse();

    adc2_config_channel_atten(pinNr, atten);

    esp_adc_cal_characteristics_t adcChars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_2, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adcChars);
    uint32_t adc_reading = 0;

    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) 
    {
        int raw;
        esp_err_t errorCheck = adc2_get_raw((adc2_channel_t)pinNr, ADC_WIDTH_BIT_12, &raw);
        if (errorCheck != ESP_OK)
        {
            ESP_LOGW(PINS_TAG, "%s : read failed channel adc 2 nr %d esp error code %s", __func__, pinNr, esp_err_to_name(errorCheck));
            return -1;
        }
        adc_reading += raw;
    }

    adc_reading /= NO_OF_SAMPLES;

    //Convert adc_reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adcChars);

    return voltage;
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
