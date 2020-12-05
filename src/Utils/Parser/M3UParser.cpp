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

#include <QRegExp>
#include <QStringList>

M3UParser::M3UParser(const QString& filename) :
	AbstractPlaylistParser(filename) {}

M3UParser::~M3UParser() = default;

void M3UParser::parse()
{
	const QStringList list = content().split('\n');

	MetaData md;

	for(QString line : list)
	{
		line = line.trimmed();
		if(line.isEmpty()){
			continue;
		}

		if(line.startsWith("#EXTINF:", Qt::CaseInsensitive)) {
			md = MetaData();
			parseFirstLine(line, md);
			continue;
		}

		if(line.trimmed().startsWith('#')) {
			continue;
		}

		if(Util::File::isPlaylistFile(line)){
			MetaDataList v_md = PlaylistParser::parsePlaylist(line);
			addTracks(v_md);
			continue;
		}

		else if( !Util::File::isWWW(line)) {
			parseLocalFile(line, md);
		}

		else {
			parseWWWFile(line, md);
		}

		if(!md.filepath().isEmpty()){
			addTrack(md);
			md = MetaData();
		}
	}
}

bool M3UParser::parseFirstLine(const QString& line, MetaData& md)
{
	if(line.indexOf("#EXTINF:") != 0)
	{
		return false;
	}

	QString substring;
	int currentIndex = 8;
	int newIndex = line.indexOf(',', 8);
	substring = line.mid(currentIndex, newIndex - currentIndex);

	md.setDurationMs(substring.toInt() * 1000);

	currentIndex = newIndex + 1;
	newIndex = line.indexOf(" - ", currentIndex);
	substring = line.mid(currentIndex, newIndex - currentIndex);

	if(newIndex > 0)
	{
		md.setArtist(substring);
		currentIndex = newIndex + 3;
		substring = line.mid(currentIndex);
	}

	md.setTitle(substring);

	return true;
}

void M3UParser::parseLocalFile(const QString& line, MetaData& md)
{
	QString absoluteFilename = getAbsoluteFilename(line);
	if(absoluteFilename.isEmpty()){
		return;
	}

	md.setFilepath(absoluteFilename);
	Tagging::Utils::getMetaDataOfFile(md);
}

void M3UParser::parseWWWFile(const QString& line, MetaData& md)
{
	md.setRadioStation(line);
	md.setFilepath(line);
}
