Project (TurningPolyhedronReloaded)
CMake_Minimum_Required (VERSION 2.6)
Set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(DEBUG_CONFIGURATIONS "Debug" CACHE STRING "Specify which configurations are for debugging. The value must be a semi-colon separated list of configuration names")
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix for Debug mode, usually d on windows")

Find_Package (SDL2)
# Find_Package (Freetype REQUIRED)
Find_Package (OpenSceneGraph 3.4.0 REQUIRED
	osg osgDB osgFX osgGA osgManipulator osgParticle osgShadow osgSim osgText osgUtil osgViewer osgWidget)
Find_Package (OpenGL REQUIRED)
Find_Package (MyGUI REQUIRED)

Set (MY_OPENGL_INCLUDE_DIR ${OPENGL_gl_INCLUDE_DIR})
Set (MY_OPENGL_LIBRARY ${OPENGL_gl_LIBRARY})

Include_Directories(
	${CMAKE_SOURCE_DIR}
	${MY_OPENGL_INCLUDE_DIR}
#	${SDL2_INCLUDE_DIR}
#	${FREETYPE_INCLUDE_DIRS}
	${OPENSCENEGRAPH_INCLUDE_DIRS}
	${MYGUI_INCLUDE_DIRS}
)

File (GLOB SOURCES ${PROJECT_SOURCE_DIR}/jni/src/*.cpp)
File (GLOB HEADERS ${PROJECT_SOURCE_DIR}/jni/src/*.h)

Add_Executable (TurningPolyhedronReloaded ${SOURCES} ${HEADERS})

Target_Link_Libraries (
	TurningPolyhedronReloaded
	${MY_OPENGL_LIBRARY}
#	${SDL2_LIBRARY}
#	${FREETYPE_LIBRARIES}
	${OPENSCENEGRAPH_LIBRARIES}
	${MYGUI_LIBRARIES}
	${MYGUI_OPENGL_LIBRARIES}
)


