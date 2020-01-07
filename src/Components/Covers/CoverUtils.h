/* CoverHelper.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef COVERHELPER_H
#define COVERHELPER_H

namespace DB
{
	class Connector;
}

class QString;
class QPixmap;
/**
 * @ingroup Covers
 */
namespace Cover
{
	enum Source
	{
		Database,
		SayonaraDir,
		Library,
		AudioFile,
		WWW,
		Unknown
	};


	class Location;
	namespace Utils
	{
		/**
		 * @brief calc_cover_token calculate the hash for a cover
		 * @param artist artist name
		 * @param album album name
		 * @return hash of cover name
		 */
		QString calc_cover_token(const QString& artist, const QString& album);

		bool add_temp_cover(const QPixmap& pm, const QString& hash);
		void delete_temp_covers();

		QString cover_directory();
		QString cover_directory(const QString& append_filename);
		QString cover_temp_directory();

		void write_cover_to_sayonara_dir(const Cover::Location& cl, const QPixmap& pm);
		void write_cover_to_db(const Cover::Location& cl, const QPixmap& pm);
		void write_cover_to_db(const Cover::Location& cl, const QPixmap& pm, DB::Connector* db);
		void write_cover_to_library(const Cover::Location& cl, const QPixmap& pm);

	}
}

#endif // COVERHELPER_H
