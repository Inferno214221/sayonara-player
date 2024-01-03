/* CreateDB.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

	if(argc < 2)
	{
		std::cerr << "Usage " << argv[0] << " <out_dir>" << std::endl;
		return 1;
	}

	Q_INIT_RESOURCE(Database);

	QString sourceDirectory(":/Database");
	QString targetDirectory(argv[1]);
	QString databseFilename("player.db");

	Util::File::deleteFiles({databseFilename});
	DB::Connector* db = DB::Connector::customInstance(sourceDirectory, targetDirectory, databseFilename);
	DB::Settings* setting_connector = db->settingsConnector();

	Settings* settings = Settings::instance();
	settings->checkSettings();
	QList<SettingKey> invalid_keys = SettingRegistry::undeployableKeys();
	SettingArray arr = settings->settings();

	QList<AbstrSetting*> invalid_settings;
	for(AbstrSetting* s: arr)
	{
		if(!s->isDatabaseSetting())
		{
			continue;
		}

		if(invalid_keys.contains(s->getKey()))
		{
			invalid_settings << s;
		}

		s->assignDefaultValue();
	}

	setting_connector->storeSettings();

	for(AbstrSetting* s: invalid_settings)
	{
		setting_connector->dropSetting(s->dbKey());
	}

	std::cout << "Written to " << QDir(targetDirectory).absoluteFilePath(databseFilename).toStdString() << std::endl;

	return 0;
}
