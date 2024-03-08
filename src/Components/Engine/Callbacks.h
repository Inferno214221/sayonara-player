/* EngineCallbacks.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ENGINECALLBACKS_H
#define ENGINECALLBACKS_H

#include <QString>
#include <gst/gst.h>

struct GstURIDecodeBin;
namespace Engine::Callbacks
{
	struct TrackContext
	{
		QString userAgent;
	};

	gboolean busStateChanged(GstBus* bus, GstMessage* msg, gpointer data);

	gboolean levelHandler(GstBus* bus, GstMessage* message, gpointer data);

	gboolean spectrumHandler(GstBus* bus, GstMessage* message, gpointer data);

	void decodebinReady(GstElement* src, GstPad* newSrcPad, gpointer data);

	void sourceReady(GstURIDecodeBin* bin, GstElement* source, gpointer userData);

	gboolean positionChanged(gpointer data);

	GstFlowReturn newBuffer(GstElement* sink, gpointer data);
}

#endif // ENGINECALLBACKS_H

