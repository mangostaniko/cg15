project(seganku)
cmake_minimum_required(VERSION 2.8)

# add CMakeLists.txt source dir and cmake module dir to the cmake module path
# cmake modules are cmake files used e.g. to find library files
set(CMAKE_MODULE_PATH 
    ${CMAKE_MODULE_PATH} 
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/CMakeModules"
    ) 

# specify build dir for out-of-source build (aka shadow build)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


### COMPILER FLAGS ###
# note: add_definitions can be used as well, but is intended for preprocessor definitions

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++0x")


### EXTERNAL LIBRARIES ###

find_package(OpenGL REQUIRED)

if(MSVC)
    set(GLEW_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glew-1.1.0)
    set(GLFW_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw-3.1.1.bin.WIN32)
	set(GLM_PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glm)
	set(FreeImage_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/FreeImage)
	set(Assimp_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/assimp--3.0.1270-sdk)
else(MSVC)
	find_package(GLEW REQUIRED)
	find_package(GLFW REQUIRED)
	find_package(GLM REQUIRED)
	find_package(FreeImage REQUIRED)
	find_package(Assimp REQUIRED)
	find_package(Freetype REQUIRED)
endif(MSVC)


### SOURCE FILES ###

# relative path to every single h and cpp file
set(SRC_CLASSES 

    SEGANKU/main.cpp

	SEGANKU/shader.h
	SEGANKU/shader.cpp
	SEGANKU/texture.h
	SEGANKU/texture.cpp
	SEGANKU/textrenderer.h
	SEGANKU/textrenderer.cpp

	SEGANKU/sceneobject.h
	SEGANKU/sceneobject.cpp
	SEGANKU/camera.h
	SEGANKU/camera.cpp
	SEGANKU/light.h
	SEGANKU/light.cpp
	SEGANKU/geometry.h
	SEGANKU/geometry.cpp
	SEGANKU/surface.h
	SEGANKU/surface.cpp

	SEGANKU/player.h
	SEGANKU/player.cpp

	SEGANKU/effects/ssaopostprocessor.h
	SEGANKU/effects/ssaopostprocessor.cpp
	SEGANKU/effects/particlesystem.h
	SEGANKU/effects/particlesystem.cpp


    )
	
# relative path to shader files
set(SRC_SHADERS
		SEGANKU/shaders/textured_blinnphong.vert
		SEGANKU/shaders/textured_blinnphong.frag
		SEGANKU/shaders/normal_mapping.vert
		SEGANKU/shaders/normal_mapping.frag

		SEGANKU/shaders/text.vert
		SEGANKU/shaders/text.frag
		SEGANKU/shaders/particles.vert
		SEGANKU/shaders/particles.frag

		SEGANKU/shaders/depth_shader.vert
		SEGANKU/shaders/depth_shader.frag
		SEGANKU/shaders/quad_debug.vert
		SEGANKU/shaders/quad_debug.frag

		SEGANKU/shaders/ssao.vert
		SEGANKU/shaders/ssao.frag
		SEGANKU/shaders/blur.vert
		SEGANKU/shaders/blur.frag

		)
		
# adds an executable target with given name to be built from the source files listed afterwards
add_executable(${PROJECT_NAME} ${SRC_CLASSES} ${SRC_SHADERS})


if(MSVC)
	### INCLUDE HEADER FILES ###
	include_directories(${OPENGL_INCLUDE_DIRS} 
						${GLEW_ROOT_DIR}/include
						${GLFW_ROOT_DIR}/include
						${GLM_PACKAGE_DIR}
						${FreeImage_ROOT_DIR}/Dist/x32
						${FreeImage_ROOT_DIR}/Wrapper/FreeImagePlus/dist/x32
						${Assimp_ROOT_DIR}/include
						)

	### LINK LIBRARIES ###
	target_link_libraries(${PROJECT_NAME}
						${OPENGL_LIBRARIES}
						glew32s.lib
						glfw3.lib
						FreeImage.lib
						FreeImagePlus.lib
						assimp.lib
                        )
else(MSVC)
	### INCLUDE HEADER FILES ###
	include_directories(${OPENGL_INCLUDE_DIRS} 
						${GLEW_INCLUDE_DIRS} 
						${GLFW_INCLUDE_DIR}
						${FreeImage_INCLUDE_DIR}
						${ASSIMP_INCLUDE_DIRS}
                        ${FREETYPE_INCLUDE_DIRS}
				        )

	### LINK LIBRARIES ###
	target_link_libraries(${PROJECT_NAME}
						  ${OPENGL_LIBRARIES}
						  ${GLEW_LIBRARY}
						  ${GLFW_LIBRARY}
						  ${FreeImage_LIBRARY}
						  ${ASSIMP_LIBRARIES}
						  ${FREETYPE_LIBRARIES}
						  )
endif(MSVC)
