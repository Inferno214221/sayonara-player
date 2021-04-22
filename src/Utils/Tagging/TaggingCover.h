/* TaggingCover.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#ifndef SAYONARA_TAGGING_COVER_H
#define SAYONARA_TAGGING_COVER_H

class QPixmap;
class QString;
class QByteArray;

namespace Tagging
{
	struct ParsedTag;

	bool writeCover(const QString& filepath, const QPixmap& image);
	bool writeCover(const QString& filepath, const QString& imagePath);

	bool extractCover(const ParsedTag& parsedTag, QByteArray& coverData, QString& mimeType);
	bool extractCover(const QString& filepath, QByteArray& coverData, QString& mimeType);
	QPixmap extractCover(const QString& filepath);

	bool hasCover(const ParsedTag& parsedTag);
	bool hasCover(const QString& filepath);

	bool isCoverSupported(const QString& filepath);
}

#endif // SAYONARA_TAGGING_COVER_H
