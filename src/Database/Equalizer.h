/* Equalizer.h */
/*
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
#ifndef SAYONARA_PLAYER_DATABASE_EQUALIZER_H
#define SAYONARA_PLAYER_DATABASE_EQUALIZER_H

#include "Database/Module.h"
#include <QList>

class EqualizerSetting;

namespace DB
{
	class Equalizer :
		private Module
	{
		public:
			Equalizer(const QString& connectionName, DbId databaseId);
			~Equalizer() override;

			bool deleteEqualizer(int id);
			int insertEqualizer(const EqualizerSetting& equalizer);
			bool updateEqualizer(const EqualizerSetting& equalizer);

			bool fetchAllEqualizers(QList<EqualizerSetting>& equalizers);

			bool restoreFactoryDefaults();
			static QList<EqualizerSetting> factoryDefaults();
	};
}

#endif //SAYONARA_PLAYER_DATABASE_EQUALIZER_H
