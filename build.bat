@echo off
::config section
set DATA_PARTITION_START_OFFSET="0x001d2000"
set COM_PORT="COM3"
set LITTLEFS_DEST="%cd%\build\littleFS_data.bin"
set ESP_TOOL="D:\dev\esp-idf\components\esptool_py\esptool\esptool.py"
::end of config section

call python "%ESP_TOOL%" --help

if %ERRORLEVEL% neq 0 (
	goto :eof
)

echo building and flashing
::idf.py flash
call idf.py flash
if %ERRORLEVEL% neq 0 (
	goto :eof
)

echo converting data to littlefs image
::make_image littlefs
call tools\mklittlefs.exe -c "%cd%\data" %LITTLEFS_DEST% -s 1200000
if %ERRORLEVEL% neq 0 (
	goto :eof
)

echo upload image to data
::esp_tool upload littlefs image
python "%ESP_TOOL%" --port %COM_PORT% write_flash %DATA_PARTITION_START_OFFSET% %LITTLEFS_DEST% 
if %ERRORLEVEL% neq 0 (
	goto :eof
)

echo running monitor
::idf.py monitor
call idf.py monitor