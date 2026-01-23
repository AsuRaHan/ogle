@echo off
chcp 65001 >nul
title OGLE - Генератор решения Visual Studio

echo.
echo ========================================
echo  OGLE 3D Engine - Solution Generator
echo ========================================
echo.

REM Проверяем, есть ли CMake
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ОШИБКА] CMake не установлен или не добавлен в PATH!
    echo Установите CMake: https://cmake.org/download/
    pause
    exit /b 1
)

echo [INFO] Найден CMake: %CMAKE_VERSION%
echo.

REM Создаем папку build если её нет
if not exist "build" (
    echo [INFO] Создаем папку build...
    mkdir build
)

REM Переходим в папку build
cd build

echo [INFO] Генерируем решение для Visual Studio 2022...
echo.

REM Генерируем решение
cmake .. -G "Visual Studio 17 2022" -A x64

if %ERRORLEVEL% neq 0 (
    echo.
    echo [ОШИБКА] Ошибка генерации решения!
    echo.
    echo Возможные причины:
    echo 1. Visual Studio 2022 не установлена
    echo 2. Нет поддержки x64
    echo 3. Неправильный путь к исходникам
    echo.
    echo Попробуйте другие генераторы:
    echo   cmake .. -G "Visual Studio 16 2019" -A x64
    echo   cmake .. -G "Visual Studio 15 2017" -A x64
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo  УСПЕШНО!
echo ========================================
echo.
echo Созданы файлы:
echo   - build\OGLE.sln         (решение)
echo   - build\CMakeCache.txt   (кеш CMake)
echo.
echo Что делать дальше:
echo 1. Откройте OGLE.sln в Visual Studio
echo 2. Установите "ogle" как стартовый проект
echo 3. Соберите решение (Ctrl+Shift+B)
echo 4. Запустите (F5)
echo.
echo Быстрые команды:
echo   - build\build_all.bat    (собрать все)
echo   - build\run_ogle.bat     (запустить)
echo.

REM Создаем вспомогательные батники
echo @echo off > build_all.bat
echo echo Сборка OGLE... >> build_all.bat
echo cmake --build . --config Debug >> build_all.bat
echo if %%ERRORLEVEL%% equ 0 echo Сборка успешна! >> build_all.bat
echo pause >> build_all.bat

echo @echo off > run_ogle.bat
echo echo Запуск OGLE... >> run_ogle.bat
echo if exist "Debug\ogle.exe" ( >> run_ogle.bat
echo     Debug\ogle.exe >> run_ogle.bat
echo ) else if exist "Release\ogle.exe" ( >> run_ogle.bat
echo     Release\ogle.exe >> run_ogle.bat
echo ) else ( >> run_ogle.bat
echo     echo Файл ogle.exe не найден! >> run_ogle.bat
echo ) >> run_ogle.bat
echo pause >> run_ogle.bat

echo [INFO] Созданы вспомогательные скрипты:
echo   - build\build_all.bat
echo   - build\run_ogle.bat
echo.

REM Возвращаемся обратно
cd ..

echo ========================================
echo  Следующие шаги:
echo  1. Откройте build\OGLE.sln
echo  2. Нажмите F5 для запуска
echo ========================================
echo.
pause