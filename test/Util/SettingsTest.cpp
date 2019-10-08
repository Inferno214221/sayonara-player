#include "Database/Connector.h"
#include "Database/Settings.h"

#include "Utils/Settings/SettingRegistry.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Macros.h"
#include "Utils/FileUtils.h"

#include <QTest>
#include <QObject>

class SettingsTest : public QObject
{
	Q_OBJECT

public:
	SettingsTest(QObject* parent=nullptr) :
		QObject(parent)
	{}

private slots:
	void initTestCase();
	void cleanupTestCase();
	void test_registry();
};


void SettingsTest::test_registry()
{
	Q_INIT_RESOURCE(Database);

	Settings* s = Settings::instance();
	bool checked = s->check_settings();
	QVERIFY(checked == true);

	//QVERIFY(GetSetting(Set::Player_Language) == QLocale().name());
	QVERIFY(GetSetting(Set::Player_PublicId).isEmpty());
	QVERIFY(GetSetting(Set::Player_PrivId).isEmpty());

	auto* db = DB::Connector::instance_custom("", "/tmp", "player.db");

	QVERIFY(db->db().isOpen());

	QList<SettingKey> keys;

	QString db_version;
	db->settings_connector()->load_setting("version", db_version);
	db->settings_connector()->load_settings(keys);

	{
		int old_db_version = db->old_db_version();
		int max_db_version = DB::Connector::get_max_db_version();

		QVERIFY(old_db_version == max_db_version);
		QVERIFY(db_version.toInt() == max_db_version);
	}

	QList<SettingKey> undeployable_keys = SettingRegistry::undeployable_keys();

	int max_key = int(SettingKey::Num_Setting_Keys);
	int c = keys.count();
	int uks = undeployable_keys.size();
	qDebug() << " c, uks, maxkey " << c << " " << uks << " " << max_key;

	for(int i=0; i<max_key; i++)
	{
		auto* ptr = Settings::instance()->setting( SettingKey(i) );
		QVERIFY(ptr != nullptr);
	}

	// if this test fails, you should call the create_db binary first
	// your database is not up to date
	QVERIFY(c == (max_key - uks));

	{ // undeployable keys must not be in keys
		for(SettingKey key : undeployable_keys)
		{
			QVERIFY(keys.contains(key) == false);
		}

		// some examples
		QVERIFY(undeployable_keys.contains(SettingKey::Player_Version));
		QVERIFY(undeployable_keys.contains(SettingKey::Player_Language));
		QVERIFY(undeployable_keys.contains(SettingKey::Player_PublicId));
		QVERIFY(undeployable_keys.contains(SettingKey::Player_PrivId));
	}


	{ // test for default values
		SettingArray abstr_settings = s->settings();
		for(AbstrSetting* abstr_setting : abstr_settings)
		{
			if(undeployable_keys.contains(abstr_setting->get_key())){
				continue;
			}

			QString str = abstr_setting->value_to_string();
			abstr_setting->assign_default_value();
			QString new_val = abstr_setting->value_to_string();
			if(str != new_val){
				qDebug() << "Error with " << abstr_setting->db_key();
				qDebug() << "Current value: " << str;
				qDebug() << "Default value: " << new_val;
			}
			QVERIFY(str == new_val);
		}
	}

	{  // Actually not needed, but it does not affect tests
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
		QVERIFY(GetSetting(Set::Player_FontName).isEmpty());
		QVERIFY(GetSetting(Set::Player_FontSize) == 0);
		QVERIFY(GetSetting(Set::Logger_Level) == 0);
	}
}

void SettingsTest::initTestCase()
{
	Util::File::delete_files({"/tmp/player.db"});
}

void SettingsTest::cleanupTestCase()
{
	Util::File::delete_files({"/tmp/player.db"});
}

QTEST_GUILESS_MAIN(SettingsTest)

#include "SettingsTest.moc"
