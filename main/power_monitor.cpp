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

# define INA219_READ                            (0x01)
# define INA219_REG_CONFIG                      (0x00)
# define INA219_CONFIG_RESET                    (0x8000) // Reset Bit

# define INA219_CONFIG_BVOLTAGERANGE_MASK       (0x2000) // Bus Voltage Range Mask
# define INA219_CONFIG_BVOLTAGERANGE_16V        (0x0000) // 0-16V Range
# define INA219_CONFIG_BVOLTAGERANGE_32V        (0x2000) // 0-32V Range

# define INA219_CONFIG_GAIN_MASK                (0x1800) // Gain Mask
# define INA219_CONFIG_GAIN_1_40MV              (0x0000) // Gain 1, 40mV Range
# define INA219_CONFIG_GAIN_2_80MV              (0x0800) // Gain 2, 80mV Range
# define INA219_CONFIG_GAIN_4_160MV             (0x1000) // Gain 4, 160mV Range
# define INA219_CONFIG_GAIN_8_320MV             (0x1800) // Gain 8, 320mV Range

# define INA219_CONFIG_BADCRES_MASK             (0x0780) // Bus ADC Resolution Mask
# define INA219_CONFIG_BADCRES_9BIT             (0x0080) // 9-bit bus res = 0..511
# define INA219_CONFIG_BADCRES_10BIT            (0x0100) // 10-bit bus res = 0..1023
# define INA219_CONFIG_BADCRES_11BIT            (0x0200) // 11-bit bus res = 0..2047
# define INA219_CONFIG_BADCRES_12BIT            (0x0400) // 12-bit bus res = 0..4097

# define INA219_CONFIG_SADCRES_MASK             (0x0078) // Shunt ADC Resolution and Averaging Mask
# define INA219_CONFIG_SADCRES_9BIT_1S_84US     (0x0000) // 1 x 9-bit shunt sample
# define INA219_CONFIG_SADCRES_10BIT_1S_148US   (0x0008) // 1 x 10-bit shunt sample
# define INA219_CONFIG_SADCRES_11BIT_1S_276US   (0x0010) // 1 x 11-bit shunt sample
# define INA219_CONFIG_SADCRES_12BIT_1S_532US   (0x0018) // 1 x 12-bit shunt sample
# define INA219_CONFIG_SADCRES_12BIT_2S_1060US  (0x0048) // 2 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_4S_2130US  (0x0050) // 4 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_8S_4260US  (0x0058) // 8 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_16S_8510US (0x0060) // 16 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_32S_17MS   (0x0068) // 32 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_64S_34MS   (0x0070) // 64 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_128S_69MS  (0x0078) // 128 x 12-bit shunt samples averaged together

# define INA219_CONFIG_MODE_MASK                (0x0007) // Operating Mode Mask
# define INA219_CONFIG_MODE_POWERDOWN           (0x0000)
# define INA219_CONFIG_MODE_SVOLT_TRIGGERED     (0x0001)
# define INA219_CONFIG_MODE_BVOLT_TRIGGERED     (0x0002)
# define INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED (0x0003)
# define INA219_CONFIG_MODE_ADCOFF              (0x0004)
# define INA219_CONFIG_MODE_SVOLT_CONTINUOUS    (0x0005)
# define INA219_CONFIG_MODE_BVOLT_CONTINUOUS    (0x0006)
# define INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS (0x0007)

inline const int INA219_ADD = 64;
inline const gpio_num_t SDA_GPIO = GPIO_NUM_21;
inline const gpio_num_t SCL_GPIO = GPIO_NUM_22;

inline const char* POWERMONITORTAG = "powermonitor";

PowerMonitor::PowerMonitor()
{
	this->InitIna219Device();
	this->Update();
}

PowerMonitor& PowerMonitor::Get()
{
	static PowerMonitor instance;
	return instance;
}

float PowerMonitor::Voltage()
{
	this->Update();
	return this->voltage;
}

float PowerMonitor::Current()
{
	this->Update();
	return this->current;
}

void PowerMonitor::InitIna219Device()
{
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

	{

		//Write Calibration Value for 1A max current = 0x34 0x6D
		const int calibrationValue = 6606;
		i2c_cmd_handle_t cmd_calibrate = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd_calibrate));
		//Write Slave Address Byte
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd_calibrate, (INA219_ADD << 1) | I2C_MASTER_WRITE, true));
		//Write Register Pointer Byte
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd_calibrate, cal_addr, true));
		//Write Calibration MSB
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd_calibrate, (uint8_t)(calibrationValue >> 8), true));
		//Write Calibration LSB
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd_calibrate, (uint8_t)calibrationValue, true));

		ESP_ERROR_CHECK(i2c_master_stop(cmd_calibrate));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd_calibrate, 1000 / portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd_calibrate);
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

	{
		//config the measurement
		//Write Calibration Value for 1A max current = 0x34 0x6D
		const int configValue = INA219_CONFIG_BVOLTAGERANGE_32V |
			INA219_CONFIG_GAIN_8_320MV |
			INA219_CONFIG_BADCRES_12BIT |
			INA219_CONFIG_SADCRES_12BIT_128S_69MS |
			INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;;
		i2c_cmd_handle_t cmd_calibrate = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd_calibrate));
		//Write Slave Address Byte
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd_calibrate, (INA219_ADD << 1) | I2C_MASTER_WRITE, true));
		//Write Register Pointer Byte
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd_calibrate, cal_addr, true));
		//Write Calibration MSB
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd_calibrate, (uint8_t)(configValue >> 8), true));
		//Write Calibration LSB
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd_calibrate, (uint8_t)configValue, true));

		ESP_ERROR_CHECK(i2c_master_stop(cmd_calibrate));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd_calibrate, 1000 / portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd_calibrate);
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}


	//Read Calibration Register Value to confirm value written
	uint8_t cal_raw[2];
	i2c_cmd_handle_t cmd_read_cal = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd_read_cal));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd_read_cal, (INA219_ADD << 1) | I2C_MASTER_READ, true));
	ESP_ERROR_CHECK(i2c_master_read(cmd_read_cal, (uint8_t*)&cal_raw, 2, (i2c_ack_type_t) I2C_MASTER_ACK));
	ESP_ERROR_CHECK(i2c_master_stop(cmd_read_cal));
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd_read_cal, 1000 / portTICK_PERIOD_MS));
	i2c_cmd_link_delete(cmd_read_cal);

	int16_t cal_data = cal_raw[0] << 8 | cal_raw[1];
	printf("Calibration Register Value: %i\n", cal_data);

	//Calibration End


	//Move Pointer to desired register
	uint16_t d = 0;
	uint8_t temp;
	temp = (uint8_t)d;
	d >>= 8;

	uint8_t cmddata[3];
	cmddata[0] = reg_addr; // sends register address to read from
	cmddata[1] = (uint8_t)d; // write data hibyte
	cmddata[2] = temp; // write data lobyte;

	i2c_cmd_handle_t cmd_write = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd_write));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd_write, (INA219_ADD << 1) | I2C_MASTER_WRITE, true));
	ESP_ERROR_CHECK(i2c_master_write(cmd_write, &cmddata[0], 3, true));
	ESP_ERROR_CHECK(i2c_master_stop(cmd_write));
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd_write, 1000 / portTICK_PERIOD_MS));
	i2c_cmd_link_delete(cmd_write);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	//End

	//Read the previously set register
	uint8_t raw[2];
	i2c_cmd_handle_t cmd_read = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd_read));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd_read, (INA219_ADD << 1) | I2C_MASTER_READ, true));
	ESP_ERROR_CHECK(i2c_master_read(cmd_read, (uint8_t*)&raw, 2, (i2c_ack_type_t) I2C_MASTER_ACK));
	ESP_ERROR_CHECK(i2c_master_stop(cmd_read));
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd_read, 1000 / portTICK_PERIOD_MS));
	i2c_cmd_link_delete(cmd_read);
	//End
}

void PowerMonitor::Update()
{
	//Read the previously set register
	uint8_t raw[2];
	i2c_cmd_handle_t cmd_read = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd_read));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd_read, (INA219_ADD << 1) | I2C_MASTER_READ, true));
	ESP_ERROR_CHECK(i2c_master_read(cmd_read, (uint8_t*)&raw, 2, (i2c_ack_type_t)I2C_MASTER_ACK));
	ESP_ERROR_CHECK(i2c_master_stop(cmd_read));
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd_read, 1000 / portTICK_PERIOD_MS));
	i2c_cmd_link_delete(cmd_read);
	//End

	//Print the two bytes read from the register
	for (int i = 0; i < (sizeof(raw) / sizeof(raw[0])); i++)
	{
		//printf("%x \n", raw[i]);
	}
	//printf("\n");

	//Put the 2 bytes into a single 16 bit integer
	int16_t data = raw[0] << 8 | raw[1];

	if (reg_addr == 0x01) {
		//vShunt = raw register value multiplied by 10uV
		double vShunt = data * .00001;
		printf("Shunt Voltage: %fV \n", vShunt);
	}
	else if (reg_addr == 0x02) {
		//vBus = raw register value shifted right by 3 bits and then multiplied by 4mV
		double vBus = (data >> 3) * 0.004;
		this->voltage = vBus;
		printf("Bus Voltage: %fV \n", vBus);
	}
	else if (reg_addr == 0x03) {
		//Power = raw register value x (20 x Current_LSB)
		double Power = data * (20 * 0.00003052);
		this->power = Power;
		printf("Power: %fW \n", Power);
	}
	else if (reg_addr == 0x04) {
		//Current = raw register data x Current LSB
		double Current = data * 0.00002550;
		this->current = Current;
		printf("\rCurrent: %fA raw %d \n", Current, data);
		fflush(stdout);
	}

	//ToDo - deal with negative values
}
