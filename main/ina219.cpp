#include "ina219.h"
#include "esp_log.h"
#include <string.h>
#include <math.h>
#include "algorithm"
#include "common.h"

#define BIT_SELECT(reg, bit) ((reg >> bit) & 1)

bool ina219_check_address(uint8_t addr)
{
    return addr >= 64 && addr <= 80;
}

ina219_err_t
ina219_read_reg(ina219_handle_t *ina219, ina219_reg_t reg, uint16_t *reg_value, uint16_t timeout_ms)
{
    if (ina219 == NULL)
    {
        return INA219_ERR_INVALID_ARGS;
    }
    if (!ina219_check_address(ina219->addr))
    {
        return INA219_ERR_INVALID_ADDRESS;
    }
    if (reg > INA219_REG_CALIBRATION || reg_value == NULL)
    {
        return INA219_ERR_INVALID_ARGS;
    }

    //ESP_LOGI(__func__, "read reg %d, with from this device %d", (int)reg, (int)ina219->addr);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_start(cmd));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_byte(cmd, ((ina219->addr) << 1) | I2C_MASTER_WRITE, true));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_byte(cmd, reg, true));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_stop(cmd));
    esp_err_t ret = i2c_master_cmd_begin(ina219->port, cmd, pdMS_TO_TICKS(timeout_ms));
    ESP_ERROR_CHECK_WITHOUT_ABORT(ret);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        ESP_LOGE(__func__, "ret0: %d", ret);
        return (ina219_err_t)ret;
    }

    uint8_t data[2];
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ((ina219->addr) << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 2, I2C_MASTER_ACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(ina219->port, cmd, pdMS_TO_TICKS(timeout_ms));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        ESP_LOGE(__func__, "ret1: %d", ret);
        return (ina219_err_t)ret;
    }

    *reg_value = (data[0] << 8) | data[1];

    //ESP_LOGE(__func__, "reg_value in hex: 0x%05" PRIx16 "\n", (int) reg_value);

    return INA219_OK;
}

ina219_err_t ina219_write_reg(ina219_handle_t *ina219, ina219_reg_t reg, uint16_t reg_value, uint16_t timeout_ms)
{
    if (ina219 == NULL)
    {
        return INA219_ERR_INVALID_ARGS;
    }

    if (reg > INA219_REG_CALIBRATION)
    {
        return INA219_ERR_INVALID_ARGS;
    }

    ESP_LOGI(__func__, "write reg %d, with value %d to this device %d", (int)reg, (int)reg_value, (int)ina219->addr);
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ((ina219->addr) << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, reg_value >> 8, true);
    i2c_master_write_byte(cmd, reg_value & 0x00FF, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(ina219->port, cmd, pdMS_TO_TICKS(timeout_ms));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        return (ina219_err_t)ret;
    }

    return INA219_OK;
}

ina219_err_t ina219_read_config(ina219_handle_t *ina219, ina219_reg_config_t *cfg, uint16_t timeout_ms)
{
    uint16_t reg_value = 0;
    ina219_err_t ret = ina219_read_reg(ina219, INA219_REG_CONFIGURATION, &reg_value, timeout_ms);
    if (ret != INA219_OK)
    {
        return ret;
    }

    cfg->RESET = (bool) BIT_SELECT(reg_value, 15);
    cfg->RESERVED = (uint8_t) BIT_SELECT(reg_value, 14);
    cfg->BUS_VOLTAGE_RANGE = (ina219_brng_t) BIT_SELECT(reg_value, 13);
    cfg->PG = (ina219_pg_t) ((BIT_SELECT(reg_value, 12) << 1) | (BIT_SELECT(reg_value, 11) << 0));
    cfg->BADC = (ina219_adc_t) ((BIT_SELECT(reg_value, 10) << 3) | (BIT_SELECT(reg_value, 9) << 2) | (BIT_SELECT(reg_value, 8) << 1) | (BIT_SELECT(reg_value, 7) << 0));
    cfg->SADC = (ina219_adc_t) ((BIT_SELECT(reg_value, 6) << 3) | (BIT_SELECT(reg_value, 5) << 2) | (BIT_SELECT(reg_value, 4) << 1) | (BIT_SELECT(reg_value, 3) << 0));
    cfg->MODE = (ina219_mode_t)((BIT_SELECT(reg_value, 2) << 2) | (BIT_SELECT(reg_value, 1) << 1) | (BIT_SELECT(reg_value, 0) << 0));

    return INA219_OK;
}

ina219_err_t ina219_read_shunt_voltage(ina219_handle_t *ina219, float *voltage, uint16_t timeout_ms)
{
    uint16_t reg_value = 0;

    ina219_err_t ret = ina219_read_reg(ina219, INA219_REG_SHUNT_VOLTAGE, &reg_value, timeout_ms);
    if (ret != INA219_OK)
    {
        return ret;
    }

    *voltage = ((int16_t)reg_value) * 0.00001;

    return INA219_OK;
}

ina219_err_t ina219_read_bus_voltage(ina219_handle_t *ina219, float *voltage, uint16_t timeout_ms)
{
    uint16_t reg_value = 0;

    ina219_err_t ret = ina219_read_reg(ina219, INA219_REG_BUS_VOLTAGE, &reg_value, timeout_ms);
    if (ret != INA219_OK)
    {
        return ret;
    }

    ESP_LOGI(__func__, "raw voltage reading %d", reg_value);

    *voltage = (reg_value >> 3) * 0.004;

    return INA219_OK;
}

ina219_err_t ina219_read_power(ina219_handle_t *ina219, float *power, uint16_t timeout_ms)
{
    uint16_t reg_value = 0;
    ina219_err_t ret = ina219_read_reg(ina219, INA219_REG_POWER, &reg_value, timeout_ms);
    if (ret != INA219_OK)
    {
        return ret;
    }


    ESP_LOGI(__func__, "raw reading: %d", (int)reg_value);

    const float ina219_powerMultiplier_mW = 2;

    *power = reg_value * (ina219_powerMultiplier_mW / 100.0f);

    return INA219_OK;
}

ina219_err_t ina219_read_current(ina219_handle_t *ina219, float *current, uint16_t timeout_ms)
{
    uint16_t reg_value = 0;
    ina219_err_t ret = ina219_read_reg(ina219, INA219_REG_CURRENT, &reg_value, timeout_ms);
    if (ret != INA219_OK)
    {
        return ret;
    }

    const int normalisation = 65535;
    const float ina219_currentDivider_mA = 10000;

    //overflow
    if (reg_value == 0) reg_value = normalisation;

    const int normalised = std::clamp((normalisation - reg_value), 0, normalisation);

    ESP_LOGI(__func__, "raw reading: %d, normalised %d", (int)reg_value, normalised);

    *current = normalised / ina219_currentDivider_mA;//ina219->cal._cur;

    return INA219_OK;
}

ina219_err_t ina219_calibrate(ina219_handle_t *ina219, uint16_t timeout_ms)
{
    uint16_t ina219_calValue = 4096;

    //set current
    ina219_write_reg(ina219, INA219_REG_CALIBRATION, ina219_calValue, timeout_ms);

    ESP_LOGI(__func__, "calibrated");

    return INA219_OK;
}

ina219_err_t ina219_init(i2c_port_t i2c_port, uint8_t addr, ina219_handle_t *handle)
{
    if (!ina219_check_address(addr))
    {
        return INA219_ERR_INVALID_ADDRESS;
    }
    if (handle == NULL)
    {
        return INA219_ERR_INVALID_ARGS;
    }

    if (i2c_port >= I2C_NUM_MAX)
    {
        return INA219_ERR_INVALID_ARGS;
    }

    handle->addr = addr;
    handle->port = i2c_port;

    return INA219_OK;
}

ina219_err_t ina219_set_config(ina219_handle_t *ina219, ina219_reg_config_t cfg, uint16_t timeout_ms)
{

    if (cfg.BUS_VOLTAGE_RANGE > INA219_BRNG_32V)
    {
        return INA219_ERR_INVALID_CONFIG;
    }

    if (cfg.PG > INA219_PG_320mV)
    {
        return INA219_ERR_INVALID_CONFIG;
    }
    if (cfg.BADC > INA219_ADC_68100us || cfg.SADC > INA219_ADC_68100us)
    {
        return INA219_ERR_INVALID_CONFIG;
    }

    if (cfg.MODE > INA219_MODE_SHUNT_BUS_CONTINUOUS)
    {
        return INA219_ERR_INVALID_CONFIG;
    }
    // cfg.RESET = true;
    uint16_t cfg_data = (cfg.RESET << 15) | (cfg.RESERVED << 14) | (cfg.BUS_VOLTAGE_RANGE << 13) | (cfg.PG << 11) | (cfg.BADC << 7) | (cfg.SADC << 3) | (cfg.MODE << 0);
    // uint16_t cfg_data = 0;
    ina219_err_t ret = ina219_write_reg(ina219, INA219_REG_CONFIGURATION, cfg_data, timeout_ms);
    if (ret != INA219_OK)
    {
        return ret;
    }

    ina219->cfg = cfg;

    return INA219_OK;
}

ina219_err_t ina219_set_default_config(ina219_handle_t* ina219, uint16_t timeout_ms)
{
    enum {
        INA219_CONFIG_BVOLTAGERANGE_16V = (0x0000), // 0-16V Range
        INA219_CONFIG_BVOLTAGERANGE_32V = (0x2000), // 0-32V Range
    };

    /** values for gain bits **/
    enum {
        INA219_CONFIG_GAIN_1_40MV = (0x0000),  // Gain 1, 40mV Range
        INA219_CONFIG_GAIN_2_80MV = (0x0800),  // Gain 2, 80mV Range
        INA219_CONFIG_GAIN_4_160MV = (0x1000), // Gain 4, 160mV Range
        INA219_CONFIG_GAIN_8_320MV = (0x1800), // Gain 8, 320mV Range
    };

    /** values for bus ADC resolution **/
    enum {
        INA219_CONFIG_BADCRES_9BIT = (0x0000),  // 9-bit bus res = 0..511
        INA219_CONFIG_BADCRES_10BIT = (0x0080), // 10-bit bus res = 0..1023
        INA219_CONFIG_BADCRES_11BIT = (0x0100), // 11-bit bus res = 0..2047
        INA219_CONFIG_BADCRES_12BIT = (0x0180), // 12-bit bus res = 0..4097
        INA219_CONFIG_BADCRES_12BIT_2S_1060US =
        (0x0480), // 2 x 12-bit bus samples averaged together
        INA219_CONFIG_BADCRES_12BIT_4S_2130US =
        (0x0500), // 4 x 12-bit bus samples averaged together
        INA219_CONFIG_BADCRES_12BIT_8S_4260US =
        (0x0580), // 8 x 12-bit bus samples averaged together
        INA219_CONFIG_BADCRES_12BIT_16S_8510US =
        (0x0600), // 16 x 12-bit bus samples averaged together
        INA219_CONFIG_BADCRES_12BIT_32S_17MS =
        (0x0680), // 32 x 12-bit bus samples averaged together
        INA219_CONFIG_BADCRES_12BIT_64S_34MS =
        (0x0700), // 64 x 12-bit bus samples averaged together
        INA219_CONFIG_BADCRES_12BIT_128S_69MS =
        (0x0780), // 128 x 12-bit bus samples averaged together

    };

    /** values for shunt ADC resolution **/
    enum {
        INA219_CONFIG_SADCRES_9BIT_1S_84US = (0x0000),   // 1 x 9-bit shunt sample
        INA219_CONFIG_SADCRES_10BIT_1S_148US = (0x0008), // 1 x 10-bit shunt sample
        INA219_CONFIG_SADCRES_11BIT_1S_276US = (0x0010), // 1 x 11-bit shunt sample
        INA219_CONFIG_SADCRES_12BIT_1S_532US = (0x0018), // 1 x 12-bit shunt sample
        INA219_CONFIG_SADCRES_12BIT_2S_1060US =
        (0x0048), // 2 x 12-bit shunt samples averaged together
        INA219_CONFIG_SADCRES_12BIT_4S_2130US =
        (0x0050), // 4 x 12-bit shunt samples averaged together
        INA219_CONFIG_SADCRES_12BIT_8S_4260US =
        (0x0058), // 8 x 12-bit shunt samples averaged together
        INA219_CONFIG_SADCRES_12BIT_16S_8510US =
        (0x0060), // 16 x 12-bit shunt samples averaged together
        INA219_CONFIG_SADCRES_12BIT_32S_17MS =
        (0x0068), // 32 x 12-bit shunt samples averaged together
        INA219_CONFIG_SADCRES_12BIT_64S_34MS =
        (0x0070), // 64 x 12-bit shunt samples averaged together
        INA219_CONFIG_SADCRES_12BIT_128S_69MS =
        (0x0078), // 128 x 12-bit shunt samples averaged together
    };

    /** values for operating mode **/
    enum {
        INA219_CONFIG_MODE_POWERDOWN = 0x00,       /**< power down */
        INA219_CONFIG_MODE_SVOLT_TRIGGERED = 0x01, /**< shunt voltage triggered */
        INA219_CONFIG_MODE_BVOLT_TRIGGERED = 0x02, /**< bus voltage triggered */
        INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED =
        0x03,                         /**< shunt and bus voltage triggered */
        INA219_CONFIG_MODE_ADCOFF = 0x04, /**< ADC off */
        INA219_CONFIG_MODE_SVOLT_CONTINUOUS = 0x05, /**< shunt voltage continuous */
        INA219_CONFIG_MODE_BVOLT_CONTINUOUS = 0x06, /**< bus voltage continuous */
        INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS =
        0x07, /**< shunt and bus voltage continuous */
    };


    uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
        INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
        INA219_CONFIG_SADCRES_12BIT_1S_532US |
        INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

    auto result = ina219_write_reg(ina219, INA219_REG_CONFIGURATION, config, timeout_ms);

    return result;
}

ina219_err_t ina219_reset_config(ina219_handle_t *ina219, uint16_t timeout_ms)
{
    return ina219_write_reg(ina219, INA219_REG_CONFIGURATION, 0x8000, timeout_ms);
}
