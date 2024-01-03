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
#include "Utils/Settings/SettingConverter.h"
#include "Utils/Settings/SettingConvertible.h"

#include <QByteArray>
#include <QPoint>
#include <QSize>
#include <QStringList>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	struct CustomType :
		public SettingConvertible
	{
		int x {0};
		QString s;
		bool b {false};

		CustomType() = default;

		CustomType(int x_, QString s_, bool b_) :
			x {x_},
			s {std::move(s_)},
			b {b_} {}

		bool loadFromString(const QString& str) override
		{
			const auto lst = str.split(",");
			if(lst.size() < 3)
			{
				return false;
			}

			bool ok {false};
			x = lst[0].toInt(&ok);
			if(!ok)
			{
				return false;
			}

			s = lst[1];
			b = static_cast<bool>(lst[2].toInt(&ok));

			return ok;
		}

		[[nodiscard]] QString toString() const override
		{
			return QString("%1,%2,%3").arg(x).arg(s).arg(b);
		}
	};
}

class SettingConverterTest :
	public Test::Base
{
	Q_OBJECT

	public:
		SettingConverterTest() :
			Test::Base("SettingConverterTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void boolToString()
		{
			QVERIFY(SettingConverter::toString(true) == "true");
			QVERIFY(SettingConverter::toString(false) == "false");
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void stringToBool()
		{
			struct TestCase
			{
				QString str;
				bool expected {false};
			};

			const auto testCases = std::array {
				TestCase {"true", true},
				TestCase {"false", false},
				TestCase {"1", true},
				TestCase {"20", true},
				TestCase {"0", false},
				TestCase {"-5", false},
			};

			for(const auto& testCase: testCases)
			{
				bool b {false};
				const auto success = SettingConverter::fromString(testCase.str, b);
				QVERIFY(success);
				QVERIFY(b == testCase.expected);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void intToString()
		{
			QVERIFY(SettingConverter::toString(0) == "0");
			QVERIFY(SettingConverter::toString(3) == "3");
			QVERIFY(SettingConverter::toString(static_cast<int>(-252564658)) == "-252564658");
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void stringToInt()
		{
			struct TestCase
			{
				QString str;
				int expected {0};
				bool success {false};
			};

			const auto testCases = std::array {
				TestCase {"-654651321", -654651321, true},
				TestCase {"65465", 65465, true},
				TestCase {"0", 0, true},
				TestCase {"20.3453", 0, false}
			};

			for(const auto& testCase: testCases)
			{
				int i {0};
				const auto success = SettingConverter::fromString(testCase.str, i);
				QVERIFY(success == testCase.success);
				QVERIFY(i == testCase.expected);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void floatToString()
		{
			QVERIFY(SettingConverter::toString(0.3453F) == "0.3453"); // NOLINT(readability-magic-numbers)
			QVERIFY(SettingConverter::toString(-342.342F) == "-342.342"); // NOLINT(readability-magic-numbers)
			QVERIFY(SettingConverter::toString(0.0F) == "0");
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void stringToFloat()
		{
			struct TestCase
			{
				QString str;
				float expected {0};
				bool success {false};
			};

			const auto testCases = std::array {
				TestCase {"6548.234", 6548.234F, true},
				TestCase {"65465", 65465.0F, true},
				TestCase {"0", 0, true},
				TestCase {".0F", 0, false},
				TestCase {"hallo", 0, false}
			};

			for(const auto& testCase: testCases)
			{
				float f {0};
				const auto success = SettingConverter::fromString(testCase.str, f);
				QVERIFY(success == testCase.success);
				if(testCase.success)
				{
					QVERIFY(f == testCase.expected);
				}
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void stringListToString()
		{
			QVERIFY(SettingConverter::toString(QStringList {"a", "b", "c"}) == "a,b,c");
			QVERIFY(SettingConverter::toString(QStringList {}) == "");
			QVERIFY(SettingConverter::toString(QStringList {"x"}) == "x");
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void stringToStringList()
		{
			struct TestCase
			{
				QString str;
				QStringList expected;
			};

			const auto testCases = std::array {
				TestCase {"x,y,z", {"x", "y", "z"}},
				TestCase {"", {""}},
				TestCase {"asdf", {"asdf"}},
			};

			for(const auto& testCase: testCases)
			{
				QStringList stringList;
				const auto success = SettingConverter::fromString(testCase.str, stringList);
				QVERIFY(success);
				QVERIFY(stringList == testCase.expected);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void sizeToString()
		{
			const auto str = SettingConverter::toString(QSize {});
			QVERIFY(SettingConverter::toString(QSize {1, 2}) == "1,2");
			QVERIFY(SettingConverter::toString(QSize {}) == "-1,-1");
			QVERIFY(SettingConverter::toString(QSize {-3, -435}) == "-3,-435");
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void stringToSize()
		{
			struct TestCase
			{
				QString str;
				QSize expected;
				bool success {false};
			};

			const auto testCases = std::array {
				TestCase {"1,2", {1, 2}, true},
				TestCase {"", {}, false},
				TestCase {"2", {}, false},
				TestCase {"2,3,4", {2, 3}, true},
				TestCase {"a,b", {}, false},
			};

			for(const auto& testCase: testCases)
			{
				QSize size;
				const auto success = SettingConverter::fromString(testCase.str, size);
				QVERIFY(success == testCase.success);
				QVERIFY(size == testCase.expected);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void pointToString()
		{
			QVERIFY(SettingConverter::toString(QPoint {1, 2}) == "1,2");
			QVERIFY(SettingConverter::toString(QPoint {}) == "0,0");
			QVERIFY(SettingConverter::toString(QPoint {-3, -435}) == "-3,-435");
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void stringToPoint()
		{
			struct TestCase
			{
				QString str;
				QPoint expected;
				bool success {false};
			};

			const auto testCases = std::array {
				TestCase {"1,2", {1, 2}, true},
				TestCase {"", {}, false},
				TestCase {"2", {}, false},
				TestCase {"2,3,4", {2, 3}, true},
				TestCase {"a,b", {}, false},
			};

			for(const auto& testCase: testCases)
			{
				QPoint point;
				const auto success = SettingConverter::fromString(testCase.str, point);
				QVERIFY(success == testCase.success);
				QVERIFY(point == testCase.expected);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testByteArrayConversion()
		{
			struct TestCase
			{
				QByteArray arr;
			};

			const auto testCases = std::array {
				TestCase {},
				TestCase {"some complicated string"},
				TestCase {"98023lkn/;a'][\';\';;;'"},
				TestCase {"Радость Простых Мелодий"},
				TestCase {"頭士奈生樹"},
				TestCase {""}
			};

			for(const auto& testCase: testCases)
			{
				const auto str = SettingConverter::toString(testCase.arr);

				auto byteArray = QByteArray {};
				const auto convertedToBA = SettingConverter::fromString(str, byteArray);

				QVERIFY(convertedToBA);
				QVERIFY(byteArray == testCase.arr);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void customTypeToString()
		{
			QVERIFY(SettingConverter::toString(CustomType {3, "hallo", false}) == "3,hallo,0");
			QVERIFY(SettingConverter::toString(CustomType {-3, "", true}) == "-3,,1");

		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
		[[maybe_unused]] void stringToCustomType()
		{
			struct TestCase
			{
				QString str;
				CustomType expected;
				bool success;
			};

			const auto testCases = std::array {
				TestCase {"3,hallo,0", {3, "hallo", false}, true},
				TestCase {"asdf, asdf, asdf", {0, {}, false}, false}
			};

			for(const auto& testCase: testCases)
			{
				CustomType customType;
				const auto success = SettingConverter::fromString(testCase.str, customType);
				QVERIFY(success == testCase.success);
				if(success)
				{
					QVERIFY(customType.x == testCase.expected.x);
					QVERIFY(customType.s == testCase.expected.s);
					QVERIFY(customType.b == testCase.expected.b);
				}
			}
		}
};

QTEST_GUILESS_MAIN(SettingConverterTest)

#include "SettingConverterTest.moc"
