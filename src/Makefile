.DEFAULT_GOAL := all

CXX = g++
CXXFLAGS = -g -std=c++20 -Wall

RAY_DIR = ../raylib/src
RLIMGUI_DIR = ../rlImGui/src
IMGUI_DIR = ../rlImGui/imgui
IMGUI_BE = ../rlImGui/imgui/backends
RAY_INC = -I$(RAY_DIR)
IMGUI_INC = -I$(RLIMGUI_DIR) -I$(IMGUI_DIR) -I$(IMGUI_BE)

PATH_LIBS = -L$(RAY_DIR) -L$(RLIMGUI_DIR)
IMGUILIB = -lrlImgui

ifeq ($(OS), Windows_NT)
	LIBS = -lraylib -lgdi32 -lwinmm
	CXXFLAGS += -D_WIN32
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin) # Apple
	LIBS = -lrlImGui -lraylib -framework OpenGL -framework CoreFoundation -framework CoreGraphics -framework IOKit -framework AppKit
	CXXFLAGS += -Wextra -Wno-missing-field-initializers
endif

# $(info OS= $(OS))
# $(info UNAME_S= $(UNAME_S))
# $(info LIBS= $(LIBS))

OBJDIR = objs
OBJS = $(OBJDIR)/main.o $(OBJDIR)/log.o

OUTDIR = ..

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.c $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(RAY_INC) $(IMGUI_INC) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(RAY_INC) $(IMGUI_INC) -c -o $@ $<

cube: $(OBJS)
	$(CXX) -o $(OUTDIR)/$@ $^ $(PATH_LIBS) $(LIBS) $(IMGUILIB)


all: clean $(OBJS) cube
	cd $(OUTDIR) && ./cube

clean:
	rm -rf $(OBJDIR) $(OUTDIR)/cube
