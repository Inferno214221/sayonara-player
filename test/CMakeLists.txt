project(say_tests)
include(${CMAKE_SOURCE_DIR}/src/CMake/Compiler.cmake)

if(POLICY CMP0071)
	cmake_policy(SET CMP0071 NEW)
endif()

find_package(Qt5 COMPONENTS Core Widgets Sql Network Network Xml Test REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(COPY ${CMAKE_SOURCE_DIR}/resources/player.db
	DESTINATION	${CMAKE_CURRENT_BINARY_DIR}
)

set(RESOURCES
	resources/Test.qrc
)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Xml_INCLUDE_DIRS})
include_directories(${Qt5Sql_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})
include_directories(${Qt5Test_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${QT_DEFINITIONS})

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src/3rdParty)

include_directories(${CMAKE_SOURCE_DIR}/test)
include_directories(${CMAKE_BINARY_DIR}/test)

include("new_test.cmake")
QT5_ADD_RESOURCES(RESOURCES_RCC
		${RESOURCES}
)

NEW_TEST(Database/PodcastTest.cpp)
NEW_TEST(Database/AlbumTest.cpp)

NEW_TEST(Util/CryptTest.cpp)
NEW_TEST(Util/FileHelperTest.cpp)
NEW_TEST(Util/MetaDataListTest.cpp)
NEW_TEST(Util/LanguageKeyTest.cpp)
NEW_TEST(Util/LanguageUtilTest.cpp)
NEW_TEST(Util/MetaDataTest.cpp)
NEW_TEST(Util/SettingsTest.cpp)
#NEW_TEST(Shortcuts/ShortcutIdentifierTest.cpp)
NEW_TEST(StreamRecorder/StreamRecorderUtilsTest.cpp)
NEW_TEST(StreamRecorder/StreamRecorderTest.cpp)
NEW_TEST(Tagging/LyricsTest.cpp ${RESOURCES_RCC} Tagging/AbstractTaggingTest.cpp)
NEW_TEST(Tagging/AlbumArtistTest.cpp ${RESOURCES_RCC} Tagging/AbstractTaggingTest.cpp)
NEW_TEST(Tagging/DiscnumberTest.cpp ${RESOURCES_RCC} Tagging/AbstractTaggingTest.cpp)
NEW_TEST(Tagging/RatingTest.cpp ${RESOURCES_RCC} Tagging/AbstractTaggingTest.cpp)
NEW_TEST(Tagging/TagFromPathTest.cpp)

#NEW_TEST(StreamSearchTest.cpp)


