function(NEW_TEST ...)
	string(REGEX MATCH "^(.*/)*(.*)\\.[^.]*$" dummy ${ARGV0})
	set(TEST_NAME ${CMAKE_MATCH_2})

	add_executable(${TEST_NAME} Common/SayonaraTest.cpp Common/PlayManagerMock.h ${ARGV} ${RESOURCES_RCC})
	target_link_libraries(${TEST_NAME}
		sayonara_components
		Qt5::Test
	)

	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} -o "${TEST_NAME}.out")
	message("Add test ${TEST_NAME}")
endfunction()
