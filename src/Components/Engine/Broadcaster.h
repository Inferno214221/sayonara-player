#ifndef BROADCASTER_H
#define BROADCASTER_H

#include "Utils/Pimpl.h"
#include <gst/gst.h>

class Broadcaster
{
	PIMPL(Broadcaster)

	public:
		Broadcaster(GstElement* pipeline, GstElement* tee);
		virtual ~Broadcaster();

		bool init();
		bool set_enabled(bool b);
};

#endif // BROADCASTER_H
