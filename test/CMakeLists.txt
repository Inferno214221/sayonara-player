project(say_tests)
include(${CMAKE_SOURCE_DIR}/src/CMake/Compiler.cmake)

if(POLICY CMP0071)
	cmake_policy(SET CMP0071 NEW)
endif()

find_package(Qt5 COMPONENTS Core Gui Sql Network Xml Test REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(COPY ${CMAKE_SOURCE_DIR}/resources/player.db
	DESTINATION	${CMAKE_CURRENT_BINARY_DIR}
)

set(RESOURCES
	resources/Test.qrc
	${CMAKE_CURRENT_SOURCE_DIR}/../resources/Database.qrc
	${CMAKE_CURRENT_SOURCE_DIR}/../resources/Resources.qrc
)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
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
include_directories(${CMAKE_BINARY_DIR}/src/3rdParty/taglib)

include_directories(${CMAKE_SOURCE_DIR}/test)
include_directories(${CMAKE_BINARY_DIR}/test)

include("new_test.cmake")
qt5_add_resources(RESOURCES_RCC
	${RESOURCES}
)

NEW_TEST(Database/PodcastTest.cpp ${RESOURCES_RCC})
NEW_TEST(Database/AlbumTest.cpp ${RESOURCES_RCC})

NEW_TEST(Util/CryptTest.cpp)
NEW_TEST(Util/FileHelperTest.cpp ${RESOURCES_RCC})
NEW_TEST(Util/MetaDataListTest.cpp)
NEW_TEST(Util/LanguageKeyTest.cpp)
NEW_TEST(Util/LanguageUtilTest.cpp)
NEW_TEST(Util/MetaDataTest.cpp)
NEW_TEST(Util/SettingsTest.cpp ${RESOURCES_RCC})
NEW_TEST(StreamRecorder/StreamRecorderUtilsTest.cpp)
NEW_TEST(StreamRecorder/StreamRecorderTest.cpp)
NEW_TEST(Tagging/LyricsTest.cpp ${RESOURCES_RCC} Tagging/AbstractTaggingTest.cpp)
NEW_TEST(Tagging/AlbumArtistTest.cpp ${RESOURCES_RCC} Tagging/AbstractTaggingTest.cpp)
NEW_TEST(Tagging/DiscnumberTest.cpp ${RESOURCES_RCC} Tagging/AbstractTaggingTest.cpp)
NEW_TEST(Tagging/RatingTest.cpp ${RESOURCES_RCC} Tagging/AbstractTaggingTest.cpp)
NEW_TEST(Tagging/TagFromPathTest.cpp)
NEW_TEST(Tagging/EditorTest.cpp ${RESOURCES_RCC})


