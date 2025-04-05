@echo off
pushd raylib\src
make clean
popd

pushd rlImGui
make clean
:: rmdir /s /q obj
:: del src\librlImGui.a
popd

del librlImGui.a libraylib.a
