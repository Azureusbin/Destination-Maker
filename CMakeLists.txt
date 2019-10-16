cmake_minimum_required(VERSION 3.10.0)

SET(CMAKE_CONFIGURATION_TYPES "Debug;Release;")

PROJECT(DestinationMaker)

add_definitions(-D _CRT_NONSTDC_NO_DEPRECATE)
add_definitions(-D _CRT_SECURE_NO_WARNINGS)

find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Gui)

SET(CMAKE_AUTOMOC ON)

FILE(GLOB CPP_FILES "./*.cpp")
FILE(GLOB HEADER_FILES "./*.h")
FILE(GLOB UI_FILES "./*.ui")
FILE(GLOB QRC_SOURCE_FILES "./*.qrc")
FILE(GLOB RC_FILES "./DestinationMaker.rc" "./DestinationMaker.ico" "./PATH.ico")
FILE(GLOB DEST_HDR_FILES "DestSystem/*.h")
FILE(GLOB DEST_SRC_FILES "DestSystem/*.cpp")

#ui files handle
qt5_wrap_ui(WRAP_UI_FILES ${UI_FILES})
#QT5_WRAP_CPP(WRAP_CPP_FILES CMainWindow.h)
qt5_add_resources(WRAP_QRC_FILES ${QRC_SOURCE_FILES})

source_group("Code" FILES ${CPP_FILES} ${HEADER_FILES})
source_group("UI" FILES ${UI_FILES})
source_group("GenerateFiles" FILES ${WRAP_UI_FILES} ${WRAP_QRC_FILES})
source_group("RC" FILES ${QRC_SOURCE_FILES} ${RC_FILES})
source_group("DestSystem" FILES ${DEST_HDR_FILES} ${DEST_SRC_FILES})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES("./")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/Bin)

add_executable(DMaker 
${UI_FILES} ${WRAP_UI_FILES} 
${QRC_SOURCE_FILES} ${WRAP_QRC_FILES} 
${HEADER_FILES} ${CPP_FILES} 
${DEST_HDR_FILES} ${DEST_SRC_FILES}
${RC_FILES}
)

set_target_properties(DMaker PROPERTIES 
 CMAKE_INCLUDE_CURRENT_DIR ON)
target_link_libraries(DMaker Qt5::Widgets Qt5::Core Qt5::Gui)

