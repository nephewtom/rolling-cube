.DEFAULT_GOAL := all

CC = gcc
CXX = g++
CXXFLAGS = -g -Wall

R3D_DIR = ../src
RAY_DIR = ../../raylib/src
RLIMGUI_DIR = ../../rlImGui/src
IMGUI_DIR = ../rlImGui/imgui
IMGUI_BE = ../rlImGui/imgui/backends

RAY_INC = -I$(R3D_DIR) -I$(RAY_DIR)
IMGUI_INC = -I$(RLIMGUI_DIR) -I$(IMGUI_DIR) -I$(IMGUI_BE)

PATH_LIBS = -L$(R3D_DIR) -L$(RAY_DIR) -L$(RLIMGUI_DIR)
LIBS = -lr3d -lraylib -lgdi32 -lwinmm

OBJDIR = objs
OBJS = $(OBJDIR)/basic.o $(OBJDIR)/bloom.o

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CXXFLAGS) $(RAY_INC) $(IMGUI_INC) -c -o $@ $<

basic: $(OBJDIR)/basic.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

bloom: $(OBJDIR)/bloom.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

directional: $(OBJDIR)/directional.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

fog: $(OBJDIR)/fog.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

instanced: $(OBJDIR)/instanced.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

lights: $(OBJDIR)/lights.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

particles: $(OBJDIR)/particles.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

pbr: $(OBJDIR)/pbr.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

resize: $(OBJDIR)/resize.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

skybox: $(OBJDIR)/skybox.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

sponza: $(OBJDIR)/sponza.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

sprite: $(OBJDIR)/sprite.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

transparency: $(OBJDIR)/transparency.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

tom: $(OBJDIR)/tom.o
	$(CXX) -o $@ $< $(PATH_LIBS) $(LIBS) $(IMGUILIB)

all: basic bloom directional fog instanced lights particles pbr resize skybox sponza sprite tom transparency

clean:
	rm -f $(OBJDIR)/*.o *.exe
