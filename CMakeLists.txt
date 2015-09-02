Project (TurningPolyhedronReloaded)
CMake_Minimum_Required (VERSION 2.6)
Set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix for Debug mode, usually d on windows")

Find_Package (SDL2)
# Find_Package (Freetype REQUIRED)
Find_Package (OSG REQUIRED)
Find_Package (OpenGL REQUIRED)

Set (MY_OPENGL_INCLUDE_DIR ${OPENGL_gl_INCLUDE_DIR})
Set (MY_OPENGL_LIBRARY ${OPENGL_gl_LIBRARY})

Include_Directories(
	${CMAKE_SOURCE_DIR}
	${MY_OPENGL_INCLUDE_DIR}
#	${SDL2_INCLUDE_DIR}
#	${FREETYPE_INCLUDE_DIRS}
	${OSG_INCLUDE_DIR}
)

File (GLOB SOURCES ${PROJECT_SOURCE_DIR}/jni/src/*.cpp)
File (GLOB HEADERS ${PROJECT_SOURCE_DIR}/jni/src/*.h)

Add_Executable (TurningPolyhedronReloaded ${SOURCES} ${HEADERS})

Target_Link_Libraries (
	TurningPolyhedronReloaded
	${MY_OPENGL_LIBRARY}
#	${SDL2_LIBRARY}
#	${FREETYPE_LIBRARIES}
	${OSG_LIBRARY} ${OSGGA_LIBRARY} ${OSGUTIL_LIBRARY} ${OSGDB_LIBRARY}
	${OSGTEXT_LIBRARY} ${OSGVIEWER_LIBRARY} ${OPENTHREADS_LIBRARY}
	${OSGFX_LIBRARY} ${OSGSHADOW_LIBRARY} ${OSGPARTICLE_LIBRARY}
)


