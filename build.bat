@echo off
del .\cube.exe

@echo on
if "%1"=="noimgui" (
   g++ -g -Wall -D NO_IMGUI src/log.cpp src/main.cpp -o cube.exe -D_WIN32 -I./src -I./raylib/src -L./raylib/src -lraylib -lgdi32 -lwinmm -std=c++20
) else (
  g++ -g -Wall src/log.cpp src/main.cpp -o cube.exe -D_WIN32 -I./src -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -lgdi32 -lwinmm -std=c++20
)
@echo off
.\cube.exe
