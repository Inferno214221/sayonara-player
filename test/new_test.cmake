function(NEW_TEST ...)

	string(REGEX MATCH "^(.*/)*(.*)\\.[^.]*$" dummy ${ARGV0})
	set(TEST_NAME ${CMAKE_MATCH_2})

	add_executable(${TEST_NAME} ${ARGV})
	target_link_libraries(${TEST_NAME}
		gui_player
		gui_library
		gui_directories
		gui_plugins
		gui_preferences
		gui_somafm
		gui_soundcloud
		gui_resources

		sayonara_components
		interface_library
		Qt5::Test
	)

	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} -o "${TEST_NAME}.out")
	message("Add test ${TEST_NAME}")

endfunction()

