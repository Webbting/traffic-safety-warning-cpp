@echo off
setlocal

set "ROOT=%~dp0"
set "VSDEVCMD="

if exist "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" (
    set "VSDEVCMD=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\Tools\VsDevCmd.bat" (
    set "VSDEVCMD=C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\Tools\VsDevCmd.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    set "VSDEVCMD=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" (
    set "VSDEVCMD=C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
)

if not defined VSDEVCMD (
    echo Cannot find Visual Studio VsDevCmd.bat.
    echo Install Visual Studio Build Tools with the C++ workload, or build with CMake and another C++ compiler.
    exit /b 1
)

if not exist "%ROOT%build" mkdir "%ROOT%build"

call "%VSDEVCMD%" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 1

cl /nologo /std:c++17 /EHsc /W4 /permissive- /Fo"%ROOT%build\\" "%ROOT%src\main.cpp" /Fe:"%ROOT%build\traffic_safety_ai.exe"
if errorlevel 1 exit /b 1

echo.
echo Build complete:
echo   %ROOT%build\traffic_safety_ai.exe
echo.
echo Try:
echo   "%ROOT%build\traffic_safety_ai.exe" --demo
