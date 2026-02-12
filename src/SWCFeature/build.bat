@echo off
REM Build script for Algorithm CLI tools
REM Requires: Visual Studio 2022 (or later) with C++ workload, Qt6 Core, Ninja

call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
if errorlevel 1 (
    echo ERROR: Failed to initialize MSVC environment.
    exit /b 1
)

cd /d "%~dp0"
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
if errorlevel 1 (
    echo ERROR: CMake configure failed.
    exit /b 1
)

cmake --build build
if errorlevel 1 (
    echo ERROR: CMake build failed.
    exit /b 1
)

echo.
echo === Build Succeeded ===
echo Executables:
echo   build\pruning_swc\pruning_swc.exe
echo   build\resample_swc\resample_swc.exe
echo   build\sort_neuron_swc\sort_neuron_swc.exe
echo   build\global_neuron_feature\global_neuron_feature.exe
