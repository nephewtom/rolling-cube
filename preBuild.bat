@echo off
set /p=***** Building raylib *****<nul
pushd raylib\src
@echo on
mingw32-make.exe
@echo off
popd
copy raylib\src\libraylib.a .

echo.
set /p=***** Building rlImGui *****<nul
pushd rlImGui
@echo on
call build.bat
@echo off
popd
copy rlImGui\src\librlImGui.a .
