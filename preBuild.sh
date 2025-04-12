echo "***** Building raylib *****"
cd raylib/src
make -j4
cd -
cp raylib/src/libraylib.a .
echo
echo "***** Building rlImGui *****"
cd rlImGui
make -j4
cd -
cp rlImGui/src/librlImGui.a .
