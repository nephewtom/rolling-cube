.DEFAULT_GOAL := help

CC = gcc
CXX = g++
CXXFLAGS = -Wall

RAY_DIR = ../raylib/src
RLIMGUI_DIR = ../rlImGui/src
IMGUI_DIR = ../rlImGui/imgui
IMGUI_BE = ../rlImGui/imgui/backends
RAY_INC = -I$(RAY_DIR)
IMGUI_INC = -I$(RLIMGUI_DIR) -I$(IMGUI_DIR) -I$(IMGUI_BE)

PATH_LIBS = -L$(RAY_DIR) -L$(RLIMGUI_DIR)
IMGUILIB = -lrlImgui
LIBS = -lraylib -lgdi32 -lwinmm

OBJDIR = objs
OBJS = $(OBJDIR)/pbr.o $(OBJDIR)/shader.o $(OBJDIR)/shadertoy.o $(OBJDIR)/shadowmap.o $(OBJDIR)/skybox.o

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CXXFLAGS) $(RAY_INC) $(IMGUI_INC) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(RAY_INC) $(IMGUI_INC) -c -o $@ $<

fog: $(OBJDIR)/fog.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

pbr: $(OBJDIR)/pbr.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

shader: $(OBJDIR)/shader.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

shadertoy: $(OBJDIR)/shadertoy.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

shadowmap: $(OBJDIR)/shadowmap.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

skybox: $(OBJDIR)/skybox.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS)

collisions: $(OBJDIR)/collisions.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS)

partikel_demo: $(OBJDIR)/partikel_demo.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS)

particle: $(OBJDIR)/particle.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS)

all: pbr shader shadertoy skybox pbr shadowmap collisions partikel_demo

clean:
	rm -f $(OBJDIR)/*.o *.exe

help:
	@echo "Valid targets:"
	@echo "  all         Build all executables"
	@echo "  pbr         Build the 'pbr' executable"
	@echo "  shader      Build the 'shader' executable"
	@echo "  shadertoy   Build the 'shadertoy' executable"
	@echo "  shadowmap   Build the 'shadowmap' executable"
	@echo "  skybox      Build the 'skybox' executable"
	@echo "  collisions  Build the 'collisions' executable"
	@echo "  clean       Remove object files and executables"
	@echo "  help        Show this message"
