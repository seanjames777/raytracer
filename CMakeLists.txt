cmake_minimum_required(VERSION 2.6)
project(raytracer)

include_directories(include/)

# RPath
set(CMAKE_INSTALL_RPATH "\$ORIGIN/")
set(CMAKE_INSTALL_NAME_DIR "@executable_path/")

message("Compiler: ${CMAKE_CXX_COMPILER_ID}")
message("System: ${CMAKE_SYSTEM_NAME}")

if(NOT(${CMAKE_SYSTEM_NAME} MATCHES "Windows"))
    message("Unix")
    add_definitions(-fno-rtti -fno-exceptions -Wno-comment -Wno-unused-value -msse4.1 -std=c++11)

    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        message("Debug build")
        add_definitions(-O0 -g)
    else()
        message("Debug build")
        add_definitions(-O3)
    endif()
endif()

include(prebuilt/CMakeLists.txt)

add_library(rt SHARED
    src/kdtree/kdbuilder.cpp
    src/kdtree/kdmedianbuilder.cpp
    src/kdtree/kdnode.cpp
    src/kdtree/kdsahbuilder.cpp
    src/kdtree/kdtree.cpp
    src/util/path.cpp
    src/util/glimagedisplay.cpp
	src/camera.cpp
	src/light.cpp
	src/pointlight.cpp
	src/directionallight.cpp
	src/polygon.cpp
	src/shader.cpp
	src/scene.cpp
	src/image.cpp
	src/raytracer.cpp
	src/timer.cpp
	src/fbxloader.cpp
	src/raytracersettings.cpp
    src/bmpimage.cpp

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
    include/util/glimagedisplay.h
	include/bmpimage.h
	include/camera.h
	include/directionallight.h
	include/fbxloader.h
	include/image.h
	include/light.h
	include/shader.h
	include/pointlight.h
	include/polygon.h
	include/raytracer.h
	include/raytracersettings.h
	include/scene.h
	include/timer.h
    include/rt_defs.h
)

add_executable(raytracer src/raytracer/main.cpp)
target_link_libraries(raytracer rt)
install(TARGETS raytracer DESTINATION bin)

FIND_PACKAGE(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

target_link_libraries(rt
	glfw
	glew
	${OPENGL_LIBRARIES}
	fbx
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(rt wsock32 ws2_32)
endif()

install(TARGETS rt DESTINATION bin)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/content/" DESTINATION bin/content)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	install(FILES ${PROJECT_BINARY_DIR}/Debug/host.pdb DESTINATION bin CONFIGURATIONS Debug)
	install(FILES ${PROJECT_BINARY_DIR}/RelWithDebInfo/host.pdb DESTINATION bin CONFIGURATIONS RelWithDebInfo)
	install(FILES ${PROJECT_BINARY_DIR}/Debug/worker.pdb DESTINATION bin CONFIGURATIONS Debug)
	install(FILES ${PROJECT_BINARY_DIR}/RelWithDebInfo/worker.pdb DESTINATION bin CONFIGURATIONS RelWithDebInfo)
endif ()
