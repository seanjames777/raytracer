# - Config file for the glfw package
# It defines the following variables
#   GLFW_INCLUDE_DIR, the path where GLFW headers are located
#   GLFW_LIBRARY_DIR, folder in which the GLFW library is located
#   GLFW_LIBRARY, library to link against to use GLFW

set(GLFW_INCLUDE_DIR "/home/sjames/CityGame/external/prebuilt_linux/include")
set(GLFW_LIBRARY_DIR "/home/sjames/CityGame/external/prebuilt_linux/lib")

find_library(GLFW_LIBRARY "glfw" HINTS ${GLFW_LIBRARY_DIR})
