#pragma once
#include "common.h"
//#include "i2c-lcd1602.h"
#include <string>
#include <mutex>

class Display
{
public:
    static Display& Get();

    void Print(const char* ConstStringPtr);

    void Print(std::string& StringIn);

    void Print(std::string& StringIn, uint x, uint y);

    void Clear();

    void GoTo(uint x, uint y);

    void ShowCursor(bool bShowCursor);

    void Backlight(bool bIsTurnOn);

    void EnableAutoScroll(bool bIsEnabled);
private:
    Display(Display const&) = delete;
    void operator=(Display const&) = delete;

private:

    Display();
    std::recursive_mutex Mutex;
    //i2c_lcd1602_info_t* lcd_info;

};