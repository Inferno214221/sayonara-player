cmake_minimum_required(VERSION 2.8)
project(say_resources)

file(WRITE "${CMAKE_BINARY_DIR}/resources/sayonara" "?package(sayonara): \\
  needs=\"X11\" \\
  section=\"Applications/Sound\" \\
  title=\"Sayonara\" \\
  command=\"${CMAKE_INSTALL_PREFIX}/bin/sayonara\" \\
  icon=\"${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/32x32/apps/sayonara.xpm\"\n"
)

if(WIN32)
	install(FILES "player.db" DESTINATION share)
else()
	install(FILES "player.db" DESTINATION share/sayonara)
	install(FILES "sayonara.desktop" DESTINATION share/applications)
	#install(FILES "${CMAKE_BINARY_DIR}/resources/sayonara" DESTINATION share/menu)
	install(FILES "sayonara.appdata.xml" DESTINATION share/appdata)
	install(FILES "manpage/sayonara.1.gz" DESTINATION share/man/man1)
	install(DIRECTORY "logos/" DESTINATION share/icons/hicolor)
endif()

