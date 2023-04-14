#include "esp_log.h"

//#include "smbus.h"
//#include "i2c-lcd1602.h"
#include "display.h"
#include <sstream>
#include "sdkconfig.h"

#define TAG "display"

// LCD1602
#define LCD_NUM_ROWS               2
#define LCD_NUM_COLUMNS            32
#define LCD_NUM_VISIBLE_COLUMNS    16

// LCD2004
//#define LCD_NUM_ROWS               4
//#define LCD_NUM_COLUMNS            40
//#define LCD_NUM_VISIBLE_COLUMNS    20

// Undefine USE_STDIN if no stdin is available (e.g. no USB UART) - a fixed delay will occur instead of a wait for a keypress.
#define USE_STDIN  1
//#undef USE_STDIN

#define I2C_MASTER_NUM           I2C_NUM_0
#define I2C_MASTER_TX_BUF_LEN    0                     // disabled
#define I2C_MASTER_RX_BUF_LEN    0                     // disabled
#define I2C_MASTER_FREQ_HZ       100000
#define I2C_MASTER_SDA_IO        CONFIG_I2C_MASTER_SDA
#define I2C_MASTER_SCL_IO        CONFIG_I2C_MASTER_SCL


//Display::Display()
//{
//    int i2c_master_port = I2C_MASTER_NUM;
//    i2c_config_t conf;
//    conf.mode = I2C_MODE_MASTER;
//    conf.clk_flags = 0;
//    conf.sda_io_num = I2C_MASTER_SDA_IO;
//    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;  // GY-2561 provides 10k pullups
//    conf.scl_io_num = I2C_MASTER_SCL_IO;
//    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;  // GY-2561 provides 10k pullups
//    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
//    i2c_param_config(i2c_master_port, &conf);
//    i2c_driver_install(
//        i2c_master_port, 
//        conf.mode,
//        I2C_MASTER_RX_BUF_LEN,
//        I2C_MASTER_TX_BUF_LEN, 0);
//
//    i2c_port_t i2c_num = I2C_MASTER_NUM;
//    uint8_t address = CONFIG_LCD1602_I2C_ADDRESS;
//
//    // Set up the SMBus
//    smbus_info_t* smbus_info = smbus_malloc();
//    ESP_ERROR_CHECK(smbus_init(smbus_info, i2c_num, address));
//    ESP_ERROR_CHECK(smbus_set_timeout(smbus_info, 1000 / portTICK_RATE_MS));
//
//    // Set up the LCD1602 device with backlight off
//    lcd_info = i2c_lcd1602_malloc();
//    ESP_ERROR_CHECK(i2c_lcd1602_init(lcd_info, smbus_info, true,
//        LCD_NUM_ROWS, LCD_NUM_COLUMNS, LCD_NUM_VISIBLE_COLUMNS));
//
//    ESP_ERROR_CHECK(i2c_lcd1602_reset(lcd_info));
//
//    i2c_lcd1602_set_backlight(lcd_info, false);
//    ESP_LOGI(TAG, "initialised");
//}
//
//Display& Display::Get()
//{
//    static Display instance;
//    return instance;
//}
//
//void Display::Print(const char* ConstStringPtr)
//{
//    if (Mutex.try_lock())
//    {
//        std::stringstream ss(ConstStringPtr);
//        std::string out;
//        uint y = 0;
//        if (ConstStringPtr != nullptr)
//        {
//            while (std::getline(ss, out, '\n'))
//            {
//                GoTo(0, y);
//                i2c_lcd1602_write_string(lcd_info, out.c_str());;
//                y++;
//            }
//        }
//        GoTo(0, 0);
//        Mutex.unlock();
//    }
//    else
//    {
//        ESP_LOGW(TAG, "unable to write to LCD (mutex locked)");
//    }
//    
//}
//
//void Display::Print(std::string& StringIn)
//{
//    if (Mutex.try_lock())
//    {
//        std::stringstream ss(StringIn);
//        std::string out;
//        uint y = 0;
//        while (std::getline(ss, out, '\n'))
//        {
//            GoTo(0, y);
//            i2c_lcd1602_write_string(lcd_info, out.c_str());;
//            y++;
//        }
//        GoTo(0, 0);
//        Mutex.unlock();
//    }
//    else
//    {
//        ESP_LOGW(TAG, "unable to write to LCD (mutex locked)");
//    }
//}
//
//void Display::Print(std::string& StringIn, uint x, uint y)
//{
//    if (Mutex.try_lock())
//    {
//        GoTo(x, y);
//        std::stringstream ss(StringIn);
//        std::string out;
//        while (std::getline(ss, out, '\n'))
//        {
//            GoTo(x, y);
//            i2c_lcd1602_write_string(lcd_info, out.c_str());;
//            y++;
//        }
//        Mutex.unlock();
//    }
//    else
//    {
//        ESP_LOGW(TAG, "unable to write to LCD (mutex locked)");
//    }
//}
//
//void Display::Clear()
//{
//    if (Mutex.try_lock())
//    {
//        GoTo(0, 0);
//        i2c_lcd1602_clear(lcd_info);
//        Mutex.unlock();
//    }
//    else
//    {
//        ESP_LOGW(TAG, "unable to clear LCD (mutex locked)");
//    }
//}
//
//void Display::GoTo(uint x, uint y)
//{
//    if (Mutex.try_lock())
//    {
//        i2c_lcd1602_move_cursor(lcd_info, x, y);
//        Mutex.unlock();
//    }
//    else
//    {
//        ESP_LOGW(TAG, "unable to move LCD cursor (mutex locked)");
//    }
//}
//
//void Display::ShowCursor(bool bShowCursor)
//{
//    if (Mutex.try_lock())
//    {
//        i2c_lcd1602_set_cursor(lcd_info, bShowCursor);
//        Mutex.unlock();
//    }
//    else
//    {
//        ESP_LOGW(TAG, "unable to show LCD cursor (mutex locked)");
//    }
//}
//
//void Display::Backlight(bool bIsTurnOn)
//{
//    if (Mutex.try_lock())
//    {
//        i2c_lcd1602_set_backlight(lcd_info, bIsTurnOn);
//        Mutex.unlock();
//    }
//    else
//    {
//        ESP_LOGW(TAG, "unable to set backlight LCD (mutex locked)");
//    }
//}
//
//void Display::EnableAutoScroll(bool bIsEnabled)
//{
//    if (Mutex.try_lock())
//    {
//        i2c_lcd1602_set_auto_scroll(lcd_info, bIsEnabled);
//        Mutex.unlock();
//    }
//    else
//    {
//        ESP_LOGW(TAG, "unable to set autoscroll LCD (mutex locked)");
//    }
//}
//
