#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Database/Connector.h"
#include "Database/Settings.h"
#include "Utils/FileUtils.h"
#include <QApplication>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	Util::File::delete_files({"./player.db"});
	DB::Connector* db = DB::Connector::instance(".", "player.db");
	DB::Settings* setting_connector = db->settings_connector();

	Settings* settings = Settings::instance();
	settings->check_settings();

	QList<SettingKey> invalid_keys
	{
		SettingKey::Player_Version,
		SettingKey::Player_Language,
		SettingKey::Player_PrivId,
		SettingKey::Player_PublicId
	};

	QList<AbstrSetting*> invalid_settings;

	SettingArray arr = settings->settings();

	for(AbstrSetting* s : arr)
	{
		if(!s->is_db_setting()){
			continue;
		}

		if(invalid_keys.contains(s->get_key()))
		{
			invalid_settings << s;
		}

		s->assign_default_value();
	}

	setting_connector->store_settings();

	for(AbstrSetting* s : invalid_settings)
	{
		setting_connector->drop_setting(s->db_key());
	}

	return 0;
}
