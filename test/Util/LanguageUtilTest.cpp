#include "SayonaraTest.h"

#include "Utils/Macros.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Language/LanguageUtils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/StandardPaths.h"

#include <QStringList>

using namespace Util;

class LanguageUtilTest : public Test::Base
{
    Q_OBJECT

public:
	LanguageUtilTest() :
		Test::Base("LanguageUtilTest")
	{}

	~LanguageUtilTest() override = default;

private slots:
	void basic_path_tests();
	void language_version_test();
	void four_letter_test();
	void similar_language_test();
	void available_language_test();
};

void LanguageUtilTest::basic_path_tests()
{
	QString sp = Util::translationsSharePath();
	QString hp = Util::translationsPath();

	QString path;
	QString expected;

	path = Language::getSharePath("ab_CD");
	expected = File::cleanFilename(sp + "/sayonara_lang_ab_CD.qm");
	QVERIFY(path == expected);

	path = Language::getFtpPath("ab_CD");
	expected = QString("ftp://sayonara-player.com/translation/sayonara_lang_ab_CD.qm");
	QVERIFY(path == expected);

	path = Language::getHttpPath("ab_CD");
	expected = QString("https://sayonara-player.com/files/translation/sayonara_lang_ab_CD.qm");
	QVERIFY(path == expected);

	path = Language::getHomeTargetPath("ab_CD");
	expected = File::cleanFilename(hp + "/sayonara_lang_ab_CD.qm");
	QVERIFY(path == expected);

	path = Language::getIconPath("ab_CD");
	expected = QString();
	QVERIFY(path == expected);

	path = Language::getSharePath("hallo");
	QVERIFY(path.isEmpty());

	path = Language::getSharePath("");
	QVERIFY(path.isEmpty());

	path = Language::getSharePath("en_GBS");
	QVERIFY(path.isEmpty());

	path = Language::getSharePath("EN");
	QVERIFY(path.isEmpty());
}

void LanguageUtilTest::language_version_test()
{
	Settings* s = Settings::instance();
	s->checkSettings();

	QString current_version = SAYONARA_VERSION;
	current_version.replace("2", "3");
	current_version.replace("1", "2");
	current_version.replace("0", "1");

	Language::setTestMode();

	Language::setLanguageVersion("fr_FR", "1.2.0");
	Language::setLanguageVersion("de_DE", current_version);
	Language::setLanguageVersion("pt_PT", "");
	Language::setLanguageVersion("ab_CD", SAYONARA_VERSION);

	bool outdated;
	outdated = Language::isOutdated("fr_FR");
	QVERIFY(outdated == true);

	outdated = Language::isOutdated("de_DE");
	QVERIFY(outdated == false);

	outdated = Language::isOutdated("pt_PT");
	QVERIFY(outdated == true);

	outdated = Language::isOutdated("ab_CD");
	QVERIFY(outdated == false);

	outdated = Language::isOutdated("");
	QVERIFY(outdated == true);
}

void LanguageUtilTest::four_letter_test()
{
	QString expected;
	QString four_letter;

	four_letter = Language::extractLanguageCode("blupp.ts");
	expected = QString();
	QVERIFY(four_letter == expected);

	four_letter = Language::extractLanguageCode("sayonara_lang_de_DE.ts");
	expected = "de_DE";
	QVERIFY(four_letter == expected);

	four_letter = Language::extractLanguageCode("sayonara_lang_de.ts");
	expected = "de";
	QVERIFY(four_letter == expected);

	four_letter = Language::extractLanguageCode("asd;flkjasdsayonara_lang_de_DE.qm");
	expected = "de_DE";
	QVERIFY(four_letter == expected);

	four_letter = Language::extractLanguageCode("sayonara_lang_zh_CN.GB2312.ts");
	expected = "zh_CN.GB2312";
	QVERIFY(four_letter == expected);

	four_letter = Language::extractLanguageCode("sayonara_lang_de_DE.ISO-8859-15.ts");
	expected = "de_DE.ISO-8859-15";
	QVERIFY(four_letter == expected);

	four_letter = Language::extractLanguageCode("sayonara_lang_zh_CN.GB2312.ts.qm");
	expected = "";
	QVERIFY(four_letter == expected);

	four_letter = Language::extractLanguageCode("asd;flkjasdsayonara_lang_de_DE.xz");
	expected = QString();
	QVERIFY(four_letter == expected);

	four_letter = Language::extractLanguageCode("asd;flkjasdsayonara_lang_DE_DE.qm");
	expected = QString();
	QVERIFY(four_letter == expected);

	four_letter = Language::extractLanguageCode("blupp.ts");
	expected = QString();
	QVERIFY(four_letter == expected);
}

void LanguageUtilTest::similar_language_test()
{
//	QString similar_language, four_letter;
//	QString expected;

//	similar_language = Language::get_similar_language_4("en_US");
//	four_letter = Language::extract_four_letter(similar_language);
//	expected = "en_GB";
//	QVERIFY(four_letter == expected);
//	QVERIFY(File::exists(similar_language));

//	similar_language = Language::get_similar_language_4("fr_AL");
//	four_letter = Language::extract_four_letter(similar_language);
//	expected = "fr_FR";
//	QVERIFY(four_letter == expected);
//	QVERIFY(File::exists(similar_language));
}

void LanguageUtilTest::available_language_test()
{
//	QMap<QString, QLocale> loc = Lang::available_languages();
//	QVERIFY(loc.isEmpty() == false);
}

QTEST_GUILESS_MAIN(LanguageUtilTest)
#include "LanguageUtilTest.moc"
