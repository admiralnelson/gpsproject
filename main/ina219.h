/**
 * Author: Beowulf20
 * Repo: https://github.com/beowulf20/ina219-esp32
 * Reference Datasheet: https://www.ti.com/lit/ds/symlink/ina219.pdf?ts=1599093965648&ref_url=https%253A%252F%252Fwww.google.com%252F
 * 
 * INA219, is a Zerø-Drift, Bidirectional Current/Power Monitor With I²C Interface
 */


#pragma once

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

/**
 * INA219 error
 * 
 */
typedef enum
{
    INA219_OK,                  /*!< No error */
    INA219_ERR_INVALID_ADDRESS, /*!< invalid ina219 address */
    INA219_ERR_INVALID_ARGS,    /*!< invalid function arguments */
    INA219_ERR_INVALID_CONFIG,  /*!< invalid configuration struct */
} ina219_err_t;

typedef enum
{
    INA219_REG_CONFIGURATION = 0, /*!< configuration register */
    INA219_REG_SHUNT_VOLTAGE,     /*!< shunt voltage register */
    INA219_REG_BUS_VOLTAGE,       /*!< bus voltage register */
    INA219_REG_POWER,             /*!< power register */
    INA219_REG_CURRENT,           /*!< current register */
    INA219_REG_CALIBRATION        /*!< calibration register */
} ina219_reg_t;

typedef enum
{
    INA219_BRNG_16V, /*!< 16V FSR */
    INA219_BRNG_32V  /*!< 32V FSR */
} ina219_brng_t;

typedef enum
{
    INA219_PG_40mV = 0, /*!< Range:±40mV Gain:1/1  */
    INA219_PG_80mV,     /*!< Range:±80mV Gain:1/2  */
    INA219_PG_160mV,    /*!< Range:±160mV Gain:1/4 */
    INA219_PG_320mV     /*!< Range:±320mV Gain:1/8 */
} ina219_pg_t;

typedef enum
{
    INA219_ADC_84us = 0,        /*!< Mode/Samples: 9bit   Conversion Time: 84μs    */
    INA219_ADC_148us,           /*!< Mode/Samples: 10bit  Conversion Time: 148μs   */
    INA219_ADC_276us,           /*!< Mode/Samples: 11bit  Conversion Time: 276μs   */
    INA219_ADC_532us,           /*!< Mode/Samples: 12bit  Conversion Time: 532μs   */
    INA219_ADC_1060us = 0b1001, /*!< Mode/Samples: 2      Conversion Time: 1.06ms  */
    INA219_ADC_2130us,          /*!< Mode/Samples: 4      Conversion Time: 2.13ms  */
    INA219_ADC_4260us,          /*!< Mode/Samples: 8      Conversion Time: 4.26ms  */
    INA219_ADC_8510us,          /*!< Mode/Samples: 16     Conversion Time: 8.51ms  */
    INA219_ADC_17020us,         /*!< Mode/Samples: 32     Conversion Time: 17.02ms */
    INA219_ADC_34050us,         /*!< Mode/Samples: 64     Conversion Time: 34.05ms */
    INA219_ADC_68100us,         /*!< Mode/Samples: 128    Conversion Time: 68.10ms */
} ina219_adc_t;

typedef enum
{
    INA219_MODE_POWER_DOWN,          /*!< Power Down */
    INA219_MODE_SHUNT_TRIGGER,       /*!< Shunt Voltage, triggered */
    INA219_MODE_BUS_TRIGGER,         /*!< Bus Voltage, triggered */
    INA219_MODE_SHUNT_BUS_TRIGGER,   /*!< Shunt and bus, triggered */
    INA219_MODE_ADC_OFF,             /*!< ADC off (disabled) */
    INA219_MODE_SHUNT_CONTINUOUS,    /*!< Shunt voltage, continuous */
    INA219_MODE_BUS_CONTINUOUS,      /*!< Bus voltage, continuous */
    INA219_MODE_SHUNT_BUS_CONTINUOUS /*!< Shunt and bus, continuous */
} ina219_mode_t;

typedef struct ina219_reg_config_t
{
    bool RESET;
    uint8_t RESERVED;
    ina219_brng_t BUS_VOLTAGE_RANGE;
    ina219_pg_t PG;
    ina219_adc_t BADC;
    ina219_adc_t SADC;
    ina219_mode_t MODE;
} ina219_reg_config_t;

typedef struct ina219_i2c_calibration_t
{
    float _cur;
} ina219_i2c_calibration_t;

typedef struct ina219_handle_t
{
    ina219_i2c_calibration_t cal;
    i2c_port_t port;
    uint8_t addr;
    ina219_reg_config_t cfg;
} ina219_handle_t;

/**
 * @brief Initialize INA219 handler
 * @param i2c_port port num
 * @param addr slave device address
 * @param handle out handler
 * @return ina219 error type
 */
ina219_err_t ina219_init(i2c_port_t i2c_port, uint8_t addr, ina219_handle_t *handle);

/**
 * @brief Set configuration register
 * @param ina219 handler
 * @param cfg configuration struct
 * @param timeout_ms register write timeout in ms
 * @return ina219 error type
 */
ina219_err_t ina219_set_config(ina219_handle_t *ina219, ina219_reg_config_t cfg, uint16_t timeout_ms);

/*
*  @brief Reset to default config (32v 2A)
* 
*/
ina219_err_t ina219_set_default_config(ina219_handle_t* ina219, uint16_t timeout_ms);

/**
 * @brief Reset configuration register
 * @param ina219 handler 
 * @param timeout_ms register write timeout in ms
 * @return ina219 error type
 */
ina219_err_t ina219_reset_config(ina219_handle_t *ina219, uint16_t timeout_ms);

/**
 * @brief Read configuration register
 * @param ina219 handler 
 * @param cfg configuration struct
 * @return ina219 error type
 */
ina219_err_t ina219_read_config(ina219_handle_t *ina219, ina219_reg_config_t *cfg, uint16_t timeout_ms);

/**
 * @brief Read shunt voltage
 * @param ina219 INA219 handler 
 * @param voltage out shunt voltage
 * @param timeout_ms register read timeout in ms
 * @return ina219 error type
 */
ina219_err_t ina219_read_shunt_voltage(ina219_handle_t *ina219, float *voltage, uint16_t timeout_ms);

/**
 * @brief Read bus voltage register
 * @param ina219 handler 
 * @param voltage out bus voltage
 * @param timeout_ms register read timeout in ms
 * @return ina219 error type
 */
ina219_err_t ina219_read_bus_voltage(ina219_handle_t *ina219, float *voltage, uint16_t timeout_ms);

/**
 * @brief Read Power register
 * @param ina219 handler
 * @param power out power
 * @param timeout_ms register read timeout in ms
 * @return ina219 error type
 */
ina219_err_t ina219_read_power(ina219_handle_t *ina219, float *power, uint16_t timeout_ms);

/**
 * @brief Read current register
 * @param ina219 handler 
 * @param current out current
 * @return ina219 error type
 */
ina219_err_t ina219_read_current(ina219_handle_t *ina219, float *current, uint16_t timeout_ms);

/**
 * @brief Set Calibration register
 * @param ina219 handler 
 * @param shuntR shunt resistor value
 * @param maxCurrent max expected current
 * @param timeout_ms register write timeout in ms
 * @return ina219 error type
 */
ina219_err_t ina219_calibrate(ina219_handle_t *ina219, uint16_t timeout_ms);