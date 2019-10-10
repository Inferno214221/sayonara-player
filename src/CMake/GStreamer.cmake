set(GSTREAMER_PLUGIN_LIBRARY_NAMES
	gstaiff
	gstalsa
	gstapetag
	gstapp
	gstaudioconvert
	gstaudioconvert
	gstaudioresample
	gstcoreelements
	gstequalizer
	gstfaad
	gstflac
	gstgio
	gstid3demux
	gstid3tag
	gstlame
	gstlevel
	gstmpg123
	gstogg
	gstopus
	gstopusparse
	gstplayback
	gstproxy
	gstpulseaudio
	gstrawparse
	gstsoundtouch
	gstspectrum
	gsttaglib
	gstvolume
)

set(GSTREAMER_PLUGIN_LIBRARIES "")
foreach(GST_LIB ${GSTREAMER_PLUGIN_LIBRARY_NAMES})
	set(GST_LIB_VAR "LIB_${GST_LIB}")

	find_library(${GST_LIB_VAR} ${GST_LIB} PATH_SUFFIXES /gstreamer-1.0)
	set(FULLPATH ${${GST_LIB_VAR}})

	if(GST_LIB_FULL)
		set(GSTREAMER_PLUGIN_LIBRARIES
			${FULLPATH}
			${GSTREAMER_PLUGIN_LIBRARIES}
		)
	endif()
endforeach()


