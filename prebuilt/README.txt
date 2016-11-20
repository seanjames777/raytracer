To use with CMake, simply copy prebuilt/ directory to your
top-level, and add include(prebuilt/CMakeLists.txt) to your
CMakeLists.txt

To use without CMake, simply include the include directory for
your platform, and target the static/shared libraries as usual.

Refer to Updating_Libraries.txt for instructions to build each
library on each platform if they are updated.

Included libraries:

	libjpeg
	zlib
	libpng
	freetype2
	glew
	glfw
	FBX SDK
	glm
	jsoncpp