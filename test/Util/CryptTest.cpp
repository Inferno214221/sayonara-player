/* CryptTest.cpp
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
