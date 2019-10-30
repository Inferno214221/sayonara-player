function(NEW_TEST ...)

	string(REGEX MATCH "^(.*/)*(.*)\\.[^.]*$" dummy ${ARGV0})
	set(TEST_NAME ${CMAKE_MATCH_2})

	add_executable(${TEST_NAME} SayonaraTest.cpp ${ARGV})
	target_link_libraries(${TEST_NAME}
		sayonara_components
		Qt5::Test
	)

	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} -o "${TEST_NAME}.out")
	message("Add test ${TEST_NAME}")

endfunction()


function(NEW_TEST_RCC ...)

	qt5_add_resources(RESOURCES_RCC
		${RESOURCES}
	)

	string(REGEX MATCH "^(.*/)*(.*)\\.[^.]*$" dummy ${ARGV0})
	set(TEST_NAME ${CMAKE_MATCH_2})


	add_executable(${TEST_NAME} SayonaraTest.cpp ${ARGV} ${RESOURCES_RCC})
	target_link_libraries(${TEST_NAME}
		sayonara_components
		Qt5::Test
	)

	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} -o "${TEST_NAME}.out")
	target_compile_definitions(${TEST_NAME} PRIVATE "SAYONARA_TEST_WITH_RCC")
	message("Add test ${TEST_NAME}")

endfunction()


