/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "Common/SayonaraTest.h"
#include "Common/PlayManagerMock.h"
#include "Components/Engine/Callbacks.h"
#include "Components/Engine/Engine.h"
#include "Components/Engine/EngineUtils.h"
#include "Utils/Utils.h"

#include <glib-2.0/gobject/gvalue.h>
#include <gstreamer-1.0/gst/gstbuffer.h>
#include <gstreamer-1.0/gst/gstcaps.h>
#include <gstreamer-1.0/gst/gstmessage.h>
#include <gstreamer-1.0/gst/gsttaglist.h>

namespace
{
	GstTagList* createTagList(const int buffersize, const char* mimeDataString)
	{
		auto* tagList = gst_tag_list_new_empty();
		auto* buffer = gst_buffer_new_and_alloc (buffersize);
		auto* caps = mimeDataString
		             ? gst_caps_new_empty_simple(mimeDataString)
		             : gst_caps_new_empty();
		auto* sample = gst_sample_new(buffer, caps, nullptr, nullptr);
		gst_buffer_unref(buffer);

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
		gst_tag_list_add(tagList, GST_TAG_MERGE_REPLACE, GST_TAG_IMAGE, sample, nullptr);

		return tagList;
	}
}

class EngineMock :
	public ::Engine::Engine
{
	public:
		EngineMock() :
			::Engine::Engine(nullptr) {}

		~EngineMock() override = default;

		void updateBitrate(Bitrate /*br*/, GstElement* /*src*/) override {}

		void updateDuration(GstElement* /*src*/) override {}

		void setTrackReady(GstElement* /*src*/) override {}

		void setTrackAlmostFinished(MilliSeconds /*time2go*/) override {}

		void setTrackFinished(GstElement* /*src*/) override {}

		[[nodiscard]] bool isStreamRecorderRecording() const override { return false; }

		void setStreamRecorderRecording(bool /*b*/) override {}

		void setSpectrum(const std::vector<float>& /*spectrum*/) override {}

		[[nodiscard]] const std::vector<float>& spectrum() const override { return m_spectrum; }

		void setLevel(float /*left*/, float /*right*/) override {}

		[[nodiscard]] QPair<float, float> level() const override { return {}; }

		void setVisualizerEnabled(bool /*levelEnabled*/, bool /*spectrumEnabled*/) override {}

		void setBroadcastEnabled(bool /*b*/) override {}

		void setEqualizer(int /*band*/, int /*value*/) override {}

		[[nodiscard]] MetaData currentTrack() const override { return {}; }

		void play() override {}

		void stop() override {}

		void pause() override {}

		void jumpAbsMs(MilliSeconds /*ms*/) override {}

		void jumpRelMs(MilliSeconds /*ms*/) override {}

		void jumpRel(double /*percent*/) override {}

		void updateMetadata(const MetaData& /*track*/, GstElement* /*src*/) override {}

		struct CoverData
		{
			GstElement* src {nullptr};
			QByteArray data;
			QString mimedata;
		};

		void updateCover(GstElement* src, const QByteArray& data, const QString& mimedata) override
		{
			m_coverData = CoverData {src, data, mimedata};
		}

		[[nodiscard]] CoverData coverData() const { return m_coverData; }

		bool changeTrack(const MetaData&  /*track*/) override { return false; }

		void setBufferState(int /*progress*/, GstElement* /*src*/) override {}

		void error(const QString& /*error*/, const QString& /*elementName*/) override {}

	private:
		std::vector<float> m_spectrum;
		CoverData m_coverData;

};
// access working directory with Test::Base::tempPath("somefile.txt");

class CallbackTest :
	public Test::Base
{
	Q_OBJECT

	public:
		CallbackTest() :
			Test::Base("CallbackTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
		[[maybe_unused]] void coverTest()
		{
			gst_init(nullptr, nullptr);

			struct TestCase
			{
				int buffersize {0};
				int expectedBuffersize {0};
				const char* mimeDataStr {nullptr};
				const char* elementName {nullptr};
			};

			const auto testCases = std::array {
				TestCase {123, 123, "image/png", "sink"},
				TestCase {123, 0, "audio/mpeg", "sink"},
				TestCase {123, 0, "", "sink"},
				TestCase {123, 0, nullptr, "sink"},
				TestCase {123, 0, "image/png", "fakesink"},
				TestCase {123, 0, "image/png", "lamesink"},
				TestCase {123, 123, "image/png", "supersink"},
			};

			for(const auto& testCase: testCases)
			{
				auto* tagList = createTagList(testCase.buffersize, testCase.mimeDataStr);
				auto* element = gst_element_factory_make("fakesink", testCase.elementName);
				auto* message =
					gst_message_new_tag(GST_OBJECT(element), tagList); // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)

				auto engine = EngineMock();

				auto result = Engine::Callbacks::busStateChanged(nullptr, message, &engine);

				QVERIFY(result);
				if(testCase.expectedBuffersize > 0)
				{
					QVERIFY(engine.coverData().src == element);
					QVERIFY(engine.coverData().data.size() == testCase.expectedBuffersize);
					QVERIFY(engine.coverData().mimedata == QString(testCase.mimeDataStr));
				}
			}
		}
};

QTEST_GUILESS_MAIN(CallbackTest)

#include "CallbackTest.moc"
