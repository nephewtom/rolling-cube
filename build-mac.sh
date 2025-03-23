rm -f ./main
g++ -g  -std=c++17 -Wall main.cpp -o main -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib  -framework OpenGL -framework CoreFoundation -framework CoreGraphics -framework IOKit -framework AppKit -Wall -Wextra -Wno-missing-field-initializers
./main
