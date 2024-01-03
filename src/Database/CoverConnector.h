/* CoverConnector.h */

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



#ifndef COVERCONNECTOR_H
#define COVERCONNECTOR_H

#include "Database/Module.h"
#include "Utils/SetFwd.h"

#include <QMap>
#include <QString>
#include <QPixmap>

namespace DB
{
	class Covers :
		private DB::Module
	{
		public:
			Covers(const QString& connectionName, DbId databaseId);
			~Covers();

			bool exists(const QString& hash);
			bool getCover(const QString& hash, QPixmap& data);
			bool setCover(const QString& hash, const QPixmap& data);

			bool updateCover(const QString& hash, const QPixmap& data);
			bool insertCover(const QString& hash, const QPixmap& data);
			bool removeCover(const QString& hash);

			Util::Set<QString> getAllHashes();

			bool getAllCovers(QMap<QString, QPixmap>& covers);
			void clear();
	};
}

#endif // COVERCONNECTOR_H
