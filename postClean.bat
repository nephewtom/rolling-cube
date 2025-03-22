@echo off
pushd raylib\src
mingw32-make clean
popd

pushd rlImGui
rmdir /s /q obj
del src\librlImGui.a
popd

del librlImGui.a libraylib.a
