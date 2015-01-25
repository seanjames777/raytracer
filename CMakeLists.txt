cmake_minimum_required(VERSION 2.6)
project(raytracer)

include_directories(include/)

# RPath
set(CMAKE_INSTALL_RPATH "\$ORIGIN/")
set(CMAKE_INSTALL_NAME_DIR "@executable_path/")

message("Compiler: ${CMAKE_CXX_COMPILER_ID}")
message("System: ${CMAKE_SYSTEM_NAME}")

if(NOT(${CMAKE_SYSTEM_NAME} MATCHES "Windows"))
    add_definitions(-std=c++11 -fno-exceptions -fno-rtti -O3 -Wno-comment -Wno-unused-value)
	# TODO: -g
#	set(CMAKE_CXX_FLAGS "-std=c++11 -fno-exceptions -g -fno-rtti -O3 -Wno-comment -Wno-unused-value") #TODO exceptions
#	set(CMAKE_CXX_FLAGS "-I/usr/local/include/ -I/opt/local/include/ -I/opt/local/include/OpenEXR/ -I${CMAKE_CURRENT_SOURCE_DIR}/external/fbx/include/ ${CMAKE_CXX_FLAGS}")
endif()

#CC=clang++
#CFLAGS=-O3 -g -Iinclude/ -I/opt/local/include/ -I/opt/local/include/OpenEXR -Iexternal/fbx/include/ -std=c++11 -stdlib=libc++ -Wno-unused-value -Wno-comment
#LDFLAGS=-O3 -g -std=c++11 -stdlib=libc++ -L/opt/local/lib/ -Lexternal/fbx/lib/Mac/ -lfbxsdk -lglfw -lilmimf -framework Foundation -framework OpenGL -framework Cocoa

add_executable(raytracer
    src/kdtree/kdbuilder.cpp
    src/kdtree/kdmedianbuilder.cpp
    src/kdtree/kdnode.cpp
    src/kdtree/kdsahbuilder.cpp
    src/kdtree/kdtree.cpp
	src/camera.cpp
	src/light.cpp
	src/pointlight.cpp
	src/directionallight.cpp
	src/polygon.cpp
	src/rtmath.cpp
	src/material.cpp
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

	# Include files for visual studio navigation/search
	include/kdtree/kdbuilder.h
	include/kdtree/kdmedianbuilder.h
	include/kdtree/kdnode.h
	include/kdtree/kdsahbuilder.h
	include/kdtree/kdstack.h
	include/kdtree/kdtree.h
	include/math/matrix.h
	include/math/vector.h
	include/bmpimage.h
	include/camera.h
	include/directionallight.h
	include/fbxloader.h
	include/glimagedisplay.h
	include/image.h
	include/light.h
	include/material.h
	include/path.h
	include/pointlight.h
	include/polygon.h
	include/raytracer.h
	include/raytracersettings.h
	include/rtmath.h
	include/scene.h
	include/timer.h
)

add_library(glfw SHARED IMPORTED)
add_library(glew SHARED IMPORTED)
add_library(fbxsdk SHARED IMPORTED)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    include_directories(external/prebuilt_linux/include)
    set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/external/prebuilt_linux/lib/libglfw.so)
    set_target_properties(glew PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/external/prebuilt_linux/lib64/libGLEW.so)
    set_target_properties(fbxsdk PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/external/prebuilt_linux/lib/libfbxsdk.so)
    set_target_properties(fbxsdk PROPERTIES IMPORTED_NO_SONAME TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # TODO do this whole thing better
	include_directories(external/prebuilt_osx/include)
    set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/external/prebuilt_osx/lib/libglfw.dylib)
    set_target_properties(glew PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/external/prebuilt_osx/lib/libGLEW.dylib)
    set_target_properties(fbxsdk PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/external/prebuilt_osx/lib/libfbxsdk.dylib)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include_directories(external/prebuilt_windows/include/)
	set_target_properties(glfw PROPERTIES IMPORTED_IMPLIB_DEBUG ${CMAKE_SOURCE_DIR}/external/prebuilt_windows/lib/x64_debug/glfw3dll.lib)
	set_target_properties(glfw PROPERTIES IMPORTED_IMPLIB_RELEASE ${CMAKE_SOURCE_DIR}/external/prebuilt_windows/lib/x64_release/glfw3dll.lib)
    set_target_properties(glew PROPERTIES IMPORTED_IMPLIB_DEBUG ${CMAKE_SOURCE_DIR}/external/prebuilt_windows/lib/x64_debug/glew32.lib)
	set_target_properties(glew PROPERTIES IMPORTED_IMPLIB_RELEASE ${CMAKE_SOURCE_DIR}/external/prebuilt_windows/lib/x64_release/glew32.lib)
    set_target_properties(fbxsdk PROPERTIES IMPORTED_IMPLIB_DEBUG ${CMAKE_SOURCE_DIR}/external/prebuilt_windows/lib/x64_debug/libfbxsdk.lib)
	set_target_properties(fbxsdk PROPERTIES IMPORTED_IMPLIB_RELEASE ${CMAKE_SOURCE_DIR}/external/prebuilt_windows/lib/x64_release/libfbxsdk.lib)
endif()

FIND_PACKAGE(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

target_link_libraries(raytracer
	glfw
	glew
	fbxsdk
	${OPENGL_LIBRARIES}
)

install(TARGETS raytracer DESTINATION bin)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/content/" DESTINATION bin/content)

# Install shared libraries
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install(
        CODE "file(GLOB    _LIBS_ \"${CMAKE_SOURCE_DIR}/external/prebuilt_linux/lib*/*.so*\")"
        CODE "file(INSTALL \${_LIBS_} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)"
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    install(
        CODE "file(GLOB    _LIBS_ \"${CMAKE_SOURCE_DIR}/external/prebuilt_osx/lib/*.dylib\")"
        CODE "file(INSTALL \${_LIBS_} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)"
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	if (CMAKE_BUILD_TYPE MATCHES "debug")
	set(LIB_DIR "debug")
	else ()
	set(LIB_DIR "release")
	endif ()

    install(
        CODE "file(GLOB    _LIBS_ \"${CMAKE_SOURCE_DIR}/external/prebuilt_windows/lib/x64_${LIB_DIR}/*.dll\")"
        CODE "file(INSTALL \${_LIBS_} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)"
    )
endif ()

#bin/libfbxsdk.dylib: bin/
#	cp ./external/fbx/lib/Mac/libfbxsdk.dylib bin/

#encode:
#	ffmpeg -y -i output/frame%d.bmp -c:v libx264 -preset slow -crf 18 -r 30 -pix_fmt yuv420p output/out.mp4
