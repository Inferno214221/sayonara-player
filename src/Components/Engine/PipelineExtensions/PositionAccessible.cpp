/* SeekHandler.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "PositionAccessible.h"
#include "Components/Engine/EngineUtils.h"
#include "Utils/Logger/Logger.h"

#include <gst/gst.h>
#include <gst/base/gstbasesrc.h>

using namespace PipelineExtensions;

namespace Seek
{
	const GstSeekFlags SeekAccurate=static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE);
	const GstSeekFlags SeekNearest=static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE);

	bool seek(GstElement* audioSource, GstSeekFlags flags, NanoSeconds ns)
	{
		if(!audioSource){
			return false;
		}

		bool success = gst_element_seek_simple (
					audioSource,
					GST_FORMAT_TIME,
					flags,
					ns);

		if(!success)
		{
			spLog(Log::Warning, "SeekHandler") << "seeking not possible";
		}

		return success;
	}

	bool seekAccurate(GstElement* audioSource, NanoSeconds ns)
	{
		return seek(audioSource, SeekAccurate, ns);
	}

	bool seekNearest(GstElement* audioSource, NanoSeconds ns)
	{
		return seek(audioSource, SeekNearest, ns);
	}
}

PositionAccessible::~PositionAccessible() = default;

NanoSeconds PositionAccessible::seekRelative(double percent, NanoSeconds refNs)
{
	NanoSeconds newTimeNs;

	if (percent > 1.0){
		newTimeNs = refNs;
	}

	else if(percent < 0) {
		newTimeNs = 0;
	}

	else {
		newTimeNs = (percent * refNs); // nsecs
	}

	if(Seek::seekAccurate(positionElement(), newTimeNs) ) {
		return newTimeNs;
	}

	return 0;
}

NanoSeconds PositionAccessible::seekAbsolute(NanoSeconds ns)
{
	if( Seek::seekAccurate(positionElement(), ns) ) {
		return ns;
	}

	return 0;
}

NanoSeconds PositionAccessible::seekNearest(NanoSeconds ns)
{
	if( Seek::seekNearest(positionElement(), ns) ) {
		return ns;
	}

	return 0;
}

NanoSeconds PositionAccessible::seekRelativeMs(double percent, MilliSeconds refMs)
{
	return seekRelative(percent, refMs	* 1000000);
}

NanoSeconds PositionAccessible::seekAbsoluteMs(MilliSeconds ms)
{
	return seekAbsolute(ms * 1000000);
}

NanoSeconds PositionAccessible::seekNearestMs(MilliSeconds ms)
{
	return seekNearest(ms * 1000000);
}

MilliSeconds PositionAccessible::positionMs() const
{
	return Engine::Utils::getPositionMs(positionElement());
}

MilliSeconds PositionAccessible::durationMs() const
{
	return Engine::Utils::getDurationMs(positionElement());
}
