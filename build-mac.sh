rm -f ./main
clang++ -g -std=c++20 -Wall log.cpp main.cpp -o main -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -framework OpenGL -framework CoreFoundation -framework CoreGraphics -framework IOKit -framework AppKit -Wall -Wextra -Wno-missing-field-initializers
./main
