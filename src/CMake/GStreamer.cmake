set(GSTREAMER_PLUGIN_LIBRARY_NAMES
	gstaiff
	gstalsa
	gstapetag
	gstapp
	gstasf
	gstaudioconvert
	gstaudioparsers
	gstaudioresample
	gstautoconvert
	gstautodetect
	gstcoreelements
	gstequalizer
	gstfaad
	gstflac
	gstgio
	gsticydemux
	gstid3demux
	gstid3tag
	gstlame
	gstlevel
	gstlibav
	gstmpg123
	gstogg
	gstopus
	gstopusparse
	gstplayback
	gstpulse
	gstrawparse
	gstsoundtouch
	gstspectrum
	gsttaglib
	gsttypefindfunctions
	gstvolume
	gstwavpack
	gstwavparse
	gstxingmux
)

set(GSTREAMER_PLUGIN_LIBRARIES "")
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


