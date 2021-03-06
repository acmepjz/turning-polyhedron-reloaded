Project (TurningPolyhedronReloaded)
CMake_Minimum_Required (VERSION 2.6)
Set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(DEBUG_CONFIGURATIONS "Debug" CACHE STRING "Specify which configurations are for debugging. The value must be a semi-colon separated list of configuration names")
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix for Debug mode, usually d on windows")

add_subdirectory(3rdparty/lzmasdk)

Find_Package (SDL2)
# Find_Package (Freetype REQUIRED)
Find_Package (OpenSceneGraph 3.4.0 REQUIRED
	osg osgDB osgFX osgGA osgManipulator osgParticle osgShadow osgSim osgText osgUtil osgViewer osgWidget)
Find_Package (OpenGL REQUIRED)

# important note: we need to use a customized version of MyGUI: https://github.com/acmepjz/mygui/tree/experimental
Find_Package (MyGUI REQUIRED)

Set (MY_OPENGL_INCLUDE_DIR ${OPENGL_gl_INCLUDE_DIR})
Set (MY_OPENGL_LIBRARY ${OPENGL_gl_LIBRARY})

Include_Directories(
#	${CMAKE_SOURCE_DIR}
	${MY_OPENGL_INCLUDE_DIR}
#	${SDL2_INCLUDE_DIR}
#	${FREETYPE_INCLUDE_DIRS}
	${OPENSCENEGRAPH_INCLUDE_DIRS}
	${MYGUI_INCLUDE_DIR}
	3rdparty
)

File (GLOB SOURCES ${PROJECT_SOURCE_DIR}/jni/src/*.cpp)
File (GLOB HEADERS ${PROJECT_SOURCE_DIR}/jni/src/*.h)

SOURCE_GROUP("Header Files\\util" FILES
	${PROJECT_SOURCE_DIR}/jni/src/CompressionManager.h
	${PROJECT_SOURCE_DIR}/jni/src/LZSS.h
	${PROJECT_SOURCE_DIR}/jni/src/LZMA.h
	${PROJECT_SOURCE_DIR}/jni/src/Rect.h
	${PROJECT_SOURCE_DIR}/jni/src/u8file.h
	${PROJECT_SOURCE_DIR}/jni/src/util_err.h
	${PROJECT_SOURCE_DIR}/jni/src/util_filesystem.h
	${PROJECT_SOURCE_DIR}/jni/src/util_misc.h
	${PROJECT_SOURCE_DIR}/jni/src/util_object.h
	${PROJECT_SOURCE_DIR}/jni/src/XMLReaderWriter.h
)

SOURCE_GROUP("Source Files\\util" FILES
	${PROJECT_SOURCE_DIR}/jni/src/CompressionManager.cpp
	${PROJECT_SOURCE_DIR}/jni/src/LZSS.cpp
	${PROJECT_SOURCE_DIR}/jni/src/LZMA.cpp
	${PROJECT_SOURCE_DIR}/jni/src/u8file.cpp
	${PROJECT_SOURCE_DIR}/jni/src/util_filesystem.cpp
	${PROJECT_SOURCE_DIR}/jni/src/util_misc.cpp
	${PROJECT_SOURCE_DIR}/jni/src/XMLReaderWriter.cpp
)

SOURCE_GROUP("Header Files\\gfx" FILES
	${PROJECT_SOURCE_DIR}/jni/src/Appearance.h
	${PROJECT_SOURCE_DIR}/jni/src/AppearanceMap.h
	${PROJECT_SOURCE_DIR}/jni/src/SimpleGeometry.h
)

SOURCE_GROUP("Source Files\\gfx" FILES
	${PROJECT_SOURCE_DIR}/jni/src/Appearance.cpp
	${PROJECT_SOURCE_DIR}/jni/src/AppearanceMap.cpp
	${PROJECT_SOURCE_DIR}/jni/src/SimpleGeometry.cpp
)

SOURCE_GROUP("Header Files\\game" FILES
	${PROJECT_SOURCE_DIR}/jni/src/EventAction.h
	${PROJECT_SOURCE_DIR}/jni/src/EventDescription.h
	${PROJECT_SOURCE_DIR}/jni/src/EventHandler.h
	${PROJECT_SOURCE_DIR}/jni/src/Interaction.h
	${PROJECT_SOURCE_DIR}/jni/src/Level.h
	${PROJECT_SOURCE_DIR}/jni/src/LevelCollection.h
	${PROJECT_SOURCE_DIR}/jni/src/MapData.h
	${PROJECT_SOURCE_DIR}/jni/src/MapPosition.h
	${PROJECT_SOURCE_DIR}/jni/src/ObjectType.h
	${PROJECT_SOURCE_DIR}/jni/src/Polyhedron.h
	${PROJECT_SOURCE_DIR}/jni/src/PolyhedronAnimation.h
	${PROJECT_SOURCE_DIR}/jni/src/PolyhedronMerge.h
	${PROJECT_SOURCE_DIR}/jni/src/PolyhedronPosition.h
	${PROJECT_SOURCE_DIR}/jni/src/TileProperty.h
	${PROJECT_SOURCE_DIR}/jni/src/TileType.h
)

SOURCE_GROUP("Source Files\\game" FILES
	${PROJECT_SOURCE_DIR}/jni/src/EventAction.cpp
	${PROJECT_SOURCE_DIR}/jni/src/EventDescription.cpp
	${PROJECT_SOURCE_DIR}/jni/src/EventHandler.cpp
	${PROJECT_SOURCE_DIR}/jni/src/Interaction.cpp
	${PROJECT_SOURCE_DIR}/jni/src/Level.cpp
	${PROJECT_SOURCE_DIR}/jni/src/LevelCollection.cpp
	${PROJECT_SOURCE_DIR}/jni/src/MapData.cpp
	${PROJECT_SOURCE_DIR}/jni/src/MapPosition.cpp
	${PROJECT_SOURCE_DIR}/jni/src/ObjectType.cpp
	${PROJECT_SOURCE_DIR}/jni/src/Polyhedron.cpp
	${PROJECT_SOURCE_DIR}/jni/src/PolyhedronAnimation.cpp
	${PROJECT_SOURCE_DIR}/jni/src/PolyhedronMerge.cpp
	${PROJECT_SOURCE_DIR}/jni/src/PolyhedronPosition.cpp
	${PROJECT_SOURCE_DIR}/jni/src/TileProperty.cpp
	${PROJECT_SOURCE_DIR}/jni/src/TileType.cpp
)

SOURCE_GROUP("Header Files\\mygui" FILES
	${PROJECT_SOURCE_DIR}/jni/src/DropdownListButton.h
	${PROJECT_SOURCE_DIR}/jni/src/BaseLayout.h
	${PROJECT_SOURCE_DIR}/jni/src/FileDialog.h
	${PROJECT_SOURCE_DIR}/jni/src/InputBox.h
	${PROJECT_SOURCE_DIR}/jni/src/MessageBox.h
	${PROJECT_SOURCE_DIR}/jni/src/MYGUIAccelerator.h
	${PROJECT_SOURCE_DIR}/jni/src/MYGUIManager.h
)

SOURCE_GROUP("Source Files\\mygui" FILES
	${PROJECT_SOURCE_DIR}/jni/src/DropdownListButton.cpp
	${PROJECT_SOURCE_DIR}/jni/src/BaseLayout.cpp
	${PROJECT_SOURCE_DIR}/jni/src/FileDialog.cpp
	${PROJECT_SOURCE_DIR}/jni/src/InputBox.cpp
	${PROJECT_SOURCE_DIR}/jni/src/MessageBox.cpp
	${PROJECT_SOURCE_DIR}/jni/src/MYGUIAccelerator.cpp
	${PROJECT_SOURCE_DIR}/jni/src/MYGUIManager.cpp
)

Add_Executable (TurningPolyhedronReloaded ${SOURCES} ${HEADERS})

Target_Link_Libraries (
	TurningPolyhedronReloaded
	${MY_OPENGL_LIBRARY}
#	${SDL2_LIBRARY}
#	${FREETYPE_LIBRARIES}
	${OPENSCENEGRAPH_LIBRARIES}
	${MYGUI_LIBRARIES}
	${MYGUI_OPENGL_LIBRARIES}
	lzmasdk
)

if(MSVC)
    set_property(TARGET TurningPolyhedronReloaded APPEND PROPERTY LINK_FLAGS /FORCE:MULTIPLE)
endif()

