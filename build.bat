@echo off
setlocal

REM Параметр: конфигурация (Debug|Release), по умолчанию Debug
set CONFIGURATION=Debug
if "%1"=="Release" set CONFIGURATION=Release
if "%1"=="RelWithDebInfo" set CONFIGURATION=RelWithDebInfo

REM Собрать проект OGLE3D в папке build
if not exist build mkdir build

cd /d %~dp0build
cmake .. -DCMAKE_BUILD_TYPE=%CONFIGURATION%
if errorlevel 1 (
    echo CMake configuration failed.
    exit /b 1
)

cmake --build . --config %CONFIGURATION%
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

if "%CONFIGURATION%"=="Debug" (
    set OUTPUT=bin\OGLE3D.exe
) else (
    set OUTPUT=bin\OGLE3D.exe
)

echo Build complete. Исполняемый файл: %~dp0%OUTPUT%
endlocal
