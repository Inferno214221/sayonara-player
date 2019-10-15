project(say_resources)

# create sayonara menu file
# menu file not needed anymore since .desktop file exists
# configure_file(sayonara.menu.in
#	${CMAKE_CURRENT_BINARY_DIR}/sayonara
#	@ONLY
# )

# manpage
find_program(GZIP_BINARY
	NAMES gzip	
)

if(NOT GZIP_BINARY)
    message("gzip not found. No manpage will be created")
else()
    ## create date for manpage
    execute_process(COMMAND env LC_TIME="en_US.utf8" date "+%d %B %Y" OUTPUT_VARIABLE SAYONARA_MANPAGE_DATE)
    string(REGEX REPLACE "\n$" "" SAYONARA_MANPAGE_DATE "${SAYONARA_MANPAGE_DATE}")
    
    ## create manpage
    configure_file(sayonara.1.man.in 
    	${CMAKE_CURRENT_BINARY_DIR}/sayonara.1 
    	@ONLY
    )

    ## create zipped manpage
    execute_process(COMMAND gzip ${CMAKE_CURRENT_BINARY_DIR}/sayonara.1 -k)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sayonara.1.gz" DESTINATION man/man1)
endif()

install(FILES "sayonara.desktop" DESTINATION share/applications)
install(FILES "sayonara.appdata.xml" DESTINATION share/appdata)
install(DIRECTORY "logos/" DESTINATION share/icons/hicolor)
