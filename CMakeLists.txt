project(seganku)
cmake_minimum_required(VERSION 2.8)

# add CMakeLists.txt source dir and cmake module dir to the cmake module path
# cmake modules are cmake files used e.g. to find library files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/CMakeModules") 

# specify build dir for out-of-source build (aka shadow build)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


### COMPILER FLAGS ###
# note: add_definitions can be used as well, but is intended for preprocessor definitions

set(CMAKE_CXX_FLAGS "-Wall -std=c++0x")


### EXTERNAL LIBRARIES ###

find_package(OpenGL REQUIRED)

if(MSVC)
    set(GLEW_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glew-1.1.0)
    set(GLFW_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw-3.0.4.bin.WIN32)
	set(GLM_PACKAGE_DIR ${CMAKE_SOURCE_DIR}/external/glm-0.9.5.1)
else(MSVC)
	find_package(GLEW REQUIRED)
	find_package(GLFW REQUIRED)
	find_package(GLM REQUIRED)
endif(MSVC)


### SOURCE FILES ###

# relative path to every single h and cpp file
set(SRC_CLASSES 
    src/main.cpp
	src/shader.h
	src/shader.cpp
	src/texture.h
	src/texture.cpp
	src/sceneobject.h
	src/sceneobject.cpp
    
	src/sceneobjects/cube.h
	src/sceneobjects/cube.cpp
)

# relative path to shader files
set(SRC_SHADERS
	src/shaders/testshader.vert
	src/shaders/testshader.frag
)

# adds an executable target with given name to be built from the source files listed afterwards
add_executable(${PROJECT_NAME} ${SRC_CLASSES} ${SRC_SHADERS})


### INCLUDE HEADER FILES ###

include_directories( ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})


### LINK LIBRARIES ###

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} ${GLEW_LIBRARY} ${GLFW_LIBRARY})
