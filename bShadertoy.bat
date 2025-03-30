@echo off
del .\shadertoy.exe
@echo on
g++ -g -Wall shadertoy.cpp -o shadertoy.exe -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -lgdi32 -lwinmm
@echo off
.\shadertoy.exe
