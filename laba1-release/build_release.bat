@echo off
rem Build ItteC (Laboratory 1, variant 9) for Windows in release mode and deploy.
rem Requirements: Qt for Windows (MinGW), qmake, mingw32-make and windeployqt
rem must be available in PATH (use the Qt MinGW command prompt).

setlocal
set ROOT=%~dp0
cd /d "%ROOT%..\src"

set BUILD_DIR=build-release
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

qmake ItteC.pro -spec win32-g++ CONFIG+=release CONFIG-=debug || goto :error
mingw32-make -j%NUMBER_OF_PROCESSORS% || goto :error

copy /y release\ItteC.exe "%ROOT%ItteC.exe" || goto :error
windeployqt --release --compiler-runtime --dir "%ROOT%" "%ROOT%ItteC.exe" || goto :error

echo.
echo Done. Release folder: %ROOT%
exit /b 0

:error
echo.
echo Build failed. Check that qmake/mingw32-make/windeployqt are in PATH.
exit /b 1