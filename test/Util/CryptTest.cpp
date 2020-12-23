#include "SayonaraTest.h"
#include "Utils/Crypt.h"
// access working directory with Test::Base::tempPath("somefile.txt");

class CryptTest :
	public Test::Base
{
	Q_OBJECT

	public:
		CryptTest() :
			Test::Base("CryptTest")
		{}

	private slots:
		void test();
};

void CryptTest::test()
{
	const QStringList sources
		{
			"Das hier ist ein ganz langer string ohne irgendwelchen speziellen Dinge",
			QString::fromLocal8Bit("Hier ein päär ßönderzeichen"),
			QString::fromLocal8Bit("Выбор и предварительный просмотр нескольких обложек")
		};

	for(const auto& data : sources)
	{
		{
			const auto key = "small key";
			const auto encrypted = Util::Crypt::encrypt(data, key);
			QVERIFY(encrypted != data);

			const auto decrypted = Util::Crypt::decrypt(encrypted, key);
			QVERIFY(decrypted == data);

			const auto decrypted2 = Util::Crypt::decrypt(encrypted, "key");
			QVERIFY(decrypted2 != data);
		}

		{
			const auto key = "This is a really long key. Even bigger than the input data. ";
			const auto encrypted = Util::Crypt::encrypt(data, key);
			QVERIFY(encrypted != data);

			const auto decrypted = Util::Crypt::decrypt(encrypted, key);
			QVERIFY(decrypted == data);

			const auto decrypted2 = Util::Crypt::decrypt(encrypted, "key");
			QVERIFY(decrypted2 != data);
		}
	}
}

QTEST_GUILESS_MAIN(CryptTest)

#include "CryptTest.moc"
