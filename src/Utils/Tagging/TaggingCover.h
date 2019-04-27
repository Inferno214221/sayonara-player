/* TaggingCover.h */

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

#ifndef TAGGINGCOVER_H
#define TAGGINGCOVER_H

class QPixmap;
class QString;
class QByteArray;

namespace Tagging
{
	struct ParsedTag;

	namespace Covers
	{
		bool write_cover(const QString& filepath, const QPixmap& image);
		bool write_cover(const QString& filepath, const QString& image_path);

		bool extract_cover(const ParsedTag& parsed_tag, QByteArray& cover_data, QString& mime_type);
		bool extract_cover(const QString& filepath, QByteArray& cover_data, QString& mime_type);
		QPixmap extract_cover(const QString& filepath);

		bool has_cover(const ParsedTag& parsed_tag);
		bool has_cover(const QString& filepath);

		bool is_cover_supported(const QString& filepath);
	}
}

#endif // TAGGINGCOVER_H
