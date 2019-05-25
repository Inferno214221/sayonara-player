/* DatabaseVisualStyles.h */

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

#ifndef DATABASEVISSTYLES_H
#define DATABASEVISSTYLES_H

#include "Database/Module.h"
#include <QList>

struct RawColorStyle;

namespace DB
{
	class VisualStyles : private Module
	{
		public:
			VisualStyles(const QString& connection_name, DbId db_id);
			~VisualStyles();

			QList<RawColorStyle> get_raw_color_styles();
			bool insert_raw_color_style_to_db(const RawColorStyle& rcs);
			bool update_raw_color_style(const RawColorStyle& rcs);
			bool raw_color_style_exists(QString name);
			bool delete_raw_color_style(QString name);
	};
}

#endif // DATABASEVISSTYLES_H
