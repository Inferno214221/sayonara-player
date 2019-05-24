#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "Utils/Pimpl.h"
#include <gst/gst.h>

class Visualizer
{
	PIMPL(Visualizer)

public:
	Visualizer(GstElement* pipeline, GstElement* tee);
	~Visualizer();

	bool init();
	bool set_enabled(bool b);
};

#endif // VISUALIZER_H
