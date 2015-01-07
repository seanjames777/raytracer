cmake_minimum_required(VERSION 2.6)
project(raytracer)

include_directories(include/)

# TODO: -g
set(CMAKE_CXX_FLAGS "-std=c++11 -fno-exceptions -g -fno-rtti -O3 -Wno-comment -Wno-unused-value") #TODO exceptions
set(CMAKE_CXX_FLAGS "-I/usr/local/include/ -I/opt/local/include/ -I/opt/local/include/OpenEXR/ -I${CMAKE_CURRENT_SOURCE_DIR}/external/fbx/include/ ${CMAKE_CXX_FLAGS}")

#CC=clang++
#CFLAGS=-O3 -g -Iinclude/ -I/opt/local/include/ -I/opt/local/include/OpenEXR -Iexternal/fbx/include/ -std=c++11 -stdlib=libc++ -Wno-unused-value -Wno-comment
#LDFLAGS=-O3 -g -std=c++11 -stdlib=libc++ -L/opt/local/lib/ -Lexternal/fbx/lib/Mac/ -lfbxsdk -lglfw -lilmimf -framework Foundation -framework OpenGL -framework Cocoa

add_executable(raytracer
	src/camera.cpp
	src/light.cpp
	src/pointlight.cpp
	src/directionallight.cpp
	src/polygon.cpp
	src/rtmath.cpp
	src/material.cpp
	src/kdtree.cpp
	src/scene.cpp
	src/image.cpp
	src/main.cpp
	src/raytracer.cpp
	src/glimagedisplay.cpp
	src/timer.cpp
	src/fbxloader.cpp
	src/raytracersettings.cpp
    src/path.cpp
    src/bmpimage.cpp
)

target_link_libraries(raytracer
    -L${CMAKE_CURRENT_SOURCE_DIR}/external/fbx/lib/Mac/
    -L/opt/local/lib/
    -lfbxsdk
    -lglfw
    -lilmimf
    -framework\ OpenGL
)

install(TARGETS raytracer DESTINATION bin)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/external/fbx/lib/Mac/libfbxsdk.dylib" DESTINATION bin)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/content/" DESTINATION bin/content)

#bin/libfbxsdk.dylib: bin/
#	cp ./external/fbx/lib/Mac/libfbxsdk.dylib bin/

#encode:
#	ffmpeg -y -i output/frame%d.bmp -c:v libx264 -preset slow -crf 18 -r 30 -pix_fmt yuv420p output/out.mp4
