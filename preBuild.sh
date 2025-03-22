echo "***** Building raylib *****"
cd raylib/src
make
cd -
cp raylib/src/libraylib.a .
echo
echo "***** Building rlImGui *****"
cd rlImGui
./build.sh
cd -
cp rlImGui/src/librlImGui.a .
