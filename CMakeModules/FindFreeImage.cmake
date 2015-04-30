# Try to find FreeImage
# When found, this will define the following:
# - FreeImage_FOUND          system has FreeImage
# - FreeImage_INCLUDE_DIRS   the FreeImage include directories
# - FreeImage_LIBRARY        link these to use FreeImage

# the include dir
find_path(FreeImage_INCLUDE_DIR
	NAMES FreeImage.h
	HINTS $ENV{FREEIMAGE_ROOT}/include
)

# the library itself
find_library(FreeImage_LIBRARY
	NAMES freeimageplus
	HINTS $ENV{FREEIMAGE_ROOT}/lib
)

if(FreeImage_INCLUDE_DIR)
	message(STATUS "Found FreeImage: ${FreeImage_INCLUDE_DIR}")
	message(STATUS "Found FreeImage: ${FreeImage_LIBRARY}")
endif(FreeImage_INCLUDE_DIR)
