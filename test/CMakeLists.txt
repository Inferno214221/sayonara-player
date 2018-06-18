cmake_minimum_required(VERSION 2.8)
project(say_tests)
include(${CMAKE_SOURCE_DIR}/src/compiler.cmake)

find_package(Qt5 COMPONENTS Core Widgets Sql Network DBus Network Xml Test REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(RESOURCES
        resources/Test.qrc
)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Xml_INCLUDE_DIRS})
include_directories(${Qt5Sql_INCLUDE_DIRS})
include_directories(${Qt5DBus_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})
include_directories(${Qt5Test_INCLUDE_DIRS})
include_directories("${CURRENT_SOURCE_DIR}/../src")

add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${QT_DEFINITIONS})

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src)

include_directories(${CMAKE_SOURCE_DIR}/test)
include_directories(${CMAKE_BINARY_DIR}/test)

include("new_test.cmake")
QT5_ADD_RESOURCES(RESOURCES_RCC
        ${RESOURCES}
)

NEW_TEST(Helper/FileHelperTest.cpp)
NEW_TEST(Helper/MetaDataListTest.cpp)
NEW_TEST(Helper/LanguageKeyTest.cpp)
NEW_TEST(Helper/MetaDataTest.cpp)
NEW_TEST(Tagging/LyricsTest.cpp ${RESOURCES_RCC} Tagging/AbstractTaggingTest.cpp)
NEW_TEST(Tagging/AlbumArtistTest.cpp ${RESOURCES_RCC} Tagging/AbstractTaggingTest.cpp)
NEW_TEST(Tagging/DiscnumberTest.cpp ${RESOURCES_RCC} Tagging/AbstractTaggingTest.cpp)
NEW_TEST(StreamRecorder/StreamRecorderUtilsTest.cpp)
NEW_TEST(StreamRecorder/StreamRecorderTest.cpp)

