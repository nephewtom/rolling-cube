:: @echo off
:: del .\shader.exe
:: @echo on
:: g++ -g -Wall shader.cpp -o shader.exe -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -lgdi32 -lwinmm
:: @echo off
:: .\shader.exe


@echo off
del .\main.exe
@echo on
g++ -g -Wall main.cpp -o main.exe -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -lgdi32 -lwinmm
@echo off
.\main.exe
