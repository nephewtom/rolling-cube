.DEFAULT_GOAL := all

CC = gcc
AR = ar

SRC = src
R3D_LIB = $(SRC)/libr3d.a
DETAILS = $(SRC)/details
EMBED = $(SRC)/embedded

CFLAGS = -DGRAPHICS_API_OPENGL_33 -DPLATFORM_DESKTOP -std=gnu99
CFLAGS += -I./src -I./src/details -I../raylib/src -I../raylib/src/external

SOURCES0 = r3d_environment.c r3d_particles.c r3d_lighting.c r3d_culling.c r3d_skybox.c r3d_curves.c r3d_sprite.c r3d_utils.c r3d_state.c r3d_core.c
SOURCES0 := $(addprefix $(SRC)/, $(SOURCES0))

SOURCES1 = r3d_shaders.c r3d_textures.c
SOURCES1 := $(addprefix $(EMBED)/, $(SOURCES1))

SOURCES2 = r3d_projection.c r3d_primitives.c r3d_billboard.c r3d_collision.c r3d_drawcall.c r3d_frustum.c r3d_light.c
SOURCES2 := $(addprefix $(DETAILS)/, $(SOURCES2))

SOURCES = $(SOURCES0) $(SOURCES1) $(SOURCES2)

# $(info SOURCES = $(SOURCES))

OBJDIR = objs
OBJS = $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.c=.o)))

# $(info OBJS = $(OBJS))


$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o:$(SRC)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/%.o:$(EMBED)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/%.o:$(DETAILS)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJDIR) $(R3D_LIB)
	@echo "Building $(R3D_LIB)"

$(R3D_LIB): $(OBJS)
	$(AR) qc $@ $^
	ranlib.exe $(R3D_LIB)

clean:
	rm -f $(R3D_LIB) $(OBJDIR)/*.o

