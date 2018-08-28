#include "Database/Connector.h"
#include "Database/Settings.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Macros.h"

#include <QTest>
#include <QObject>

class SettingsTest : public QObject
{
	Q_OBJECT

private slots:
	void test_registry();
};

void SettingsTest::test_registry()
{
	Settings* s = Settings::instance();
	bool checked = s->check_settings();

	QVERIFY(checked == true);
	QVERIFY(s->get<Set::Player_Version>().compare(SAYONARA_VERSION) == 0);

	DB::Connector* db = DB::Connector::instance("/tmp", "player.db");
	db->settings_connector()->load_settings();

	QVERIFY(db->db().isOpen());
	QVERIFY(s->get<Set::Player_Fullscreen>() == false);
	QVERIFY(s->get<Set::Player_Maximized>() == false);

	QString lfm_username = s->get<Set::LFM_Username>();
	QString lfm_password = s->get<Set::LFM_Password>();
	StringPair lfm_login = s->get<Set::LFM_Login>();
	QString lfm_session_key = s->get<Set::LFM_SessionKey>();
	QVERIFY(lfm_username.isEmpty());
	QVERIFY(lfm_password.isEmpty());
	QVERIFY(lfm_login.first.isEmpty());
	QVERIFY(lfm_login.second.isEmpty());
	QVERIFY(lfm_session_key.isEmpty());

	QString proxy_username = s->get<Set::Proxy_Username>();
	QString proxy_password = s->get<Set::Proxy_Password>();
	QString proxy_hostname = s->get<Set::Proxy_Hostname>();
	QVERIFY(proxy_username.isEmpty());
	QVERIFY(proxy_password.isEmpty());
	QVERIFY(proxy_hostname.isEmpty());

	QVERIFY(s->get<Set::Engine_Vol>() > 10);

	QVERIFY(s->get<Set::PL_FontSize>() == -1);
	QVERIFY(s->get<Set::Lib_FontBold>() == true);
	QVERIFY(s->get<Set::Lib_FontSize>() == -1);
	QVERIFY(s->get<Set::Player_FontSize>() > 6);
	QVERIFY(s->get<Set::Logger_Level>() == 0);
}


QTEST_MAIN(SettingsTest)

#include "SettingsTest.moc"
