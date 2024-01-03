/* LanguageUtilTest.cpp
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

#include "Utils/Macros.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Language/LanguageUtils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/StandardPaths.h"

#include <array>
#include <utility>

class LanguageUtilTest :
	public Test::Base
{
	Q_OBJECT

	public:
		LanguageUtilTest() :
			Test::Base("LanguageUtilTest") {}

		~LanguageUtilTest() override = default;

	private slots:
		[[maybe_unused]] void basicPathTests();
		[[maybe_unused]] void languageVersionTest();
		[[maybe_unused]] void languageCodeTest();
};

using namespace Util;

[[maybe_unused]] void LanguageUtilTest::basicPathTests()
{
	namespace L = Language;

	const auto sharePath = Util::translationsSharePath();
	const auto translationsPath = Util::translationsPath();

	const auto testSets = std::array {
		std::pair {L::getSharePath("ab_CD"), File::cleanFilename(sharePath + "/sayonara_lang_ab_CD.qm")},
		std::pair {L::getHttpPath("ab_CD"),
		           QString("https://sayonara-player.com/files/translation/sayonara_lang_ab_CD.qm")
		},
		std::pair {L::getHomeTargetPath("ab_CD"), File::cleanFilename(translationsPath + "/sayonara_lang_ab_CD.qm")},
		std::pair {L::getIconPath("ab_CD"), QString()},
		std::pair {L::getSharePath("hallo"), QString()},
		std::pair {L::getSharePath(""), QString()},
		std::pair {L::getSharePath("en_GBS"), QString()},
		std::pair {L::getSharePath("EN"), QString()},
	};

	for(const auto& [input, expected]: testSets)
	{
		QVERIFY(input == expected);
	}
}

[[maybe_unused]] void LanguageUtilTest::languageVersionTest()
{
	using namespace Util;
	auto* settings = Settings::instance();
	settings->checkSettings();

	auto currentVersion = QString {SAYONARA_VERSION};
	currentVersion.replace("2", "3");
	currentVersion.replace("1", "2");
	currentVersion.replace("0", "1");

	Language::setLanguageSettingFilename(Test::Base::tempPath("somefile.ini"));
	Language::setLanguageVersion("fr_FR", "1.2.0");
	Language::setLanguageVersion("de_DE", currentVersion);
	Language::setLanguageVersion("pt_PT", "");
	Language::setLanguageVersion("ab_CD", SAYONARA_VERSION);

	const auto testSets = std::array {
		std::pair {"fr_FR", true},
		std::pair {"de_DE", false},
		std::pair {"pt_PT", true},
		std::pair {"ab_CD", false},
		std::pair {"", true}
	};

	for(const auto& [languageCode, result]: testSets)
	{
		QVERIFY(Language::isOutdated(languageCode) == result);
	}
}

[[maybe_unused]] void LanguageUtilTest::languageCodeTest()
{
	const auto testSets = std::array {
		std::pair {"blupp.ts", QString()},
		std::pair {"sayonara_lang_de_DE.ts", QString("de_DE")},
		std::pair {"sayonara_lang_de.ts", QString("de")},
		std::pair {"asd;flkjasdsayonara_lang_de_DE.qm", QString("de_DE")},
		std::pair {"sayonara_lang_de_DE.ISO-8859-15.ts", QString("de_DE.ISO-8859-15")},
		std::pair {"sayonara_lang_zh_CN.GB2312.ts", QString("zh_CN.GB2312")},
		std::pair {"sayonara_lang_zh_CN.GB2312.ts.qm", QString()},
		std::pair {"sayonara_lang_zh_CN.GB2312-15.ts.qm", QString()},
		std::pair {"asd;flkjasdsayonara_lang_de_DE.xz", QString()},
		std::pair {"asd;flkjasdsayonara_lang_DE_DE.qm", QString()}
	};

	for(const auto& [filename, expectedCode]: testSets)
	{
		QVERIFY(Language::extractLanguageCode(filename) == expectedCode);
	}
}

QTEST_GUILESS_MAIN(LanguageUtilTest)

#include "LanguageUtilTest.moc"