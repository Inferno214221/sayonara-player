/* ChangeablePipeline.cpp */

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

#include "Changeable.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Components/Engine/EngineUtils.h"

#include <QList>

#include <memory>
#include <mutex>

namespace
{
	static std::mutex globalMutex;

	constexpr const auto MaxTimeout = 2000;
	const int SleepInterval = 50;

	class WeakLockGuard
	{
		public:
			explicit WeakLockGuard(std::mutex* mtx) :
				m_mtx {mtx},
				m_couldLock {m_mtx->try_lock()} {}

			~WeakLockGuard()
			{
				if(m_couldLock)
				{
					m_mtx->unlock();
				}
			}

			[[nodiscard]] bool wasLocked() const { return m_couldLock; }

		private:
			std::mutex* m_mtx;
			bool m_couldLock;
	};

	template<typename T, typename Callback>
	bool wait(int timeoutMs, T* probeData, Callback&& onTimeoutReached)
	{
		while(!probeData->done)
		{
			Util::sleepMs(SleepInterval);
			timeoutMs -= SleepInterval;
			if(timeoutMs <= 0)
			{
				spLog(Log::Warning, "Changeable") << "Wait for timeout: Could not establish probe callback ";
				onTimeoutReached();
				return false;
			}
		}

		Engine::Utils::setState(probeData->newSink, GST_STATE_PLAYING);

		return true;
	}

	template<typename T>
	void installFlushListener(GstElement* element, T* probeData)
	{
		auto* srcpad = gst_element_get_static_pad(element, "src");
		gst_pad_add_probe(
			srcpad,
			GstPadProbeType(GST_PAD_PROBE_TYPE_BLOCK | GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM),
			probeData->onElementFlushed,
			probeData, nullptr);
		gst_object_unref(srcpad);
	}

	void flushData(GstElement* element)
	{
		auto* sinkpad = gst_element_get_static_pad(element, "sink");
		gst_pad_send_event(sinkpad, gst_event_new_eos());
		gst_object_unref(sinkpad);
	}

	struct ProbeData
	{
		GstElement* elementA = nullptr;
		GstElement* elementB = nullptr;
		GstElement* elementOfInterest = nullptr;
		GstElement* bin = nullptr;
		GstElement* newSink = nullptr;

		GstState oldState {GST_STATE_NULL};
		bool done {false};

		GstPadProbeReturn (* onElementFlushed)(GstPad*, GstPadProbeInfo*, gpointer);
		GstPadProbeReturn (* onDataFlowStopped)(GstPad*, GstPadProbeInfo*, gpointer);
	};

	GstPadProbeReturn dataFlowStoppedAndFlush(GstPad* pad, GstPadProbeInfo* info, gpointer data)
	{
		auto* probeData = static_cast<ProbeData*>(data);

		gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

		installFlushListener(probeData->elementOfInterest, probeData);
		flushData(probeData->elementOfInterest);

		return GST_PAD_PROBE_OK;
	}

	template<typename T>
	bool installDataFlowStoppedListener(GstElement* element, T* probeData)
	{
		auto* srcPad = gst_element_get_static_pad(element, "src");
		const auto probeId = gst_pad_add_probe(
			srcPad,
			GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
			probeData->onDataFlowStopped,
			probeData, nullptr);

		const auto success = wait(MaxTimeout, probeData, [&]() {
			gst_pad_remove_probe(srcPad, probeId);
		});

		return success;
	}

	GstPadProbeReturn dataFlowStoppedForInsert(GstPad* pad, GstPadProbeInfo* info, gpointer data)
	{
		auto* probeData = static_cast<ProbeData*>(data);

		gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID (info));

		Engine::Utils::setState(probeData->elementA, GST_STATE_NULL);
		Engine::Utils::setState(probeData->elementOfInterest, GST_STATE_NULL);

		Engine::Utils::addElements(GST_BIN(probeData->bin), {probeData->elementOfInterest});
		Engine::Utils::unlinkElements({probeData->elementA, probeData->elementB});

		Engine::Utils::linkElements({probeData->elementA, probeData->elementOfInterest, probeData->elementB});

		Engine::Utils::setState(probeData->elementOfInterest, probeData->oldState);
		Engine::Utils::setState(probeData->elementA, probeData->oldState);

		if(probeData->elementB)
		{
			Engine::Utils::setState(probeData->elementB, probeData->oldState);
		}

		probeData->done = true;

		return GST_PAD_PROBE_DROP;
	}

	void removeElement(ProbeData* probeData)
	{
		Engine::Utils::unlinkElements({probeData->elementA, probeData->elementOfInterest, probeData->elementB});
		Engine::Utils::removeElements(GST_BIN(probeData->bin), {probeData->elementOfInterest});
		Engine::Utils::setState(probeData->elementOfInterest, GST_STATE_NULL);
		Engine::Utils::linkElements({probeData->elementA, probeData->elementB});
		Engine::Utils::setState(probeData->bin, probeData->oldState);
	}

	GstPadProbeReturn dataFlushedForRemoval(GstPad* pad, GstPadProbeInfo* info, gpointer data)
	{
		auto* probeData = static_cast<ProbeData*>(data);

		if(GST_EVENT_TYPE(GST_PAD_PROBE_INFO_DATA(info)) != GST_EVENT_EOS)
		{
			return GST_PAD_PROBE_PASS;
		}

		// remove EOS listener
		gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

		removeElement(probeData);

		probeData->done = true;

		return GST_PAD_PROBE_DROP;
	}

	struct ReplaceSinkProbeData
	{
		GstElement* oldSink = nullptr;
		GstElement* newSink = nullptr;
		GstElement* elementA = nullptr;
		GstElement* bin = nullptr;
		GstPadProbeReturn (* onDataFlowStopped)(GstPad*, GstPadProbeInfo*, gpointer) = nullptr;

		bool done {false};
		GstState oldState {GST_STATE_NULL};
	};

	bool removeElementDirectly(GstElement* elementA, GstElement* elementOfInterest, GstElement* elementB, GstBin* bin)
	{
		Engine::Utils::unlinkElements({elementA, elementOfInterest, elementB});
		Engine::Utils::removeElements(bin, {elementOfInterest});
		return Engine::Utils::linkElements({elementA, elementB});
	}

	GstPadProbeReturn dataFlowStoppedForReplace(GstPad* pad, GstPadProbeInfo* info, gpointer data)
	{
		gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID (info));

		auto* probeData = static_cast<ReplaceSinkProbeData*>(data);
		const auto pos = Engine::Utils::getPositionMs(probeData->oldSink);

		gst_object_ref(probeData->bin);
		gst_object_ref(probeData->oldSink);

		Engine::Utils::unlinkElements({probeData->elementA, probeData->oldSink});
		Engine::Utils::removeElements(GST_BIN(probeData->bin), {probeData->oldSink});
		Engine::Utils::setState(probeData->oldSink, GST_STATE_NULL);

		Engine::Utils::addElements(GST_BIN(probeData->bin), {probeData->newSink});
		Engine::Utils::linkElements({probeData->elementA, probeData->newSink});
		Engine::Utils::setState(probeData->newSink, GST_STATE_PLAYING);

		// flush, in order to force new buffers into the sink. Without seeking
		constexpr const auto SeekFlags = static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH |
		                                                           GST_SEEK_FLAG_KEY_UNIT |
		                                                           GST_SEEK_FLAG_SNAP_NEAREST);

		gst_element_seek_simple(probeData->newSink, GST_FORMAT_TIME, SeekFlags, pos * GST_MSECOND);

		probeData->done = true;

		return GST_PAD_PROBE_DROP;
	}

	bool addElementDirectly(GstElement* elementA, GstElement* elementOfInterest, GstElement* elementB, GstBin* bin)
	{
		spLog(Log::Develop, "Changeable") << "Add element directly";

		Engine::Utils::unlinkElements({elementA, elementB});

		const auto isAdded = Engine::Utils::addElements(bin, {elementOfInterest});
		if(isAdded)
		{
			const auto isLinked = Engine::Utils::linkElements({elementA, elementOfInterest, elementB});
			if(!isLinked)
			{
				spLog(Log::Warning, "Changeable") << "Could not link elements for any reason";
				Engine::Utils::removeElements(bin, {elementOfInterest});

				return false;
			}

			return true;
		}

		return false;
	}

	bool replaceSinkDirectly(GstElement* oldSink, GstElement* newSink, GstElement* elementA, GstBin* bin)
	{
		if(!Engine::Utils::addElements(bin, {newSink}))
		{
			return false;
		}

		if(!Engine::Utils::linkElements({elementA, newSink}))
		{
			Engine::Utils::removeElements(bin, {newSink});
			return false;
		}

		Engine::Utils::removeElements(bin, {oldSink});
		return true;
	}
}

namespace PipelineExtensions
{
	Changeable::Changeable() = default;
	Changeable::~Changeable() = default;

	bool Changeable::addElement(GstElement* element, GstElement* elementA, GstElement* elementB)
	{
		const auto lockGuard = WeakLockGuard(&globalMutex);
		if(!lockGuard.wasLocked())
		{
			return false;
		}

		const auto name = Engine::Utils::GStringAutoFree(gst_element_get_name(element));
		auto parent = Engine::Utils::AutoUnref(gst_element_get_parent(elementA));
		auto* parentElement = GST_ELEMENT(*parent);

		spLog(Log::Debug, this) << "Add " << name.data() << " to pipeline";
		if(Engine::Utils::hasElement(GST_BIN(parentElement), element))
		{
			spLog(Log::Debug, this) << "Element " << name.data() << "already in pipeline";
			return true;
		}

		auto probeData = std::make_shared<ProbeData>();
		probeData->elementA = elementA;
		probeData->elementB = elementB;
		probeData->elementOfInterest = element;
		probeData->bin = parentElement;
		probeData->onDataFlowStopped = dataFlowStoppedForInsert;
		probeData->oldState = Engine::Utils::getState(parentElement);

		const auto success = (probeData->oldState == GST_STATE_NULL)
		                     ? addElementDirectly(elementA, element, elementB, GST_BIN(parentElement))
		                     : installDataFlowStoppedListener(elementA, probeData.get());

		if(!success)
		{
			spLog(Log::Debug, this) << "Could not add " << name.data();
		}

		return success;
	}

	bool Changeable::removeElement(GstElement* element, GstElement* elementA, GstElement* elementB)
	{
		const auto lockGuard = WeakLockGuard(&globalMutex);
		if(!lockGuard.wasLocked())
		{
			return false;
		}

		auto parent = Engine::Utils::AutoUnref(gst_element_get_parent(elementA));
		auto* parentElement = GST_ELEMENT(*parent);
		const auto name = Engine::Utils::GStringAutoFree(gst_element_get_name(element));

		spLog(Log::Debug, this) << "Remove " << name.data() << " from pipeline";
		if(!Engine::Utils::hasElement(GST_BIN(parentElement), element))
		{
			spLog(Log::Debug, this) << "Element " << name.data() << " not in pipeline";
			return true;
		}

		auto probeData = std::make_shared<ProbeData>();
		probeData->elementA = elementA;
		probeData->elementB = elementB;
		probeData->elementOfInterest = element;
		probeData->bin = parentElement;
		probeData->oldState = Engine::Utils::getState(parentElement);
		probeData->onDataFlowStopped = dataFlowStoppedAndFlush;
		probeData->onElementFlushed = dataFlushedForRemoval;

		// we need that element later, but a gst_bin_remove decreases refcount
		gst_object_ref(element);

		const auto success = (probeData->oldState == GST_STATE_NULL)
		                     ? removeElementDirectly(elementA, element, elementB, GST_BIN(parentElement))
		                     : installDataFlowStoppedListener(elementA, probeData.get());

		if(!success)
		{
			spLog(Log::Debug, this) << "Could not remove " << name.data();
		}

		return success;
	}

	bool Changeable::replaceSink(GstElement* oldSink, GstElement* newSink, GstElement* elementA, GstElement* bin)
	{
		const auto lockGuard = WeakLockGuard(&globalMutex);
		if(!lockGuard.wasLocked())
		{
			return false;
		}

		const auto name = Engine::Utils::GStringAutoFree(gst_element_get_name(oldSink));
		spLog(Log::Debug, this) << "Remove " << name.data() << " from pipeline";

		if(!Engine::Utils::hasElement(GST_BIN(bin), oldSink))
		{
			spLog(Log::Debug, this) << "Element " << name.data() << " not in pipeline";
			return addElement(newSink, elementA, nullptr); // NOLINT(readability-suspicious-call-argument)
		}

		auto* probeData = new ReplaceSinkProbeData();
		probeData->oldSink = oldSink;
		probeData->newSink = newSink;
		probeData->elementA = elementA;
		probeData->bin = bin;
		probeData->oldState = Engine::Utils::getState(oldSink);
		probeData->onDataFlowStopped = dataFlowStoppedForReplace;

		const auto success = (probeData->oldState == GST_STATE_NULL)
		                     ? replaceSinkDirectly(oldSink, newSink, elementA, GST_BIN(bin))
		                     : installDataFlowStoppedListener(elementA, probeData);
		if(!success)
		{
			spLog(Log::Warning, this) << "Could not replace element " << name.data();
		}

		return success;
	}
}