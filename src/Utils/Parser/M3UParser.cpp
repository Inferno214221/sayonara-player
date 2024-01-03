/* M3UParser.cpp */

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

#include "PlaylistParser.h"
#include "M3UParser.h"

#include "Utils/FileSystem.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Tagging/TagReader.h"

#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QStringList>

namespace
{
	bool parseMetadataLine(const QString& line, MetaData& track)
	{
		const auto regex = QStringLiteral("#EXTINF:\\s*([0-9]+)\\s*,\\s*(.*)\\s+-\\s+(.*)");
		auto re = QRegExp(regex);
		re.setMinimal(false);
		if(re.indexIn(line) >= 0)
		{
			track.setTitle(re.cap(3).trimmed());
			track.setArtist(re.cap(2).trimmed());
			track.setDurationMs(re.cap(1).trimmed().toInt() * 1000);
		}

		return true;
	}

	void parseWWWFile(const QString& line, MetaData& track)
	{
		track.setRadioStation(line);
		track.setFilepath(line);
	}

	bool isComment(const QString& line)
	{
		return (line.startsWith('#') && !line.toLower().startsWith("#extinf"));
	}

	bool isMetadata(const QString& line)
	{
		return line.toLower().startsWith("#extinf:");
	}
}

M3UParser::M3UParser(const QString& filename,
                     const Util::FileSystemPtr& fileSystem,
                     const Tagging::TagReaderPtr& tagReader) :
	AbstractPlaylistParser(filename, fileSystem, tagReader) {}

M3UParser::~M3UParser() = default;

void M3UParser::parse()
{
	MetaData track;

	const auto lines = content().split('\n');
	for(auto it = lines.begin(); it != lines.end(); it++)
	{
		const auto line = it->trimmed();
		if(line.isEmpty() || isComment(line))
		{
			continue;
		}

		if(isMetadata(line))
		{
			parseMetadataLine(line, track);
		}

		else if(Util::File::isPlaylistFile(line))
		{
			parseSubPlaylist(line);
		}

		else if(Util::File::isSoundFile(line))
		{
			if(Util::File::isWWW(line))
			{
				parseWWWFile(line, track);
			}

			else
			{
				const auto absoluteFilename = this->getAbsoluteFilename(line);
				if(!absoluteFilename.isEmpty())
				{
					track.setFilepath(absoluteFilename);
				}
			}

			addTrack(track);
			track = MetaData {};
		}
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
	for(const auto& track: tracks)
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

	const auto text = lines.join('\n');
	file.write(text.toLocal8Bit());
	file.close();
}
