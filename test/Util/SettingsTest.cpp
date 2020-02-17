#include "SayonaraTest.h"

#include "Database/Connector.h"
#include "Database/Settings.h"

#include "Utils/Settings/SettingRegistry.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Macros.h"
#include "Utils/FileUtils.h"
#include "Utils/Utils.h"

class SettingsTest : public Test::Base
{
	Q_OBJECT

public:
	SettingsTest() :
		Test::Base("SettingsTest")
	{}

	~SettingsTest() override = default;

private slots:
	void test_registry();
};

void SettingsTest::test_registry()
{
	auto* db = DB::Connector::instance();
	QVERIFY(db->db().isOpen());

	Settings* s = Settings::instance();
	QVERIFY(s->checkSettings());

	//QVERIFY(GetSetting(Set::Player_Language) == QLocale().name());
	QVERIFY(GetSetting(Set::Player_PublicId).isEmpty());
	QVERIFY(GetSetting(Set::Player_PrivId).isEmpty());

	QString db_version;
	db->settingsConnector()->loadSetting("version", db_version);

	QList<SettingKey> keys;
	db->settingsConnector()->loadSettings(keys);

	{
		int old_db_version = db->oldDatabaseVersion();
		int max_db_version = DB::Connector::highestDatabaseVersion();

		QVERIFY(old_db_version == max_db_version);
		QVERIFY(db_version.toInt() == max_db_version);
	}

	QList<SettingKey> undeployable_keys = SettingRegistry::undeployableKeys();

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
			if(undeployable_keys.contains(abstr_setting->getKey())){
				continue;
			}

			QString str = abstr_setting->valueToString();
			abstr_setting->assignDefaultValue();
			QString new_val = abstr_setting->valueToString();
			if(str != new_val){
				qDebug() << "Error with " << abstr_setting->dbKey();
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

QTEST_GUILESS_MAIN(SettingsTest)

#include "SettingsTest.moc"
