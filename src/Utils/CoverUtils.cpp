/* CoverHelper.cpp */

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

#include "CoverUtils.h"
#include "FileUtils.h"
#include "StandardPaths.h"
#include "Utils.h"

#include <QPixmap>
#include <QDir>
#include <QStringList>

QString Util::Covers::calcCoverToken(const QString& artist, const QString& album)
{
	const auto data = QString(artist.trimmed() + album.trimmed()).toLower().toUtf8();
	return Util::calcHash(data);
}

void Util::Covers::deleteTemporaryCovers()
{
	const auto dir = Util::coverTempDirectory();
	Util::File::removeFilesInDirectory(dir);
}
