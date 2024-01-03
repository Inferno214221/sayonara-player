/* UtilTest.cpp
 *
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

#include "test/Common/SayonaraTest.h"

#include "Utils/Utils.h"

#include <QDate>
#include <QTime>
#include <QDateTime>

// access working directory with Test::Base::tempPath("somefile.txt");

class UtilTest : 
    public Test::Base
{
    Q_OBJECT

    public:
        UtilTest() :
            Test::Base("UtilTest")
        {}

    private slots:
        void createLinkTest();
        void toFirstUpperTest();
        void dateToIntTest();
        void filefilterTest();
        void easyTagFinderTest();
        void msToStringTest();
};

namespace
{
	struct Link
	{
		QString href;
		QString style;
	};

	Link splitLink(const QString& link)
	{
		Link result;
		auto reStyle = QRegExp(".*style=\"(.*)\".*");
		reStyle.setMinimal(true);
		if(reStyle.indexIn(link) >= 0){
			result.style = reStyle.cap(1);
		}

		auto reHref = QRegExp(".*href=\"(.*)\".*");
		reHref.setMinimal(true);
		if(reHref.indexIn(link) >= 0){
			result.href = reHref.cap(1);
		}

		return result;
	}
}

void UtilTest::createLinkTest()
{
	{
		const auto link = Util::createLink("/path/to/somewhere", true, true);
		const auto splitted = splitLink(link);
		QVERIFY(splitted.style.contains("text-decoration: underline"));
		QVERIFY(splitted.href == "file:///path/to/somewhere");
	}

	{
		const auto link = Util::createLink("https://path/to/somewhere", true, false);
		const auto splitted = splitLink(link);
		QVERIFY(splitted.style.contains("text-decoration: none"));
		QVERIFY(splitted.href == "https://path/to/somewhere");
	}

	{
		const auto link = Util::createLink("My Link", true, false, "https://path/to/somewhere");
		const auto splitted = splitLink(link);
		QVERIFY(link.contains(">My Link<"));
		QVERIFY(splitted.style.contains("text-decoration: none"));
		QVERIFY(splitted.href == "https://path/to/somewhere");
	}
}

void UtilTest::toFirstUpperTest()
{
	auto str = Util::stringToFirstUpper("This is a string");
	QVERIFY(str == "This Is A String");

	str = Util::stringToFirstUpper("tHis iS a STring");
	QVERIFY(str == "This Is A String");

	str = Util::stringToFirstUpper("guns 'n' roses");
	QVERIFY(str == "Guns 'n' Roses");

	str = Util::stringToFirstUpper("tito&tarantula");
	QVERIFY(str == "Tito&Tarantula");

	// very first upper
	str = Util::stringToVeryFirstUpper("This is a string");
	QVERIFY(str == "This is a string");

	str = Util::stringToVeryFirstUpper("tHis iS a STring");
	QVERIFY(str == "This is a string");

	str = Util::stringToVeryFirstUpper("guns 'n' Roses");
	QVERIFY(str == "Guns 'n' roses");

	str = Util::stringToVeryFirstUpper("Tito&Tarantula");
	QVERIFY(str == "Tito&tarantula");
}

void UtilTest::dateToIntTest()
{
	const auto date = QDate(2020, 4, 14);
	const auto time = QTime(9, 20, 32);
	auto dateTime = QDateTime(date, time);
	dateTime.setOffsetFromUtc(0);

	auto intDate = Util::dateToInt(dateTime);

	QVERIFY(intDate == 20200414092032L);

	auto dateTime2 = Util::intToDate(intDate);
	QVERIFY(dateTime2.date().year() == 2020);
	QVERIFY(dateTime2.date().month() == 4);
	QVERIFY(dateTime2.date().day() == 14);
	QVERIFY(dateTime2.time().hour() == 9);
	QVERIFY(dateTime2.time().minute() == 20);
	QVERIFY(dateTime2.time().second() == 32);
}

void UtilTest::filefilterTest()
{
	{
		const auto mask = static_cast<Util::Extensions>(+Util::Extension::Images);
		const auto filter = Util::getFileFilter(mask, "images");
		const auto extensions = Util::imageExtensions(true);
		const auto expectedString = QString("images (%1)").arg(extensions.join(" "));
		QVERIFY(filter == expectedString);
	}

	{
		const auto mask = static_cast<Util::Extensions>(+Util::Extension::Soundfile);
		const auto filter = Util::getFileFilter(mask, "sound");
		const auto extensions = Util::soundfileExtensions(true);
		const auto expectedString = QString("sound (%1)").arg(extensions.join(" "));
		QVERIFY(filter == expectedString);
	}

	{
		const auto mask = static_cast<Util::Extensions>(+Util::Extension::Playlist);
		const auto filter = Util::getFileFilter(mask, "playlist");
		const auto extensions = Util::playlistExtensions(true);
		const auto expectedString = QString("playlist (%1)").arg(extensions.join(" "));
		QVERIFY(filter == expectedString);
	}

	{
		const auto mask = static_cast<Util::Extensions>(+Util::Extension::Soundfile | +Util::Extension::Images);
		const auto filter = Util::getFileFilter(mask, "all");
		const auto extensions = Util::soundfileExtensions(true) + Util::imageExtensions(true);
		const auto expectedString = QString("all (%1)").arg(extensions.join(" "));
		QVERIFY(filter == expectedString);
	}
}

void UtilTest::easyTagFinderTest()
{
	const auto document1 = R"(
	<xml>
		<path1></path1>
		<path2></path2>
		<path3>
			<path31></path31>
			<path32>
				<element>Hallo</element>
			</path32>
		</path3>
		<path4></path4>
	</xml>)";

	{
		const auto result = Util::easyTagFinder(QString(), "THIS IS NO XML DOCUMENT");
		QVERIFY(result.isEmpty());
	}

	{
		const auto result = Util::easyTagFinder(QString(), document1);
		QVERIFY(result.isEmpty());
	}

	{
		const auto result = Util::easyTagFinder("xml.path3.path32.element", document1);
		QVERIFY(result == "Hallo");
	}

	{
		const auto result = Util::easyTagFinder("path3.path32.element", document1);
		QVERIFY(result.isEmpty());
	}

	{
		const auto result = Util::easyTagFinder("xml.path2.path32.element", document1);
		QVERIFY(result.isEmpty());
	}
}

void UtilTest::msToStringTest()
{
	{
		const auto ms = 5643154564L;
		QVERIFY(Util::msToString(ms, "$Dd $Hh $Mm $Ss") == "65d 7h 32m 34s");
		QVERIFY(Util::msToString(ms, "$Hh $Mm $Ss") == "1567h 32m 34s");
		QVERIFY(Util::msToString(ms, "$Mm $Ss") == "94052m 34s");
	}

	{
		const auto ms = 5647315L;
		QVERIFY(Util::msToString(ms, "$D:$H:$M:$S") == ":1:34:07");
		QVERIFY(Util::msToString(ms, "$H:$M:$S") == "1:34:07");
		QVERIFY(Util::msToString(ms, "$M:$S") == "94:07");
	}

	{
		const auto ms = 564315L;
		QVERIFY(Util::msToString(ms, "$D:$H:$M:$S") == "::09:24");
		QVERIFY(Util::msToString(ms, "$H:$M:$S") == ":09:24");
		QVERIFY(Util::msToString(ms, "$M:$S") == "09:24");
		QVERIFY(Util::msToString(ms, "$S") == "564");
	}
}

QTEST_GUILESS_MAIN(UtilTest)
#include "UtilTest.moc"
