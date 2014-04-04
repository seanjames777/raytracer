CC=g++
CFLAGS=-O3 -Iinclude/ -I/opt/local/include/ -std=c++11 -stdlib=libc++
LDFLAGS=-O3  -std=c++11 -stdlib=libc++ -L/opt/local/lib/ -lglfw -framework Foundation -framework OpenGL -framework Cocoa

SOURCES=src/camera.cpp \
	src/light.cpp \
	src/pointlight.cpp \
	src/directionallight.cpp \
	src/shape.cpp \
	src/sphere.cpp \
	src/cube.cpp \
	src/poly.cpp \
	src/rtmath.cpp \
	src/material.cpp \
	src/kdtree.cpp \
	src/scene.cpp \
	src/bitmap.cpp \
	src/main.cpp \
	src/raytracer.cpp \
	src/glimagedisplay.cpp

OBJS=$(patsubst src/%.cpp, obj/%.o, $(SOURCES))
DEPS=$(patsubst src/%.cpp, obj/%.d, $(SOURCES))

bin/raytracer: bin/ obj/ $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

bin/:
	mkdir bin

obj/:
	mkdir obj

obj/%.o: src/%.cpp
	$(CC) $(CFLAGS) -o $@ -c -MD -MT $(patsubst src/%.cpp, obj/%.o, $<) -MF $(patsubst src/%.cpp, obj/%.d, $<) $<

-include $(DEPS)

clean:
	rm -rf bin/
	rm -rf obj/
	rm -f output.bmp