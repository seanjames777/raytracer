cmake_minimum_required(VERSION 2.6)
project(raytracer)

include_directories(include/)

# RPath
set(CMAKE_INSTALL_RPATH "\$ORIGIN/")
set(CMAKE_INSTALL_NAME_DIR "@executable_path/")

message("Compiler: ${CMAKE_CXX_COMPILER_ID}")
message("System: ${CMAKE_SYSTEM_NAME}")

if(NOT(${CMAKE_SYSTEM_NAME} MATCHES "Windows"))
    #add_definitions(-std=c++11 -fno-exceptions -fno-rtti -O3 -Wno-comment -Wno-unused-value)
	# TODO: -g
	set(CMAKE_CXX_FLAGS "-msse4.1 -std=c++11 -fno-exceptions -fno-rtti -O3 -g -Wno-comment -Wno-unused-value") #TODO exceptions
#	set(CMAKE_CXX_FLAGS "-I/usr/local/include/ -I/opt/local/include/ -I/opt/local/include/OpenEXR/ -I${CMAKE_CURRENT_SOURCE_DIR}/external/fbx/include/ ${CMAKE_CXX_FLAGS}")
endif()

#CC=clang++
#CFLAGS=-O3 -g -Iinclude/ -I/opt/local/include/ -I/opt/local/include/OpenEXR -Iexternal/fbx/include/ -std=c++11 -stdlib=libc++ -Wno-unused-value -Wno-comment
#LDFLAGS=-O3 -g -std=c++11 -stdlib=libc++ -L/opt/local/lib/ -Lexternal/fbx/lib/Mac/ -lfbxsdk -lglfw -lilmimf -framework Foundation -framework OpenGL -framework Cocoa

include(prebuilt/CMakeLists.txt)

add_library(rt
    src/kdtree/kdbuilder.cpp
    src/kdtree/kdmedianbuilder.cpp
    src/kdtree/kdnode.cpp
    src/kdtree/kdsahbuilder.cpp
    src/kdtree/kdtree.cpp
    src/util/path.cpp
	src/camera.cpp
	src/light.cpp
	src/pointlight.cpp
	src/directionallight.cpp
	src/polygon.cpp
	src/material.cpp
	src/scene.cpp
	src/image.cpp
	src/raytracer.cpp
	src/timer.cpp
	src/fbxloader.cpp
	src/raytracersettings.cpp
    src/bmpimage.cpp
	src/net/client.cpp
	src/net/server.cpp
    src/net/protocol.cpp
    src/net/net.cpp

	# Include files for visual studio navigation/search
	include/kdtree/kdbuilder.h
	include/kdtree/kdmedianbuilder.h
	include/kdtree/kdnode.h
	include/kdtree/kdsahbuilder.h
	include/kdtree/kdtree.h
	include/math/matrix.h
	include/math/vector.h
    include/util/queue.h
    include/util/stack.h
    include/util/path.h
	include/bmpimage.h
	include/camera.h
	include/directionallight.h
	include/fbxloader.h
	include/image.h
	include/light.h
	include/material.h
	include/pointlight.h
	include/polygon.h
	include/raytracer.h
	include/raytracersettings.h
	include/scene.h
	include/timer.h
	include/net/protocol.h
	include/net/client.h
	include/net/server.h
    include/net/net.h
)

add_executable(worker
    src/worker/main.cpp
)

add_executable(host
    src/host/main.cpp
    src/host/glimagedisplay.cpp

    include/host/glimagedisplay.h
)

FIND_PACKAGE(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

target_link_libraries(worker
	fbx
    rt
)

target_link_libraries(host
    glfw
    glew
    ${OPENGL_LIBRARIES}
    rt
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(rt wsock32 ws2_32)
endif()

install(TARGETS worker DESTINATION bin)
install(TARGETS host DESTINATION bin)
install(TARGETS rt DESTINATION bin)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/content/" DESTINATION bin/content)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	install(FILES ${PROJECT_BINARY_DIR}/Debug/host.pdb DESTINATION bin CONFIGURATIONS Debug)
	install(FILES ${PROJECT_BINARY_DIR}/RelWithDebInfo/host.pdb DESTINATION bin CONFIGURATIONS RelWithDebInfo)
	install(FILES ${PROJECT_BINARY_DIR}/Debug/worker.pdb DESTINATION bin CONFIGURATIONS Debug)
	install(FILES ${PROJECT_BINARY_DIR}/RelWithDebInfo/worker.pdb DESTINATION bin CONFIGURATIONS RelWithDebInfo)
endif ()
