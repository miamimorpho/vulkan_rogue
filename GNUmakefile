NAME = rogue
CFLAGS = -g -Wall -Wextra -Wpedantic -Wno-nullability-extension -fdiagnostics-color=always
INCLUDE = -I/usr/local/include -I/usr/X11R6/include -I/usr/local/include/luajit-2.1
LDFLAGS = -L/usr/local/lib
LDADD = -lglfw -lstdc++ -lpthread -lvulkan -lm -lluajit-5.1

CSRC = $(wildcard src/*.c)
COBJ = $(patsubst src/%.c,obj/%.o,$(CSRC))

VKTERM_DIR = src/vkterm
VKTERM_SRC = $(wildcard $(VKTERM_DIR)/*.c)
VKTERM_OBJ = $(patsubst $(VKTERM_DIR)/%.c,obj/%.o,$(VKTERM_SRC))
VKTERM_NAME = vktermlib.so
VKTERM_LD = -L/usr/local/lib -lglfw -lstdc++ -lpthread -lvulkan -lm

$(shell mkdir -p obj lib)

all: lib/$(VKTERM_NAME) ${NAME}

lib/$(VKTERM_NAME): $(VKTERM_OBJ)
	cc -shared -fPIC ${CFLAGS} ${INCLUDE} -o $@ $^ ${VKTERM_LD}

obj/%.o:$(VKTERM_DIR)/%.c lib/vma.o
	cc -fPIC ${CFLAGS} ${INCLUDE} -c $< -o $@ 

obj/%.o:src/%.c
	cc ${CFLAGS} ${INCLUDE} -c $< -o $@

lib/vma.o:extern/vk_mem_alloc.h
	c++ -O0 -Wno-nullability-completeness -c extern/vma.cpp -o $@ -I/usr/local/include

${NAME}: ${COBJ} lib/vma.o shaders/vert.spv shaders/frag.spv
	cc -rdynamic ${COBJ} lib/vma.o lib/${VKTERM_NAME} -o ${NAME} ${LDFLAGS} ${LDADD}

shaders/vert.spv:shaders/shader.vert
	glslangValidator -V shaders/shader.vert -o shaders/vert.spv 

shaders/frag.spv:shaders/shader.frag
	glslangValidator -V shaders/shader.frag -o shaders/frag.spv 

clean:
	rm -r obj lib
	rm -f ${NAME} shaders/*.spv

.PHONY: clean
