#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Database/Connector.h"
#include "Database/Settings.h"
#include "Utils/FileUtils.h"
#include "DBMacros.h"

#include <QApplication>
#include <QDir>

#include <iostream>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	if(argc < 2){
		std::cerr << "Usage " << argv[0] << " <out_dir>" << std::endl;
		return 1;
	}

	QString source_dir(DB_SOURCE_DIR);
	QString target_dir(argv[1]);
	QString db_filename("player.db");

	Util::File::delete_files({db_filename});
	DB::Connector* db = DB::Connector::instance(source_dir, target_dir, db_filename);
	DB::Settings* setting_connector = db->settings_connector();

	Settings* settings = Settings::instance();
	settings->check_settings();
	QList<SettingKey> invalid_keys = settings->undeployed_keys();
	SettingArray arr = settings->settings();

	QList<AbstrSetting*> invalid_settings;
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

	std::cout << "Written to " << QDir(target_dir).absoluteFilePath(db_filename).toStdString() << std::endl;

	return 0;
}
