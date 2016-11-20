add_library(fbx      SHARED IMPORTED)
add_library(freetype SHARED IMPORTED)
add_library(glew     SHARED IMPORTED)
add_library(glfw     SHARED IMPORTED)
add_library(jpeg     SHARED IMPORTED)
add_library(png      SHARED IMPORTED)
add_library(zlib     SHARED IMPORTED)
add_library(json     SHARED IMPORTED)

if ("${CMAKE_SYSTEM}" MATCHES "Windows")
	include_directories(${CMAKE_SOURCE_DIR}/prebuilt/windows/include)

    # TODO: probably a more concise way to do this
    set_target_properties(fbx  PROPERTIES IMPORTED_IMPLIB_DEBUG          ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/libfbxsdk.lib)
    set_target_properties(fbx  PROPERTIES IMPORTED_IMPLIB_RELEASE        ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/libfbxsdk.lib)
    set_target_properties(fbx  PROPERTIES IMPORTED_IMPLIB_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/libfbxsdk.lib)

    set_target_properties(glew PROPERTIES IMPORTED_IMPLIB_DEBUG          ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/glew32.lib)
    set_target_properties(glew PROPERTIES IMPORTED_IMPLIB_RELEASE        ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/glew32.lib)
    set_target_properties(glew PROPERTIES IMPORTED_IMPLIB_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/glew32.lib)

    set_target_properties(glfw PROPERTIES IMPORTED_IMPLIB_DEBUG          ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/glfw3dll.lib)
    set_target_properties(glfw PROPERTIES IMPORTED_IMPLIB_RELEASE        ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/glfw3dll.lib)
    set_target_properties(glfw PROPERTIES IMPORTED_IMPLIB_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/glfw3dll.lib)

    set_target_properties(jpeg PROPERTIES IMPORTED_IMPLIB_DEBUG          ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/jpeg.lib)
    set_target_properties(jpeg PROPERTIES IMPORTED_IMPLIB_RELEASE        ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/jpeg.lib)
    set_target_properties(jpeg PROPERTIES IMPORTED_IMPLIB_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/jpeg.lib)

    set_target_properties(png  PROPERTIES IMPORTED_IMPLIB_DEBUG          ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/libpng16.lib)
    set_target_properties(png  PROPERTIES IMPORTED_IMPLIB_RELEASE        ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/libpng16.lib)
    set_target_properties(png  PROPERTIES IMPORTED_IMPLIB_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/libpng16.lib)

    set_target_properties(zlib PROPERTIES IMPORTED_IMPLIB_DEBUG          ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/zlib.lib)
    set_target_properties(zlib PROPERTIES IMPORTED_IMPLIB_RELEASE        ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/zlib.lib)
    set_target_properties(zlib PROPERTIES IMPORTED_IMPLIB_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/zlib.lib)

    set_target_properties(json PROPERTIES IMPORTED_IMPLIB_DEBUG          ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/lib_json.lib)
    set_target_properties(json PROPERTIES IMPORTED_IMPLIB_RELEASE        ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/lib_json.lib)
    set_target_properties(json PROPERTIES IMPORTED_IMPLIB_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/prebuilt/windows/lib/lib_json.lib)
else()
# TODO
endif()

# Install Windows shared libraries
if ("${CMAKE_SYSTEM}" MATCHES "Windows")
    install(
        CODE "file(GLOB    _DLLS_ \"${CMAKE_SOURCE_DIR}/prebuilt/windows/bin/*.dll\")"
        CODE "file(INSTALL \${_DLLS_} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)"
        CONFIGURATIONS Debug Release RelWithDebInfo MinSizeRel
    )
endif()
