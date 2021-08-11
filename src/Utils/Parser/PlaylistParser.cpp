/* PlaylistParser.cpp */

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

#include "M3UParser.h"
#include "PLSParser.h"
#include "ASXParser.h"

#include "Utils/Parser/PlaylistParser.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QUrl>
#include <QDir>
#include <QFile>

MetaDataList PlaylistParser::parsePlaylist(const QString& localFilename)
{
	if(Util::File::isWWW(localFilename))
	{
		return MetaDataList();
	}

	MetaDataList result;
	MetaDataList tmpTracks;
	MetaDataList tracksToDelete;
	AbstractPlaylistParser* playlistParser;

	if(localFilename.endsWith(QStringLiteral("m3u"), Qt::CaseInsensitive))
	{
		playlistParser = new M3UParser(localFilename);
	}

	else if(localFilename.endsWith(QStringLiteral("pls"), Qt::CaseInsensitive))
	{
		playlistParser = new PLSParser(localFilename);
	}

	else if(localFilename.endsWith(QStringLiteral("ram"), Qt::CaseInsensitive))
	{
		playlistParser = new M3UParser(localFilename);
	}

	else if(localFilename.endsWith(QStringLiteral("asx"), Qt::CaseInsensitive))
	{
		playlistParser = new ASXParser(localFilename);
	}

	else
	{
		playlistParser = new M3UParser(localFilename);
		tmpTracks = playlistParser->tracks();

		if(tmpTracks.isEmpty())
		{
			delete playlistParser;
			playlistParser = new PLSParser(localFilename);
			tmpTracks = playlistParser->tracks();
		}

		if(tmpTracks.isEmpty())
		{
			delete playlistParser;
			playlistParser = nullptr;
			playlistParser = new ASXParser(localFilename);
		}
	}

	tmpTracks = playlistParser->tracks();

	for(const auto& track : tmpTracks)
	{
		if(Util::File::checkFile(track.filepath()))
		{
			result << track;
		}
	}

	if(playlistParser)
	{
		delete playlistParser;
		playlistParser = nullptr;
	}

	result.removeDuplicates();

	return result;
}

void PlaylistParser::saveM3UPlaylist(const QString& filename, const MetaDataList& tracks, bool relative)
{
	auto f = filename;
	if(!f.endsWith("m3u", Qt::CaseInsensitive))
	{
		f.append(".m3u");
	}

	bool success;
	const auto dirString = f.left(f.lastIndexOf(QDir::separator()));
	QDir dir(dirString);
	dir.cd(dirString);

	QFile file(f);
	success = file.open(QIODevice::WriteOnly);
	if(!success)
	{
		return;
	}

	file.write(QByteArray("#EXTM3U\n"));

	auto lines = 0L;
	for(const auto& track : tracks)
	{
		QString str;
		if(relative)
		{
			str = dir.relativeFilePath(track.filepath());
		}

		else
		{
			str = track.filepath();
		}

		const auto extData =
			"#EXTINF: " + QString::number(track.durationMs() / 1000) + ", " + track.artist() + " - " + track.title() + "\n";
		lines += file.write(extData.toLocal8Bit());
		lines += file.write(str.toLocal8Bit());
		lines += file.write(QByteArray("\n"));
	}

	file.close();
}
