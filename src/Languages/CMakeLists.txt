project(sayonara_languages)

file(GLOB TS_FILES . *.ts)

if(WIN32)
	set(TRANSLATION_TARGET_PATH
		share/translations
	)
else()
	set(TRANSLATION_TARGET_PATH
		share/sayonara/translations
	)
endif()

qt5_add_translation(QM_FILES ${TS_FILES})
install(DIRECTORY ./icons
	DESTINATION ${TRANSLATION_TARGET_PATH}
	FILES_MATCHING PATTERN "*.png"
)

add_library(${PROJECT_NAME} STATIC ${QM_FILES})

install(FILES ${QM_FILES} DESTINATION ${TRANSLATION_TARGET_PATH})
