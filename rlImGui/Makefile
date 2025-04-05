# Reference to write this Makefile is: "./imgui/examples/example_null/Makefile"

# Tools and directories
CXX = gcc
AR = ar
RL_DIR = ./src
RL_LIB = $(RL_DIR)/librlImgui.a
IMGUI_DIR = ./imgui
RAYLIB_DIR = ../raylib/src

# Sources and objects
SOURCES = ./src/rlImGui.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))


# Per OS/platform in case it is needed
ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
endif

# Flags
CXXFLAGS += -std=c++17 -I$(IMGUI_DIR) -I$(RL_DIR) -I$(IMGUI_DIR)/backends -I$(RAYLIB_DIR) -I$(RAYLIB_DIR)/external -I$(RAYLIB_DIR)/external/glfw
CXXFLAGS += -DDEBUG -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS -DIMGUI_DISABLE_OBSOLETE_KEYIO -m64 -g -Wall -Wformat -Wextra -Wpedantic

# Build rules
%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(RL_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(RL_LIB)
	@echo Building $(RL_LIB)

$(RL_LIB): $(OBJS)
	$(AR) -rcs $@ $^

clean:
	rm -f $(RL_LIB) $(OBJS)
