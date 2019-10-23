message("Use gstreamer libraries")
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

set(GSTREAMER_PLUGIN_LIBRARIES "gst-plugin-scanner")
foreach(GST_LIB ${GSTREAMER_PLUGIN_LIBRARY_NAMES})
	set(GST_LIB_VAR "LIB_${GST_LIB}")

	find_library(${GST_LIB_VAR} ${GST_LIB} PATH_SUFFIXES /gstreamer-1.0)
	set(FULLPATH ${${GST_LIB_VAR}})

	if(FULLPATH)
		set(GSTREAMER_PLUGIN_LIBRARIES
			${FULLPATH}
			${GSTREAMER_PLUGIN_LIBRARIES}
		)
	endif()
endforeach()

