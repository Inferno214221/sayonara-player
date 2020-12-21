/* SimilarArtists.cpp */

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

#include "SimilarArtists.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Compressor/Compressor.h"
#include "Utils/StandardPaths.h"

#include <QDir>
#include <QMap>
#include <QStringList>

namespace
{
	QString getFilename(const QString& artist)
	{
		const auto similarArtistPath = Util::similarArtistsPath();

		const auto dir = QDir(similarArtistPath);
		const auto files = dir.entryList(
			QStringList {"*.comp"},
			static_cast<QDir::Filters>(QDir::Files)
		);

		if(files.isEmpty())
		{
			return QString();
		}

		const auto targetName = artist + ".comp";
		for(const auto& filename : files)
		{
			if(filename.compare(targetName, Qt::CaseInsensitive) == 0)
			{
				return dir.filePath(targetName);
			}
		}

		return QString();
	}
}

QMap<QString, double>
SimilarArtists::getSimilarArtists(const QString& artist)
{
	QMap<QString, double> similarArtistsMap;
	const auto filename = getFilename(artist);
	if(filename.isEmpty())
	{
		return similarArtistsMap;
	}

	QByteArray content;
	const auto success = Util::File::readFileIntoByteArray(filename, content);
	if(!success)
	{
		return similarArtistsMap;
	}

	const auto decomp = Compressor::decompress(content);
	if(decomp.isEmpty())
	{
		return similarArtistsMap;
	}

	const auto similarArtists = QString::fromLocal8Bit(decomp).split("\n");
	for(const auto& similarArtist : similarArtists)
	{
		const auto lst = similarArtist.split('\t');
		if(lst.size() < 3)
		{
			continue;
		}

		const auto& match = lst[0];
		const auto& artistName = lst[2];

		similarArtistsMap[artistName] = match.toDouble();
	}

	return similarArtistsMap;
}

QStringList SimilarArtists::getSimilarArtistNames(const QString& artist)
{
	return QStringList(getSimilarArtists(artist).keys());
}
