#include <QTest>
#include <QObject>
#include "Utils/Language/Language.h"

class LanguageKeyTest : public QObject
{
    Q_OBJECT

private slots:
    void test();

};

void LanguageKeyTest::test()
{
    int max_key = (int) (Lang::NUMBER_OF_LANGUAGE_KEYS);
    for(int key=0; key<max_key; key++)
    {
	bool ok;
	Lang::get( (Lang::Term) key, &ok );

	QVERIFY(ok);
    }

    bool ok;
    Lang::get((Lang::Term) max_key, &ok);
    QVERIFY(!ok);
}

QTEST_MAIN(LanguageKeyTest)
#include "LanguageKeyTest.moc"
