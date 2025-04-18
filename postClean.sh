cd raylib/src
make clean
cd -

cd rlImGui
rm -rf obj
rm src/librlImGui.a
cd -
rm libraylib.a librlImGui.a

cd r3d/examples
make clean
cd ..
make clean
cd ..
