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

#include "Components/SmartPlaylists/DateConverter.h"
#include "Components/SmartPlaylists/StringConverter.h"
#include "Components/SmartPlaylists/TimeSpanConverter.h"

#include <array>
#include <utility>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	using OptionalInt = std::optional<int>;
}

using namespace SmartPlaylists;

class ConverterTest :
	public Test::Base
{
	Q_OBJECT

	public:
		ConverterTest() :
			Test::Base("ConverterTest") {}

	private slots:
		void testDateConverter();
		void testStringConverter();
};

void ConverterTest::testDateConverter()
{
	const auto converter = DateConverter();
	const auto testCasesIntToString = std::array {
		std::make_pair(20091214, QString("2009-12-14")),
		std::make_pair(2056487, QString()),
		std::make_pair(204547858, QString()),
		std::make_pair(0, QString())
	};

	for(const auto& [value, expected]: testCasesIntToString)
	{
		QVERIFY(converter.intToString(value) == expected);
	}

	const auto testCasesStringToInt = std::array<std::pair<QString, OptionalInt>, 4> {
		std::make_pair(QString("2009-12-14"), 20091214),
		std::make_pair(QString(), std::nullopt),
		std::make_pair(QString("Invalid String"), std::nullopt),
		std::make_pair(QString("9876-54-32"), std::nullopt)
	};

	for(const auto& [value, expected]: testCasesStringToInt)
	{
		QVERIFY(converter.stringToInt(value) == expected);
	}
}

void ConverterTest::testStringConverter()
{
	const auto converter = StringConverter();
	const auto testCasesIntToString = std::array {
		std::make_pair(20091214, "20091214"),
		std::make_pair(-2, "-2"),
		std::make_pair(204547858, "204547858"),
		std::make_pair(0, "0")
	};

	for(const auto& [value, expected]: testCasesIntToString)
	{
		QVERIFY(converter.intToString(value) == expected);
	}

	const auto testCasesStringToInt = std::array<std::pair<QString, OptionalInt>, 5> {
		std::make_pair(QString("20091214"), std::optional {20091214}),
		std::make_pair(QString("-2"), std::optional {-2}),
		std::make_pair(QString(), std::nullopt),
		std::make_pair(QString("9876-54-32"), std::nullopt),
		std::make_pair(QString("Invalid"), std::nullopt)
	};

	for(const auto& [value, expected]: testCasesStringToInt)
	{
		QVERIFY(converter.stringToInt(value) == expected);
	}
}

QTEST_GUILESS_MAIN(ConverterTest)

#include "ConverterTest.moc"
