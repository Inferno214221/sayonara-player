/* SettingsTest.cpp
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

#include "Database/Connector.h"
#include "Database/Settings.h"
#include "Database/StandardConnectorFixes.h"

#include "Utils/Settings/SettingRegistry.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Macros.h"
#include "Utils/FileUtils.h"
#include "Utils/Utils.h"

class SettingsTest :
	public Test::Base
{
	Q_OBJECT

	public:
		SettingsTest() :
			Test::Base("SettingsTest") {}

		~SettingsTest() override = default;

	private slots:
		void testDatabaseVersion();
		void testUndeployableKeysNotInDatabase();
		void testDefaultValues();
		void testSpecialSettings();
};

void SettingsTest::testDatabaseVersion()
{
	auto* db = DB::Connector::instance();
	QVERIFY(db->db().isOpen());

	auto* settings = Settings::instance();
	QVERIFY(settings->checkSettings());

	auto dbVersion = QString {};
	db->settingsConnector()->loadSetting("version", dbVersion);

	const auto maxDbVersion = DB::StandardConnectorFixes::latestDatabaseVersion();
	QVERIFY(dbVersion.toInt() == maxDbVersion);
}

void SettingsTest::testUndeployableKeysNotInDatabase()
{
	// if this test fails, you should call the create_db binary first
	// your database is not up to date
	const auto undeployableKeys = SettingRegistry::undeployableKeys();

	auto* db = DB::Connector::instance();
	QList<SettingKey> keys;
	db->settingsConnector()->loadSettings(keys);

	const auto maxKey = static_cast<int>(SettingKey::Num_Setting_Keys);
	const auto keyCount = keys.count();
	const auto undeployableKeyCount = undeployableKeys.count();
	QVERIFY(keyCount == (maxKey - undeployableKeyCount));

	{ // undeployable keys must not be in keys
		for(const auto settingKey: undeployableKeys)
		{
			QVERIFY(keys.contains(settingKey) == false);
		}
	}
}

void SettingsTest::testDefaultValues()
{
	const auto undeployableKeys = SettingRegistry::undeployableKeys();

	auto* settings = Settings::instance();

	const auto abstractSettings = settings->settings();
	for(auto* abstractSetting: abstractSettings)
	{
		if(undeployableKeys.contains(abstractSetting->getKey()))
		{
			continue;
		}

		const auto settingValue = abstractSetting->valueToString();
		abstractSetting->assignDefaultValue();
		const auto newValue = abstractSetting->valueToString();

		if(settingValue != newValue)
		{
			qDebug() << "Error with " << abstractSetting->dbKey();
			qDebug() << "Current value: " << settingValue;
			qDebug() << "Default value: " << newValue;
		}

		QVERIFY(settingValue == newValue);
	}
}

void SettingsTest::testSpecialSettings()
{
	QVERIFY(GetSetting(Set::Player_Fullscreen) == false);
	QVERIFY(GetSetting(Set::Player_Maximized) == false);

	const auto lastFmUsername = GetSetting(Set::LFM_Username);
	const auto lastFmPassword = GetSetting(Set::LFM_Password);
	const auto lastFmLogin = GetSetting(Set::LFM_Login);
	const auto lastFmSessionKey = GetSetting(Set::LFM_SessionKey);
	QVERIFY(lastFmUsername.isEmpty());
	QVERIFY(lastFmPassword.isEmpty());
	QVERIFY(lastFmLogin.first.isEmpty());
	QVERIFY(lastFmLogin.second.isEmpty());
	QVERIFY(lastFmSessionKey.isEmpty());

	const auto proxyUsername = GetSetting(Set::Proxy_Username);
	const auto proxyPassword = GetSetting(Set::Proxy_Password);
	const auto proxyHostname = GetSetting(Set::Proxy_Hostname);
	QVERIFY(proxyUsername.isEmpty());
	QVERIFY(proxyPassword.isEmpty());
	QVERIFY(proxyHostname.isEmpty());

	QVERIFY(GetSetting(Set::Engine_Vol) > 10);

	QVERIFY(GetSetting(Set::Lib_FontBold) == true);
	QVERIFY(GetSetting(Set::Logger_Level) == 0);

	QVERIFY(GetSetting(Set::Player_PublicId).isEmpty());
	QVERIFY(GetSetting(Set::Player_PrivId).isEmpty());
}

QTEST_GUILESS_MAIN(SettingsTest)

#include "SettingsTest.moc"
