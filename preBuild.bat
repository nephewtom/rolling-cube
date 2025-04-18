@echo off
set /p=***** Building raylib *****<nul
pushd raylib\src
@echo on
make -j4
@echo off
popd
copy raylib\src\libraylib.a .

echo.
set /p=***** Building rlImGui *****<nul
pushd rlImGui
@echo on
make -j4
:: call build.bat
@echo off
popd
copy rlImGui\src\librlImGui.a .

echo.
set /p=\n***** Building r3d *****<nul
pushd r3d
@echo on
make -j4
pushd examples
make -j4
@echo off
popd
popd
