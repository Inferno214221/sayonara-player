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

