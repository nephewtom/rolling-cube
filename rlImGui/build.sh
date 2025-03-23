mkdir -p obj
echo "--- Compiling ImGui files..."
g++ -c ./imgui/imgui.cpp ./imgui/imgui_demo.cpp ./imgui/imgui_draw.cpp ./imgui/imgui_tables.cpp ./imgui/imgui_widgets.cpp -I ./imgui -I ./imgui/backends -I ../raylib/src/external -I ../raylib/src/external/glfw -DDEBUG -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS -DIMGUI_DISABLE_OBSOLETE_KEYIO -m64 -g -std=c++17

echo "--- Compiling rlImGUI..."
g++ -c ./src/rlImGui.cpp -DDEBUG -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS -DIMGUI_DISABLE_OBSOLETE_KEYIO -I../raylib/src -I../raylib/src/external -I../raylib/src/external/glfw/include -I./src/ -I./imgui -m64 -g -std=c++17

echo "--- Moving files..."
mv *.o obj

echo "--- Linking rlImGui..."
ar -rcs ./src/librlImGui.a obj/*.o
