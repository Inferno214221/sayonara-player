#include <QObject>
#include <QTest>
#include <QDebug>
#include <QString>

#include <QStringList>
#include "Utils/Crypt.h"
#include "Utils/Utils.h"

#include <algorithm>

class CryptTest : public QObject
{
	Q_OBJECT

private slots:
	void crypttest();
};


void CryptTest::crypttest()
{
	QByteArray key = Util::randomString(32).toLocal8Bit();

	QStringList sources
	{
		"Das hier ist ein ganz langer string ohne irgendwelchen speziellen Dinge",
		QString::fromLocal8Bit("Hier ein päär ßönderzeichen"),
		QString::fromLocal8Bit("Выбор и предварительный просмотр нескольких обложек")
	};

	for(const QString& source : sources)
	{
		QString enc = Util::Crypt::encrypt(source, key);
		QString dec = Util::Crypt::decrypt(enc, key);

		QVERIFY(source == dec);
	}
}

QTEST_GUILESS_MAIN(CryptTest)

#include "CryptTest.moc"
