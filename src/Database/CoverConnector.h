/* CoverConnector.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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
			Covers(const QString& connection_name, DbId db_id);
			~Covers();

			bool exists(const QString& hash);
			bool get_cover(const QString& hash, QPixmap& data);
			bool set_cover(const QString& hash, const QPixmap& data);

			bool update_cover(const QString& hash, const QPixmap& data);
			bool insert_cover(const QString& hash, const QPixmap& data);
			bool remove_cover(const QString& hash);

			Util::Set<QString> get_all_hashes();

			bool get_all_covers(QMap<QString, QPixmap>& covers);
			void clear();
	};
}

#endif // COVERCONNECTOR_H
