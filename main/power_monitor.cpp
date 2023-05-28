#include "power_monitor.h"
#include "string.h"
#include "esp_log.h"
#include "freertos\FreeRTOS.h"
#include "freertos\task.h"
#include "driver\i2c.h"
#include "esp_types.h"

#define reg_addr   0x04  //00 = config | 01 = vShunt | 02 = vBus | 03 = Power | 04 = Current 
#define cal_addr   0x05  //Calibration register address
#define reg_config 0x00

#define TIMEOUT 100

inline const int INA219_ADD = 64;
inline const gpio_num_t SDA_GPIO = GPIO_NUM_21;
inline const gpio_num_t SCL_GPIO = GPIO_NUM_22;

inline const char* POWERMONITORTAG = "powermonitor";

PowerMonitor::PowerMonitor()
{
	this->InitIna219Device();
}

PowerMonitor& PowerMonitor::Get()
{
	static PowerMonitor instance;
	return instance;
}

bool PowerMonitor::CalibrateDevice(const CalibrationData& values)
{
	this->calibrationValues = values;
	auto result = ina219_calibrate(&this->ina219Device, TIMEOUT);
	//ina219_err_t result = ina219_calibrate(&this->ina219Device, this->calibrationValues.shuntResistance, this->calibrationValues.maxCurrent, TIMEOUT);
	return result == INA219_OK;
}

float PowerMonitor::Voltage()
{
	float result = 0;
	ina219_read_bus_voltage(&this->ina219Device, &result, TIMEOUT);
	return result;
}

float PowerMonitor::Current()
{
	float result = 0;
	ina219_read_current(&this->ina219Device, &result, TIMEOUT);
	return result;
}

float PowerMonitor::Power()
{
	float result = 0;
	ina219_read_power(&this->ina219Device, &result, TIMEOUT);
	return result;
}

PowerMonitor::CalibrationData PowerMonitor::GetCalibration()
{
	return this->calibrationValues;
}

void PowerMonitor::ResetCalibration()
{
	CalibrationData defaultCalibration;
	defaultCalibration.maxCurrent = 30.0f;
	defaultCalibration.shuntResistance = 0.001f;
	this->CalibrateDevice(defaultCalibration);
}

void PowerMonitor::InitIna219Device()
{
	if (this->bInited) return;

	i2c_config_t i2c_config = 
	{ 
	  .mode = I2C_MODE_MASTER, 
	  .sda_io_num = SDA_GPIO, 
	  .scl_io_num = SCL_GPIO, 
	  .sda_pullup_en = GPIO_PULLUP_ENABLE, 
	  .scl_pullup_en = GPIO_PULLUP_ENABLE, 
	}; 

	i2c_config.master.clk_speed = 100000; 

	i2c_param_config(I2C_NUM_0, &i2c_config);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

	ina219_handle_t handle{};
	this->ina219Device = handle;	
	ina219_err_t result = ina219_init(I2C_NUM_0, INA219_ADD, &this->ina219Device);
	switch (result)
	{
	case INA219_OK:
		ESP_LOGI(POWERMONITORTAG, "configuration ok");
		this->bInited = true;
		break;
	case INA219_ERR_INVALID_ADDRESS:
		ESP_LOGE(POWERMONITORTAG, "configuration INA219_ERR_INVALID_ADDRESS");
		return;
		break;
	case INA219_ERR_INVALID_ARGS:
		ESP_LOGE(POWERMONITORTAG, "configuration INA219_ERR_INVALID_ARGS");
		return;
		break;
	case INA219_ERR_INVALID_CONFIG:
		ESP_LOGE(POWERMONITORTAG, "configuration INA219_ERR_INVALID_CONFIG");
		return;
		break;
	default:
		ESP_LOGE(POWERMONITORTAG, "unknown error on init");
		return;
		break;
	}

	ina219_set_default_config(&this->ina219Device, TIMEOUT);

	this->ResetCalibration();
}

cjsonpp::JSONObject PowerMonitor::CalibrationData::ToJsonObject() const
{
	cjsonpp::JSONObject object;
	object.set("maxCurrent", (double)this->maxCurrent);
	object.set("shuntResistance", (double)this->shuntResistance);

	return object;
}

bool PowerMonitor::CalibrationData::FromJsonObject(const cjsonpp::JSONObject& Object) noexcept
{
	try
	{
		if (Object.has("maxCurrent")) this->maxCurrent = (float)Object.get<double>("maxCurrent");
		if (Object.has("shuntResistance")) this->shuntResistance = (float)Object.get<double>("shuntResistance");
		return true;
	}
	catch (const std::exception& e)
	{
		ESP_LOGE(__func__, "malformed json data %s", e.what());
	}
	return false;
}
