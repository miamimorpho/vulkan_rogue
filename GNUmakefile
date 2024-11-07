NAME = rogue
CFLAGS = -g -Wall -Wextra -Wpedantic -Wno-nullability-extension -fdiagnostics-color=always
INCLUDE = -I/usr/local/include -I/usr/X11R6/include
LDFLAGS = -L/usr/local/lib
LDADD = -lglfw -lstdc++ -lpthread -lvulkan -lm

SOURCES != ls src/*.c
OBJS = $(patsubst src/%.c,obj/%.o,$(SOURCES))

all:${NAME}

obj/%.o:src/%.c
	cc ${CFLAGS} ${INCLUDE} -c $< -o $@

extern/vma.o:extern/vk_mem_alloc.h
	c++ -O0 -Wno-nullability-completeness -c extern/vma.cpp -o $@ -I/usr/local/include

${NAME}: ${OBJS} extern/vma.o shaders/vert.spv shaders/frag.spv
	cc ${OBJS} extern/vma.o -o ${NAME} ${LDFLAGS} ${LDADD}

shaders/vert.spv:shaders/shader.vert
	glslangValidator -V shaders/shader.vert -o shaders/vert.spv 

shaders/frag.spv:shaders/shader.frag
	glslangValidator -V shaders/shader.frag -o shaders/frag.spv 

clean:
	rm -f ${NAME} ${OBJS} extern/vma.o

.PHONY: clean
