rm -f ./trayimg
g++ -g  -std=c++17 -Wall trayimg.cpp -o trayimg -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib  -framework OpenGL -framework CoreFoundation -framework CoreGraphics -framework IOKit -framework AppKit -Wall -Wextra -Wno-missing-field-initializers
./trayimg
