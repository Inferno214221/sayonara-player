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
#include "Components/Engine/EngineUtils.h"

#include <gst/gst.h>
#include <gst/gstelement.h>
#include <gst/gstpad.h>

class EngineUtilsTest :
	public Test::Base
{
	Q_OBJECT

	public:
		EngineUtilsTest() :
			Test::Base("EngineUtilsTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testUnref()
		{
			gst_init(nullptr, nullptr);

			GstElement* element;
			const auto success = Engine::Utils::createElement(&element, "fakesink");
			QVERIFY(success);

			gst_object_ref(element);

			QVERIFY(GST_OBJECT_REFCOUNT(element) == 2);

			{
				[[maybe_unused]] const auto unref = Engine::Utils::AutoUnref {element};
			}

			QVERIFY(GST_OBJECT_REFCOUNT(element) == 1);
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testElementName()
		{
			gst_init(nullptr, nullptr);

			const auto elementName = QString {"queue2"};

			GstElement* element;
			const auto success = Engine::Utils::createElement(&element, elementName, "test");
			QVERIFY(success);

			const auto fetchedElementName = Engine::Utils::getElementName(element);

			QVERIFY(fetchedElementName == "test_queue2");

			gst_object_unref(element);
		}

		[[maybe_unused]] void testSetAndGetValue() // NOLINT(readability-convert-member-functions-to-static)
		{
			GstElement* element;
			const auto success = Engine::Utils::createElement(&element, "queue2");
			QVERIFY(success);

			Engine::Utils::setIntValue(element, "high-percent", 50);
			Engine::Utils::setUint64Value(element, "max-size-time", 100UL);

			int intValue {0};
			uint64_t uint64Value {0};
			g_object_get(element, "high-percent", &intValue, "max-size-time", &uint64Value, nullptr);

			QVERIFY(intValue == 50);
			QVERIFY(uint64Value == 100UL);
		}

		[[maybe_unused]] void testSetAndGetValueWithList() // NOLINT(readability-convert-member-functions-to-static)
		{
			GstElement* element;
			const auto success = Engine::Utils::createElement(&element, "queue2");
			QVERIFY(success);

			const auto* tempLocation = tempPath("XXXXXXXX").toLocal8Bit().constData();
			constexpr const auto doubleValue = 0.5;

			Engine::Utils::setValues(element, "high-watermark", doubleValue, "temp-template", tempLocation);

			gchar* fetchedTempLocation = nullptr;
			double fetchedHighWatermark {0.0};
			g_object_get(element,
			             "high-watermark", &fetchedHighWatermark,
			             "temp-template", &fetchedTempLocation, nullptr);

			QVERIFY(fetchedHighWatermark == doubleValue);
			QVERIFY(QString {fetchedTempLocation} == QString {tempLocation});

			g_free(fetchedTempLocation);
		}

		[[maybe_unused]] void testSetAndGetValueWithGValue() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto* element = gst_element_factory_make("queue2", nullptr);
			Engine::Utils::setIntValue(element, "high-percent", 50);
			Engine::Utils::setUint64Value(element, "max-size-time", 100UL);

			GValue intValue = G_VALUE_INIT;
			GValue uint64Value = G_VALUE_INIT;
			g_value_init(&intValue, G_TYPE_INT);
			g_value_init(&uint64Value, G_TYPE_UINT64);

			g_object_get_property(G_OBJECT(element), "high-percent", &intValue);
			g_object_get_property(G_OBJECT(element), "max-size-time", &uint64Value);

			QVERIFY(g_value_get_int(&intValue) == 50);
			QVERIFY(g_value_get_uint64(&uint64Value) == 100UL);
		}

		[[maybe_unused]] void testScopeOfQStringData() // NOLINT(readability-convert-member-functions-to-static)
		{
			QList<GstElement*> elements;

			{
				const auto elementNames = QStringList {"queue2", "appsrc", "fakesink"};
				for(const auto& elementName: elementNames)
				{
					GstElement* element {nullptr};
					const auto success = Engine::Utils::createElement(&element, elementName, "test");
					QVERIFY(success);

					elements << element;
				}
			}

			QVERIFY(Engine::Utils::getElementName(elements[0]) == "test_queue2");
			QVERIFY(Engine::Utils::getElementName(elements[1]) == "test_appsrc");
			QVERIFY(Engine::Utils::getElementName(elements[2]) == "test_fakesink");
		}

		[[maybe_unused]] void testAddElementsToBin() // NOLINT(readability-convert-member-functions-to-static)
		{
			gst_init(nullptr, nullptr);

			GstElement* bin = nullptr;

			const auto elementTypes = {"queue", "volume", "fakesink"};
			auto elements = QList<GstElement*> {};
			for(const auto& elementType: elementTypes)
			{
				GstElement* element = nullptr;
				Engine::Utils::createElement(&element, elementType, "test");

				elements << element;
			}

			const auto binCreated = Engine::Utils::createBin(&bin, elements, "test");
			QVERIFY(binCreated);

			const auto name = Engine::Utils::getObjectName(GST_OBJECT(bin));
			QVERIFY(Engine::Utils::getElementName(bin) == "testbin");

			for(auto* element: elements)
			{
				QVERIFY(Engine::Utils::hasElement(GST_BIN(bin), element));
			}

			auto srcPad0 = Engine::Utils::getStaticPad(elements[0], Engine::Utils::src);
			auto sinkPad1 = Engine::Utils::getStaticPad(elements[1], Engine::Utils::sink);
			auto srcPad1 = Engine::Utils::getStaticPad(elements[1], Engine::Utils::src);
			auto sinkPad2 = Engine::Utils::getStaticPad(elements[2], Engine::Utils::sink);

			QVERIFY(gst_pad_is_linked(*srcPad0));
			QVERIFY(gst_pad_is_linked(*sinkPad1));
			QVERIFY(gst_pad_is_linked(*srcPad1));
			QVERIFY(gst_pad_is_linked(*sinkPad2));
			QVERIFY(*Engine::Utils::getStaticPad(bin, Engine::Utils::sink) != nullptr);
		}

		[[maybe_unused]] void testScopeOfStaticPads() // NOLINT(readability-convert-member-functions-to-static)
		{
			gst_init(nullptr, nullptr);

			auto* bin = gst_bin_new("bin");

			const auto elementTypes = {"queue", "volume", "fakesink"};
			auto elements = QList<GstElement*> {};
			for(const auto& elementType: elementTypes)
			{
				GstElement* element = nullptr;
				Engine::Utils::createElement(&element, elementType, "test");

				elements << element;
			}

			const auto added = Engine::Utils::addElements(GST_BIN(bin), elements);
			QVERIFY(added);

			GstPad* srcPadReference = nullptr;
			{
				auto srcPad = Engine::Utils::getStaticPad(elements[0], Engine::Utils::src);
				auto sinkPad = Engine::Utils::getStaticPad(elements[1], Engine::Utils::sink);
				srcPadReference = *srcPad;

				QVERIFY(GST_OBJECT_REFCOUNT(*srcPad) == 2); // one from the element, one getStaticPad

				QVERIFY(Engine::Utils::linkElements(elements));
				QVERIFY(GST_OBJECT_REFCOUNT(*srcPad) == 2); // link has no effect on ref counter
			}

			QVERIFY(GST_OBJECT_REFCOUNT(srcPadReference) == 1); // one from the element

			Engine::Utils::unlinkElements(elements); // unline has no effect on ref counter
			Engine::Utils::removeElements(GST_BIN(bin), elements); // GstBin::removeElement has
			QVERIFY(GST_OBJECT_REFCOUNT(srcPadReference) == 0);
		}
};

QTEST_GUILESS_MAIN(EngineUtilsTest)

#include "EngineUtilsTest.moc"
