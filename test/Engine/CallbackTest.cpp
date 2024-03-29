/*
 * Copyright (C) 2011-2024 Michael Lugmair
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
#include "Utils/Algorithm.h"
#include "Utils/Utils.h"

#include <glib-2.0/gobject/gvalue.h>
#include <gstreamer-1.0/gst/app/gstappsink.h>
#include <gstreamer-1.0/gst/gstbuffer.h>
#include <gstreamer-1.0/gst/gstcaps.h>
#include <gstreamer-1.0/gst/gstmessage.h>
#include <gstreamer-1.0/gst/gsttaglist.h>

#include <map>

namespace
{
	GValue convertVectorToBoxedGstValueArray(const std::vector<double>& vec)
	{
		GValue valueArray = G_VALUE_INIT;
		g_value_init(&valueArray, G_TYPE_VALUE_ARRAY);

		auto* garray = g_value_array_new(0);
		for(const auto& v: vec)
		{
			GValue singleValue = G_VALUE_INIT;
			g_value_init(&singleValue, G_TYPE_DOUBLE);
			g_value_set_double(&singleValue, v);
			g_value_array_append(garray, &singleValue);
		}

		g_value_take_boxed(&valueArray, garray);

		return valueArray;
	}

	GValue ascendingValueList(const size_t count)
	{
		GValue valueList = G_VALUE_INIT;
		g_value_init(&valueList, GST_TYPE_LIST);

		for(auto i = 0U; i < count; i++)
		{
			GValue singleValue = G_VALUE_INIT;
			g_value_init(&singleValue, G_TYPE_FLOAT);
			g_value_set_float(&singleValue, static_cast<float>(i));
			gst_value_list_append_and_take_value(&valueList, &singleValue);
		}

		return valueList;
	}

	GstTagList* createTagList(const std::map<QString, QString>& tagMap)
	{
		auto* tagList = gst_tag_list_new_empty();
		for(const auto& [tag, value]: tagMap)
		{
			GValue gValue = G_VALUE_INIT;
			g_value_init(&gValue, G_TYPE_STRING);
			g_value_set_string(&gValue, value.toLocal8Bit().constData());
			gst_tag_list_add_value(tagList, GST_TAG_MERGE_REPLACE, tag.toLocal8Bit().constData(), &gValue);
		}

		return tagList;
	}

	GstTagList* createTagListForCover(const int buffersize, const char* mimeDataString)
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

		void setSpectrum(const std::vector<float>& spectrum) override { m_spectrum = spectrum; }

		[[nodiscard]] const std::vector<float>& spectrum() const override { return m_spectrum; }

		void setLevel(float left, float right) override { m_level = {left, right}; }

		[[nodiscard]] QPair<float, float> level() const override { return m_level; }

		void setVisualizerEnabled(bool /*levelEnabled*/, bool /*spectrumEnabled*/) override {}

		void setBroadcastEnabled(bool /*b*/) override {}

		void setEqualizer(int /*band*/, int /*value*/) override {}

		[[nodiscard]] MetaData currentTrack() const override { return m_track; }

		void play() override {}

		void stop() override {}

		void pause() override {}

		void jumpAbsMs(MilliSeconds /*ms*/) override {}

		void jumpRelMs(MilliSeconds /*ms*/) override {}

		void jumpRel(double /*percent*/) override {}

		void updateMetadata(const MetaData& track, GstElement* /*src*/) override { m_track = track; }

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
		QPair<float, float> m_level;
		CoverData m_coverData;
		MetaData m_track;

};
// access working directory with Test::Base::tempPath("somefile.txt");

class CallbackTest :
	public Test::Base
{
	Q_OBJECT

	public:
		CallbackTest() :
			Test::Base("CallbackTest")
		{
			gst_init(nullptr, nullptr);
		}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
		[[maybe_unused]] void coverTest()
		{
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
				auto* tagList = createTagListForCover(testCase.buffersize, testCase.mimeDataStr);
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

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
		[[maybe_unused]] void levelTest()
		{
			struct TestCase
			{
				const char* structureName {nullptr};
				const char* fieldName {nullptr};
				std::vector<double> data {0.0};
				int expectedValueCount;
			};

			const auto testCases = std::array {
				TestCase {"level", "peak", {-1.0, -0.5}, 2},
				TestCase {"level", "peak", {-1.0, -0.8, -0.7, -0.6, -0.5}, 2},
				TestCase {"level", "peak", {-1.0}, 1},
				TestCase {"level", "peak", {}, 0},
				TestCase {"elementName", "peak", {-1.0, -1.0}, 0},
			};

			for(const auto& testCase: testCases)
			{
				auto engine = EngineMock();

				auto valueArray = convertVectorToBoxedGstValueArray(testCase.data);
				auto* structure = gst_structure_new_empty(testCase.structureName);
				gst_structure_take_value(structure, testCase.fieldName, &valueArray);

				auto* element = gst_element_factory_make("fakesink", "level");
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
				auto* message = gst_message_new_element(GST_OBJECT(element), structure);

				auto result = Engine::Callbacks::busStateChanged(nullptr, message, &engine);
				QVERIFY(result == true);

				const auto [left, right] = engine.level();

				if(testCase.expectedValueCount == 1)
				{
					QVERIFY(left == static_cast<float>(testCase.data[0]));
					QVERIFY(left == right);
				}
				else if(testCase.expectedValueCount > 1)
				{
					QVERIFY(left == static_cast<float>(testCase.data[0]));
					QVERIFY(right == static_cast<float>(testCase.data[1]));
				}
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
		[[maybe_unused]] void spectrumTest()
		{
			constexpr const auto* elementName = "spectrum";
			constexpr const auto* structureName = "spectrum";
			constexpr const auto* fieldName = "magnitude";
			constexpr const auto count = 50U;

			auto engine = EngineMock();

			auto valueList = ascendingValueList(count);

			auto* structure = gst_structure_new_empty(structureName);
			gst_structure_take_value(structure, fieldName, &valueList);
			auto* element = gst_element_factory_make("fakesink", elementName);
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
			auto* message = gst_message_new_element(GST_OBJECT(element), structure);

			auto result = Engine::Callbacks::busStateChanged(nullptr, message, &engine);
			QVERIFY(result == true);

			const auto& spectrum = engine.spectrum();
			QVERIFY(spectrum.size() == count);
			for(auto i = 0U; i < count; i++)
			{
				QVERIFY(spectrum[i] == i * 1.0F);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
		[[maybe_unused]] void tagTest()
		{
			const auto title = QString("title");
			const auto artist = QString("artist");
			const auto album = QString("album");
			const auto albumArtist = QString("albumArtist");
			const auto comment = QString("comment");
			const auto showName = QString("showName");

			auto* tagList = createTagList(
				{
					{GST_TAG_TITLE,        title},
					{GST_TAG_ARTIST,       artist},
					{GST_TAG_ALBUM,        album},
					{GST_TAG_ALBUM_ARTIST, albumArtist},
					{GST_TAG_COMMENT,      comment},
					{GST_TAG_SHOW_NAME,    showName}
				}
			);

			gchar* str = nullptr;
			QVERIFY(gst_tag_list_get_string(tagList, GST_TAG_TITLE, &str));

			auto* element = gst_element_factory_make("fakesink", "sink");
			auto* message =
				gst_message_new_tag(GST_OBJECT(element), tagList); // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)

			auto engine = EngineMock();

			auto result = Engine::Callbacks::busStateChanged(nullptr, message, &engine);
			QVERIFY(result == true);

			const auto track = engine.currentTrack();
			QVERIFY(track.title() == title);
			QVERIFY(track.artist() == artist);
			QVERIFY(track.album() == album);
			QVERIFY(track.albumArtist() == albumArtist);
			QVERIFY(track.comment() == comment);
			QVERIFY(track.customField(GST_TAG_SHOW_NAME) == showName);
		}
};

QTEST_GUILESS_MAIN(CallbackTest)

#include "CallbackTest.moc"
