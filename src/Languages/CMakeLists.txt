project(sayonara_languages)

file(GLOB TS_FILES . *.ts)
file(GLOB QT_QM_FILES . *.qm)

qt5_add_translation(QM_FILES ${TS_FILES})

set(QM_FILES
	${QM_FILES}
	${QT_QM_FILES}
)

if(WIN32)
	set(TRANSLATION_TARGET_PATH
		share/translations
	)
else()
	set(TRANSLATION_TARGET_PATH
		share/sayonara/translations
	)
endif()


add_custom_target(TRANSLATIONS_TARGET DEPENDS ${QM_FILES})
install(DIRECTORY ./icons DESTINATION ${TRANSLATION_TARGET_PATH} FILES_MATCHING
    PATTERN "*.png"
    PATTERN "*.svn" EXCLUDE
)

add_library(${PROJECT_NAME} STATIC ${QM_FILES})


install(FILES ${QM_FILES} DESTINATION ${TRANSLATION_TARGET_PATH})
