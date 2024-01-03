/* ShortcutIdentifierTest.cpp
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

#include "Gui/Utils/Shortcuts/ShortcutHandler.h"

class ShortcutIdentifierTest :
	public Test::Base
{
	Q_OBJECT

public:
	ShortcutIdentifierTest() :
		Test::Base("ShortcutIdentifierTest")
	{}

private slots:
	void shortcut_identifier_test();
};


void ShortcutIdentifierTest::shortcut_identifier_test()
{
	ShortcutHandler* sch = ShortcutHandler::instance();
	for(int i=0; i<(int) (ShortcutIdentifier::Invalid); i++)
	{
		ShortcutIdentifier sci = (ShortcutIdentifier) i;
		QString str = sch->identifier(sci);
		QVERIFY(str.size() > 0);
	}
}


QTEST_MAIN(ShortcutIdentifierTest)

#include "ShortcutIdentifierTest.moc"

