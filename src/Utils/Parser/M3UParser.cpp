/* M3UParser.cpp */

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

#include "PlaylistParser.h"
#include "M3UParser.h"

#include "Tagging/Tagging.h"

#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QStringList>

namespace
{
	bool parseFirstLine(const QString& line, MetaData& track)
	{
		if(line.indexOf("#EXTINF:") != 0)
		{
			return false;
		}

		const auto regex = QStringLiteral("#EXTINF:\\s*([0-9]+)\\s*,\\s*(.*) - (.*)");
		auto re = QRegExp(regex);
		re.setMinimal(false);
		if(re.indexIn(line) >= 0)
		{
			if(track.title().isEmpty())
			{
				track.setTitle(re.cap(3).trimmed());
			}

			if(track.artist().isEmpty())
			{
				track.setArtist(re.cap(2).trimmed());
			}

			if(track.durationMs() <= 0)
			{
				track.setDurationMs(re.cap(1).trimmed().toInt() * 1000);
			}
		}

		return true;
	}

	template<typename AbsuluteFilenameFunction>
	void parseLocalFile(const QString& line, MetaData& track, AbsuluteFilenameFunction fn)
	{
		const auto absoluteFilename = fn(line);
		if(absoluteFilename.isEmpty())
		{
			return;
		}

		track.setFilepath(absoluteFilename);
		Tagging::Utils::getMetaDataOfFile(track);
	}

	void parseWWWFile(const QString& line, MetaData& track)
	{
		track.setRadioStation(line);
		track.setFilepath(line);
	}
}


M3UParser::M3UParser(const QString& filename) :
	AbstractPlaylistParser(filename) {}

M3UParser::~M3UParser() = default;

void M3UParser::parse()
{
	MetaData track;

	const auto lines = content().split('\n');
	for(auto line : lines)
	{
		line = line.trimmed();
		if(line.isEmpty())
		{
			continue;
		}

		if(line.startsWith("#EXTINF:", Qt::CaseInsensitive))
		{
			if(!track.filepath().isEmpty())
			{
				addTrack(track);
				track = MetaData();
			}

			parseFirstLine(line, track);
			continue;
		}

		if(line.trimmed().startsWith('#'))
		{
			continue;
		}

		if(Util::File::isPlaylistFile(line))
		{
			addTracks(PlaylistParser::parsePlaylist(line));
			continue;
		}

		else if(!Util::File::isWWW(line))
		{
			auto lambda = [&](const auto& filename) -> QString {
				return this->getAbsoluteFilename(filename);
			};

			parseLocalFile(line, track, lambda);
		}

		else
		{
			parseWWWFile(line, track);
		}
	}

	if(!track.filepath().isEmpty())
	{
		addTrack(track);
		track = MetaData();
	}
}


void M3UParser::saveM3UPlaylist(QString filename, const MetaDataList& tracks, bool relative)
{
	if(Util::File::getFileExtension(filename).toLower() != QStringLiteral("m3u"))
	{
		filename.append(".m3u");
	}

	auto file = QFile(filename);
	const auto success = file.open(QIODevice::WriteOnly);
	if(!success)
	{
		return;
	}

	auto dir = QDir(Util::File::getParentDirectory(filename));
	auto lines = QStringList() << QStringLiteral("#EXTM3U");
	for(const auto& track : tracks)
	{
		lines << QString("#EXTINF:%1,%2 - %3") // no, there is no space missing here.
			.arg(track.durationMs() / 1000)
			.arg(track.artist())
			.arg(track.title());

		const auto filepath = (relative)
		                      ? dir.relativeFilePath(track.filepath())
		                      : track.filepath();

		lines << filepath;
		lines << QString();
	}

	const auto text = lines.join("\n");
	file.write(text.toLocal8Bit());
	file.close();
}
