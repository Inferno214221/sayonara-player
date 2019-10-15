/* CreateDB.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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

#include "Utils/Settings/SettingRegistry.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Database/Connector.h"
#include "Database/Settings.h"
#include "Utils/FileUtils.h"

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

	Q_INIT_RESOURCE(Database);

	QString source_dir(":/Database");
	QString target_dir(argv[1]);
	QString db_filename("player.db");

	Util::File::delete_files({db_filename});
	DB::Connector* db = DB::Connector::instance_custom(source_dir, target_dir, db_filename);
	DB::Settings* setting_connector = db->settings_connector();

	Settings* settings = Settings::instance();
	settings->check_settings();
	QList<SettingKey> invalid_keys = SettingRegistry::undeployable_keys();
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
