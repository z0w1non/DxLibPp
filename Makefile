TARGET := out.exe
CXX := g++
FLAGS := -std=c++17 -Wall
COMPILE_OPTIONS := -I./include/DxLib -DDX_GCC_COMPILE -DDX_NON_INLINE_ASM
LINK_OPTIONS := -L./lib/DxLib -lDxLib -lDxUseCLib -lDxDrawFunc -ljpeg -lpng -lzlib -ltiff -ltheora_static -lvorbis_static -lvorbisfile_static -logg_static -lbulletdynamics -lbulletcollision -lbulletmath -lopusfile -lopus -lsilk_common -lcelt

.PHONY: all

all: $(TARGET)

$(TARGET): main.cpp DxLibPp.o
	$(CXX) $(FLAGS) $(LINK_OPTIONS) $(COMPILE_OPTIONS) $^ -o $(TARGET)

DxLibPp.o: DxLibPp.cpp
	$(CXX) $(FLAGS) -c $(COMPILE_OPTIONS) $^ -o $@
