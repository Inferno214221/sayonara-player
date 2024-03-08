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

namespace
{
	void initSettings(DB::Settings* settingsConnector)
	{
		auto* settings = Settings::instance();
		settings->checkSettings();

		const auto invalidKeys = SettingRegistry::undeployableKeys();
		const auto settingsArray = settings->settings();

		QList<AbstrSetting*> invalidSettings;
		for(auto* abstractSetting: settingsArray)
		{
			if(!abstractSetting->isDatabaseSetting())
			{
				continue;
			}

			if(invalidKeys.contains(abstractSetting->getKey()))
			{
				invalidSettings << abstractSetting;
			}

			abstractSetting->assignDefaultValue();
		}

		settingsConnector->storeSettings();

		for(auto* abstractSetting: invalidSettings)
		{
			settingsConnector->dropSetting(abstractSetting->dbKey());
		}
	}
}

int main(int argc, char** argv)
{
	auto app = QApplication(argc, argv);

	if(argc < 2)
	{
		std::cerr << "Usage " << argv[0] << " <out_dir>" << std::endl;
		return 1;
	}

	Q_INIT_RESOURCE(Database);

	const auto sourceDirectory = QString(":/Database");
	const auto targetDirectory = QString(argv[1]);
	const auto databseFilename = QString("player.db");

	Util::File::deleteFiles({databseFilename});
	auto* connector = DB::Connector::customInstance(sourceDirectory, targetDirectory, databseFilename);

	initSettings(connector->settingsConnector());

	std::cout << "Written to " << QDir(targetDirectory).absoluteFilePath(databseFilename).toStdString() << std::endl;

	return 0;
}
