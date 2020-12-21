/* CoverHelper.cpp */

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

#include "CoverUtils.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"

#include <QPixmap>
#include <QDir>
#include <QStringList>

QString Util::Covers::calcCoverToken(const QString& artist, const QString& album)
{
	const auto data = QString(artist.trimmed() + album.trimmed()).toLower().toUtf8();
	return Util::calcHash(data);
}

QString Util::Covers::coverDirectory()
{
	return coverDirectory(QString());
}

QString Util::Covers::coverDirectory(const QString& appendFilename)
{
	auto path = Util::sayonaraPath("covers");
	if(!Util::File::exists(path)){
		Util::File::createDir(path);
	}

	if(!appendFilename.isEmpty()){
		path += '/' + appendFilename;
	}

	return Util::File::cleanFilename(path);
}

QString Util::Covers::coverTempDirectory()
{
	return coverTempDirectory(QString());
}

QString Util::Covers::coverTempDirectory(const QString& appendFilename)
{
	auto path = Util::tempPath("covers");
	if(!Util::File::exists(path)){
		Util::File::createDir(path);
	}

	if(!appendFilename.isEmpty()){
		path += '/' + appendFilename;
	}

	return Util::File::cleanFilename(path);
}

void Util::Covers::deleteTemporaryCovers()
{
	const auto dir = coverTempDirectory();
	Util::File::removeFilesInDirectory(dir);
}
