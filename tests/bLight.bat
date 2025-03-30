@echo off
del .\shaders_basic_lighting.exe
@echo on
g++ -g -Wall shaders_basic_lighting.cpp -o shaders_basic_lighting.exe -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -lgdi32 -lwinmm
@echo off
.\shaders_basic_lighting.exe
