project(say_resources)

file(WRITE "${CMAKE_BINARY_DIR}/resources/sayonara" "?package(sayonara): \\
  needs=\"X11\" \\
  section=\"Applications/Sound\" \\
  title=\"Sayonara\" \\
  command=\"${CMAKE_INSTALL_PREFIX}/bin/sayonara\" \\
  icon=\"${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/32x32/apps/sayonara.xpm\"\n"
)

install(FILES "sayonara.desktop" DESTINATION share/applications)
install(FILES "sayonara.appdata.xml" DESTINATION share/appdata)
install(FILES "manpage/sayonara.1.gz" DESTINATION share/man/man1)
install(DIRECTORY "logos/" DESTINATION share/icons/hicolor)
