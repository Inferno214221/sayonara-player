#include "SayonaraTest.h"

#include "Utils/Macros.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Language/LanguageUtils.h"
#include "Utils/Settings/Settings.h"
#include <QStringList>

using namespace Util;

class LanguageUtilTest : public SayonaraTest
{
    Q_OBJECT

public:
	LanguageUtilTest() :
		SayonaraTest("LanguageUtilTest")
	{}

	~LanguageUtilTest() override = default;

private slots:
	void basic_path_tests();
	void language_version_test();
	void four_letter_test();
	void similar_language_test();
};

void LanguageUtilTest::basic_path_tests()
{
	QString sp = Util::share_path("translations");
	QString hp = Util::sayonara_path("translations");

	QString path;
	QString expected;

	path = Language::get_share_path("ab_CD");
	expected = File::clean_filename(sp + "/sayonara_lang_ab_CD.qm");
	QVERIFY(path == expected);

	path = Language::get_ftp_path("ab_CD");
	expected = QString("ftp://sayonara-player.com/translation/sayonara_lang_ab_CD.qm");
	QVERIFY(path == expected);

	path = Language::get_http_path("ab_CD");
	expected = QString("https://sayonara-player.com/sw/translation/sayonara_lang_ab_CD.qm");
	QVERIFY(path == expected);

	path = Language::get_home_target_path("ab_CD");
	expected = File::clean_filename(hp + "/sayonara_lang_ab_CD.qm");
	QVERIFY(path == expected);

	path = Language::get_icon_path("ab_CD");
	expected = File::clean_filename(sp + "/icons/ab_CD.png");
	QVERIFY(path == expected);

	path = Language::get_share_path("hallo");
	QVERIFY(path.isEmpty());

	path = Language::get_share_path("");
	QVERIFY(path.isEmpty());

	path = Language::get_share_path("en_USS");
	QVERIFY(path.isEmpty());

	path = Language::get_share_path("EN_us");
	QVERIFY(path.isEmpty());
}

void LanguageUtilTest::language_version_test()
{
	Settings* s = Settings::instance();
	s->check_settings();

	QString current_version = SAYONARA_VERSION;
	current_version.replace("2", "3");
	current_version.replace("1", "2");
	current_version.replace("0", "1");

	Language::set_test_mode();

	Language::set_language_version("fr_FR", "1.2.0");
	Language::set_language_version("de_DE", current_version);
	Language::set_language_version("pt_PT", "");
	Language::set_language_version("ab_CD", SAYONARA_VERSION);

	bool outdated;
	outdated = Language::is_outdated("fr_FR");
	QVERIFY(outdated == true);

	outdated = Language::is_outdated("de_DE");
	QVERIFY(outdated == false);

	outdated = Language::is_outdated("pt_PT");
	QVERIFY(outdated == true);

	outdated = Language::is_outdated("ab_CD");
	QVERIFY(outdated == false);

	outdated = Language::is_outdated("");
	QVERIFY(outdated == true);
}

void LanguageUtilTest::four_letter_test()
{
	QString expected;
	QString four_letter;

	four_letter = Language::extract_four_letter("blupp.ts");
	expected = QString();
	QVERIFY(four_letter == expected);

	four_letter = Language::extract_four_letter("sayonara_lang_de_DE.ts");
	expected = "de_DE";
	QVERIFY(four_letter == expected);

	four_letter = Language::extract_four_letter("asd;flkjasdsayonara_lang_de_DE.qm");
	expected = "de_DE";
	QVERIFY(four_letter == expected);

	four_letter = Language::extract_four_letter("asd;flkjasdsayonara_lang_de_DE.xz");
	expected = QString();
	QVERIFY(four_letter == expected);

	four_letter = Language::extract_four_letter("asd;flkjasdsayonara_lang_DE_DE.qm");
	expected = QString();
	QVERIFY(four_letter == expected);

	four_letter = Language::extract_four_letter("blupp.ts");
	expected = QString();
	QVERIFY(four_letter == expected);
}

void LanguageUtilTest::similar_language_test()
{
//	QString similar_language, four_letter;
//	QString expected;

//	similar_language = Language::get_similar_language_4("de_DE");
//	four_letter = Language::extract_four_letter(similar_language);
//	expected = "de_DE";
//	QVERIFY(four_letter == expected);
//	QVERIFY(File::exists(similar_language));

//	similar_language = Language::get_similar_language_4("fr_AL");
//	four_letter = Language::extract_four_letter(similar_language);
//	expected = "fr_FR";
//	QVERIFY(four_letter == expected);
//	QVERIFY(File::exists(similar_language));

//	similar_language = Language::get_similar_language_4("fradsf");
//	four_letter = Language::extract_four_letter(similar_language);
//	expected = QString();
//	QVERIFY(four_letter == expected);
}

QTEST_GUILESS_MAIN(LanguageUtilTest)
#include "LanguageUtilTest.moc"
