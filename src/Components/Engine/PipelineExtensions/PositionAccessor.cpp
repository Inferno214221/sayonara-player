/* SeekHandler.cpp */

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

#include "PositionAccessor.h"
#include "Components/Engine/EngineUtils.h"
#include "Utils/Logger/Logger.h"

#include <gst/gst.h>
#include <gst/base/gstbasesrc.h>

#include <chrono>

namespace
{
	using ChronoNs = std::chrono::nanoseconds;
	using ChronoMs = std::chrono::milliseconds;

	// constexpr const auto SeekAccurate = static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE);
	constexpr const auto SeekNearest = static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT |
	                                                             GST_SEEK_FLAG_SNAP_NEAREST);

	bool seek(GstElement* audioSource, const GstSeekFlags flags, const std::chrono::nanoseconds ns)
	{
		if(!audioSource)
		{
			return false;
		}

		const auto success = gst_element_seek_simple(audioSource, GST_FORMAT_TIME, flags, ns.count());
		if(!success)
		{
			spLog(Log::Warning, "PositionAccessorImpl") << "seeking not possible";
		}

		return success;
	}

	ChronoNs msToNs(const ChronoMs ms)
	{
		return std::chrono::duration_cast<ChronoNs>(ms);
	}

	ChronoNs getRelativeTimeStampNs(const double percent, const ChronoMs duration)
	{
		if(percent > 1.0)
		{
			return msToNs(duration);
		}

		if(percent < 0)
		{
			return ChronoNs {0};
		}

		return ChronoNs {static_cast<ChronoNs::rep>(static_cast<double>(msToNs(duration).count()) * percent)};
	}

	class PositionAccessorImpl :
		public PipelineExtensions::PositionAccessor
	{
		public:
			// NOLINTNEXTLINE(*-easily-swappable-parameters)
			explicit PositionAccessorImpl(GstElement* readElement, GstElement* seekElement) :
				m_readElement {readElement},
				m_seekElement {seekElement} {}

			~PositionAccessorImpl() override = default; // NOLINT(cppcoreguidelines-explicit-virtual-functions)

			void seekRelative(const double percent, const MilliSeconds duration) override
			{
				const auto newTimeNs = getRelativeTimeStampNs(percent, ChronoMs {duration});
				seek(m_seekElement, SeekNearest, newTimeNs);
			}

			void seekAbsoluteMs(const MilliSeconds ms) override
			{
				seek(m_seekElement, SeekNearest, msToNs(ChronoMs {ms}));
			}

			void seekNearestMs(const MilliSeconds ms) override
			{
				seek(m_seekElement, SeekNearest, msToNs(ChronoMs {ms}));
			}

			[[nodiscard]] MilliSeconds positionMs() const override
			{
				return Engine::Utils::getPositionMs(m_readElement);
			}

			[[nodiscard]] MilliSeconds durationMs() const override
			{
				return Engine::Utils::getDurationMs(m_readElement);
			}

			[[nodiscard]] MilliSeconds timeToGo() const override
			{
				const auto ms = Engine::Utils::getTimeToGo(m_readElement);
				return std::max<MilliSeconds>(ms - 100, 0); // NOLINT(readability-magic-numbers)
			}

		private:
			GstElement* m_readElement;
			GstElement* m_seekElement;
	};
}

namespace PipelineExtensions
{
	PositionAccessor::~PositionAccessor() = default;

	std::shared_ptr<PositionAccessor> createPositionAccessor(GstElement* readElement, GstElement* seekElement)
	{
		return std::make_shared<PositionAccessorImpl>(readElement, seekElement);
	}
}
