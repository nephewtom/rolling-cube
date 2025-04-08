:: @echo off
:: del .\shader.exe
:: @echo on
:: g++ -g -Wall shader.cpp -o shader.exe -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -lgdi32 -lwinmm
:: @echo off
:: .\shader.exe


@echo off
del .\main.exe
@echo on
g++ -g -Wall log.cpp main.cpp -o main.exe -D_WIN32 -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -lgdi32 -lwinmm -std=c++20
@echo off
.\main.exe
