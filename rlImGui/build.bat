@echo off
mkdir obj

set /p=--- Compiling ImGui files...<nul
rem @echo Compiling ImGui files...
@echo on
g++ -c ./imgui/imgui.cpp ./imgui/imgui_demo.cpp ./imgui/imgui_draw.cpp ./imgui/imgui_tables.cpp ./imgui/imgui_widgets.cpp -I ./imgui -I ./imgui/backends -I ../raylib/src/external -I ../raylib/src/external/glfw -DDEBUG -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS -DIMGUI_DISABLE_OBSOLETE_KEYIO -m64 -g -std=c++17
@echo off
@echo.

set /p=--- Compiling rlImGui...<nul
@echo on
g++ -c ./src/rlImGui.cpp -DDEBUG -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS -DIMGUI_DISABLE_OBSOLETE_KEYIO -I../raylib/src -I../raylib/src/external -I../raylib/src/external/glfw/include -I./src/ -I./imgui -m64 -g -std=c++17
@echo off
@echo.

set /p=--- Moving files...<nul
@echo on
move *.o obj
@echo off
@echo.

set /p=--- Linking rlImGui...<nul
@echo on
ar -rcs ./src/librlImGui.a obj/imgui.o obj/imgui_demo.o obj/imgui_draw.o obj/imgui_tables.o obj/imgui_widgets.o obj/rlImGui.o
