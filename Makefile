# Vector Pac-Man - MinGW cross-compile Makefile (WSL -> Windows .exe)
#
# Prerequisites:
#   - WSL with x86_64-w64-mingw32-g++  (sudo apt install mingw-w64)
#   - SDL2 MinGW dev libs in SDL2/ subdirectory
#
# Usage:
#   make              (SDL/OpenGL build)
#   make clean

CXX      = x86_64-w64-mingw32-g++

CXXFLAGS = -std=c++17 -O2 -Wall \
           -Wno-unused-variable -Wno-unused-but-set-variable \
           -ISDL2/include

LDFLAGS  = -mwindows -static-libgcc -static-libstdc++ -static

LIBS     = -LSDL2/lib \
           -lmingw32 -lSDL2main -lSDL2 \
           -lopengl32 -lglu32 \
           -lole32 -loleaut32 -limm32 -lwinmm -lgdi32 \
           -lversion -lsetupapi -lcfgmgr32

SRCS = \
	main.cpp \
	renderer.cpp \
	renderer_dvg.cpp \
	settings.cpp \
	sound.cpp \
	game.cpp \
	entity.cpp \
	entityPacman.cpp \
	entityGhost.cpp \
	entityBlinky.cpp \
	entityPinky.cpp \
	entityInky.cpp \
	entityClyde.cpp \
	entityFruit.cpp \
	enemies.cpp \
	players.cpp \
	controls.cpp \
	scene.cpp

OBJDIR = obj_win
OBJS   = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

TARGET = pacman.exe

all: $(OBJDIR) $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean
