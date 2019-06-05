project(say_comp_converter)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

