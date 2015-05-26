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
        message("Release build")
        add_definitions(-O3)
    endif()
endif()

include(prebuilt/CMakeLists.txt)

add_library(rt SHARED
    src/core/camera.cpp
    src/core/camera.cpp
    src/core/raytracer.cpp
    src/core/raytracersettings.cpp
    src/core/scene.cpp
    src/core/triangle.cpp
    src/image/bmpimage.cpp
    src/image/image.cpp
    src/kdtree/kdbuilder.cpp
    src/kdtree/kdmedianbuilder.cpp
    src/kdtree/kdnode.cpp
    src/kdtree/kdsahbuilder.cpp
    src/kdtree/kdtree.cpp
    src/light/directionallight.cpp
    src/light/light.cpp
    src/light/pointlight.cpp
    src/shader/pbrshader.cpp
    src/shader/phongshader.cpp
    src/shader/shader.cpp
    src/util/fbxloader.cpp
    src/util/glimagedisplay.cpp
    src/util/path.cpp
    src/util/timer.cpp

    include/core/camera.h
    include/core/raybuffer.h
    include/core/raytracer.h
    include/core/raytracersettings.h
    include/core/scene.h
    include/core/triangle.h
    include/image/bmpimage.h
    include/image/image.h
    include/kdtree/kdbuilder.h
    include/kdtree/kdmedianbuilder.h
    include/kdtree/kdnode.h
    include/kdtree/kdsahbuilder.h
    include/kdtree/kdtree.h
    include/light/directionallight.h
    include/light/light.h
    include/light/pointlight.h
    include/math/aabb.h
    include/math/macro.h
    include/math/matrix.h
    include/math/ray.h
    include/math/sampling.h
    include/math/vector.h
    include/rt_defs.h
    include/shader/pbrshader.h
    include/shader/phongshader.h
    include/shader/shader.h
    include/util/align.h
    include/util/fbxloader.h
    include/util/glimagedisplay.h
    include/util/path.h
    include/util/queue.h
    include/util/stack.h
    include/util/timer.h
)

add_executable(raytracer src/main.cpp)
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
