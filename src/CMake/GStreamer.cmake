include(FindPkgConfig)
message("GStreamer.cmake: Use gstreamer libraries")
set(GSTREAMER_PLUGIN_LIBRARY_NAMES
	gstaiff
	gstalsa
	gst
	gstapetag
	gstapp
	gstasf
	gstaudioconvert
	gstaudioparsers
	gstaudioresample
	gstautoconvert
	gstautodetect
	gstcoreelements
	gstdashdemux
	gstequalizer
	gstfaad
	gstflac
	gstgio
	gsticydemux
	gstid3demux
	gstid3tag
	gstisomp4
	gstlame
	gstlevel
	gstlibav
	gstmatroska
	gstmpg123
	gstogg
	gstopus
	gstopusparse
	gstplayback
	gstpulse
	gstrawparse
	gstsmoothstreaming
	gstsoundtouch
	gstspectrum
	gsttaglib
	gsttypefindfunctions
	gstvolume
	gstvorbis
	gstwavpack
	gstwavparse
	gstxingmux
)

pkg_get_variable(GSTREAMER_PLUGIN_DIR gstreamer-1.0 pluginsdir)
pkg_get_variable(GSTREAMER_LIB_DIR gstreamer-1.0 libdir)

find_path(GST_SCANNER_PATH gst-plugin-scanner 
	HINTS ${GSTREAMER_LIB_DIR}/gstreamer1.0/gstreamer-1.0
)

set(GSTREAMER_PLUGIN_LIBRARIES "")
if(GST_SCANNER_PATH)
	message("GStreamer.cmake: Found scanner in ${GST_SCANNER_PATH}")
	set(GSTREAMER_PLUGIN_LIBRARIES ${GST_SCANNER_PATH}/gst-plugin-scanner)
else()
	message("GStreamer.cmake: Could not find scanner path")
endif()

foreach(GST_LIB ${GSTREAMER_PLUGIN_LIBRARY_NAMES})
	set(GST_LIB_VAR "LIB_${GST_LIB}")

	find_library(${GST_LIB_VAR} ${GST_LIB} HINTS ${GSTREAMER_PLUGIN_DIR})
	set(FULLPATH ${${GST_LIB_VAR}})

	if(FULLPATH)
		set(GSTREAMER_PLUGIN_LIBRARIES
			${FULLPATH}
			${GSTREAMER_PLUGIN_LIBRARIES}
		)
	endif()
endforeach()

message("GStreamer.cmake: Found ${GSTREAMER_PLUGIN_LIBRARIES}")
