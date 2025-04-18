CUBE = cube.exe
SRC = ./src

RAYLIB_DIR = ./raylib/src
RAYLIB_EXAMPLES = ./raylib/examples
RAYLIB = $(RAYLIB_DIR)/libraylib.a

RLIMGUI_DIR = ./rlImGui
RLIMGUI = $(RLIMGUI_DIR)/src/librlImgui.a

R3D_DIR = ./r3d
R3D_EXAMPLES = ./r3d/examples
R3D = $(R3D_DIR)/libr3d.a

TESTS = ./tests

help:
	@echo "Available targets:"
	@echo "  all			Builds everything"
	@echo "  cube			Builds rolling-cube"
	@echo "  tests			Builds tests"
	@echo "  raylib		Builds raylib"
	@echo "  raylib-ex		Builds raylib examples"
	@echo "  rlImGui		Builds rlImGui"
	@echo "  r3d			Builds r3d"
	@echo "  r3d-ex		Builds r3d examples"
	@echo "  clean			Cleans everything"
	@echo "  clean-r		Cleans raylib and examples"
	@echo "  clean-i		Cleans imgui and rlImGui"
	@echo "  clean-r3		Cleans r3d and examples"
	@echo "  clean-t		Cleans tests"

all:raylib raylib-ex rlImGui r3d r3d-ex tests cube

cube:raylib rlImGui
	$(MAKE) -C $(SRC) -j4
# build.bat

$(RAYLIB):
	@echo
	@echo "***** Building raylib"
	$(MAKE) -C $(RAYLIB_DIR) -j4

$(RLIMGUI):
	@echo
	@echo "***** Building rlImGui"
	$(MAKE) -C $(RLIMGUI_DIR) -j4

$(R3D):
	@echo
	@echo "***** Building r3d"
	$(MAKE) -C $(R3D_DIR) -j4

raylib:$(RAYLIB)

raylib-ex:$(RAYLIB)
	@echo
	@echo "***** Building raylib examples"
	$(MAKE) -C $(RAYLIB_EXAMPLES) -j4

rlImGui:$(RLIMGUI)

r3d:$(R3D)

r3d-ex:$(R3D)
	@echo
	@echo "***** Building raylib examples"
	$(MAKE) -C $(RAYLIB_EXAMPLES) -j4

.PHONY: tests

tests:raylib rlImGui
	@echo
	@echo "***** Building tests"
	$(MAKE) -C ./tests -j4 all

clean-r:
	@echo
	@echo "***** Cleaning raylib"
	$(MAKE) -C $(RAYLIB_DIR) -j4 clean
	@echo
	@echo "***** Cleaning raylib examples"
	$(MAKE) -C $(RAYLIB_EXAMPLES) -j4 clean

clean-i:
	@echo
	@echo "***** Cleaning rlImGui"
	$(MAKE) -C $(RLIMGUI_DIR) -j4 clean

clean-r3:
	@echo
	@echo "***** Cleaning r3d"
	$(MAKE) -C $(R3D_DIR) -j4 clean
	@echo
	@echo "***** Cleaning r3d examples"
	$(MAKE) -C $(R3D_EXAMPLES) -j4 clean

clean-t:
	@echo
	@echo "***** Cleaning tests"
	$(MAKE) -C $(TESTS) -j4 clean

clean: clean-r clean-i clean-r3 clean-t
