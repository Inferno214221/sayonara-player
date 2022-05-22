#include <QTest>
#include <QObject>
#include "Utils/Language/Language.h"

class LanguageKeyTest :
	public QObject
{
	Q_OBJECT

	private slots:
		void test();
};

void LanguageKeyTest::test()
{
	const auto maxKey = static_cast<int>(Lang::NUMBER_OF_LANGUAGE_KEYS);
	for(auto key = 0; key < maxKey; key++)
	{
		auto ok = false;
		Lang::get(static_cast<Lang::Term>(key), &ok);

		QVERIFY(ok);
	}

	auto ok = false;
	Lang::get(static_cast<Lang::Term>(maxKey), &ok);
	QVERIFY(!ok);
}

QTEST_GUILESS_MAIN(LanguageKeyTest)

#include "LanguageKeyTest.moc"
