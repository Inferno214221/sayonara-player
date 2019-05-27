#include "Database/Connector.h"
#include "Database/Settings.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Macros.h"
#include "Utils/FileUtils.h"

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
	Util::File::delete_files({"/tmp/player.db"});
	Settings* s = Settings::instance();
	bool checked = s->check_settings();
	QVERIFY(checked == true);

	QVERIFY(GetSetting(Set::Player_Language) == QLocale().name());
	QVERIFY(GetSetting(Set::Player_PublicId).isEmpty());
	QVERIFY(GetSetting(Set::Player_PrivId).isEmpty());

	DB::Connector* db = DB::Connector::instance("/tmp", "player.db");
	db->settings_connector()->load_settings();

	qDebug() << "Player Version " << GetSetting(Set::Player_Version);
	QVERIFY(GetSetting(Set::Player_Version).isEmpty());
	QVERIFY(GetSetting(Set::Player_Language).isEmpty());
	QVERIFY(GetSetting(Set::Player_PublicId).isEmpty());
	QVERIFY(GetSetting(Set::Player_PrivId).isEmpty());

	QVERIFY(db->db().isOpen());
	QVERIFY(GetSetting(Set::Player_Fullscreen) == false);
	QVERIFY(GetSetting(Set::Player_Maximized) == false);

	QString lfm_username = GetSetting(Set::LFM_Username);
	QString lfm_password = GetSetting(Set::LFM_Password);
	StringPair lfm_login = GetSetting(Set::LFM_Login);
	QString lfm_session_key = GetSetting(Set::LFM_SessionKey);
	QVERIFY(lfm_username.isEmpty());
	QVERIFY(lfm_password.isEmpty());
	QVERIFY(lfm_login.first.isEmpty());
	QVERIFY(lfm_login.second.isEmpty());
	QVERIFY(lfm_session_key.isEmpty());

	QString proxy_username = GetSetting(Set::Proxy_Username);
	QString proxy_password = GetSetting(Set::Proxy_Password);
	QString proxy_hostname = GetSetting(Set::Proxy_Hostname);
	QVERIFY(proxy_username.isEmpty());
	QVERIFY(proxy_password.isEmpty());
	QVERIFY(proxy_hostname.isEmpty());

	QVERIFY(GetSetting(Set::Engine_Vol) > 10);

	QVERIFY(GetSetting(Set::PL_FontSize) == -1);
	QVERIFY(GetSetting(Set::Lib_FontBold) == true);
	QVERIFY(GetSetting(Set::Lib_FontSize) == -1);
	QVERIFY(GetSetting(Set::Player_FontSize) > 6);
	QVERIFY(GetSetting(Set::Logger_Level) == 0);
}


QTEST_MAIN(SettingsTest)

#include "SettingsTest.moc"
