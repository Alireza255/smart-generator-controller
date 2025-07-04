@echo off
setlocal enabledelayedexpansion

:: Configuration
set HEADER_FILE=controller.h
set OUTPUT_FILE=bits_constants.ini
set BASE_OFFSET=1000
set PAGE=1

:: Parse structs and extract boolean variables
set engine_vars=
set config_vars=
set byte_offset=%BASE_OFFSET%
set bit_index=0

for /f "tokens=*" %%a in ('type "%HEADER_FILE%" ^| findstr /c:"bool"') do (
    set "line=%%a"
    set "line=!line:    = !"
    set "line=!line:   = !"
    set "line=!line:  = !"
    
    if "!line!" neq "!line:engine_t=!" (
        set in_engine=1
        set in_config=0
    ) else if "!line!" neq "!line:configuration_t=!" (
        set in_engine=0
        set in_config=1
    ) else if "!line!"=="};" (
        set in_engine=0
        set in_config=0
    ) else if defined in_engine (
        for /f "tokens=2 delims= " %%b in ("!line!") do (
            set "var=%%b"
            set "var=!var:~0,-1!"
            set engine_vars=!engine_vars! !var!
        )
    ) else if defined in_config (
        for /f "tokens=2 delims= " %%b in ("!line!") do (
            set "var=%%b"
            set "var=!var:~0,-1!"
            set config_vars=!config_vars! !var!
        )
    )
)

:: Write INI header
echo ; Generated bit field constants > "%OUTPUT_FILE%"
echo ; From header: %HEADER_FILE% >> "%OUTPUT_FILE%"
echo ; Auto-generated on %date% %time% >> "%OUTPUT_FILE%"
echo. >> "%OUTPUT_FILE%"
echo [Constants] >> "%OUTPUT_FILE%"
echo page = %PAGE% >> "%OUTPUT_FILE%"
echo. >> "%OUTPUT_FILE%"

:: Process engine variables
echo ; -- engine_t booleans -- >> "%OUTPUT_FILE%"
for %%v in (%engine_vars%) do (
    echo %%v = bits, U08, !byte_offset!, [!bit_index!:!bit_index!], "Off", "On" >> "%OUTPUT_FILE%"
    set /a bit_index+=1
    if !bit_index! equ 8 (
        set /a byte_offset+=1
        set bit_index=0
    )
)

:: Process configuration variables
echo. >> "%OUTPUT_FILE%"
echo ; -- configuration_t booleans -- >> "%OUTPUT_FILE%"
for %%v in (%config_vars%) do (
    echo %%v = bits, U08, !byte_offset!, [!bit_index!:!bit_index!], "Off", "On" >> "%OUTPUT_FILE%"
    set /a bit_index+=1
    if !bit_index! equ 8 (
        set /a byte_offset+=1
        set bit_index=0
    )
)

:: Final newline
echo. >> "%OUTPUT_FILE%"

echo Generated %OUTPUT_FILE% with !byte_offset! bytes of bit fields