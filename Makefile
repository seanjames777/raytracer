CC=g++
CFLAGS=-O0 -g -Iinclude/ -I/opt/local/include/ -Iexternal/fbx/include/ -std=c++11 -stdlib=libc++ -Wno-unused-value -Wno-comment
LDFLAGS=-O0 -g -std=c++11 -stdlib=libc++ -L/opt/local/lib/ -Lexternal/fbx/lib/Mac/ -lfbxsdk -lglfw -framework Foundation -framework OpenGL -framework Cocoa

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
	src/glimagedisplay.cpp \
	src/timer.cpp \
	src/fbxloader.cpp

OBJS=$(patsubst src/%.cpp, obj/%.o, $(SOURCES))
DEPS=$(patsubst src/%.cpp, obj/%.d, $(SOURCES))

bin/raytracer: bin/ obj/ $(OBJS) bin/libfbxsdk.dylib
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

bin/libfbxsdk.dylib: bin/
	cp ./external/fbx/lib/Mac/libfbxsdk.dylib bin/

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