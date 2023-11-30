/* PLSParser.cpp */

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

#include "PLSParser.h"
#include "Utils/MetaData/MetaData.h"

#include <QRegExp>
#include <QStringList>

#include <algorithm>

struct LineEntry
{
	QString key;
	QString value;
	int trackIdx;

	LineEntry()
	{
		trackIdx = -1;
	}
};

static LineEntry split_line(const QString& line)
{
	LineEntry ret;

	int pos_idx;
	QRegExp re_idx("(\\S+)([0-9]+)");
	QStringList splitted = line.split("=");

	if(splitted.size() < 2)
	{
		return ret;
	}

	pos_idx = re_idx.indexIn(splitted[0]);
	if(pos_idx < 0)
	{
		ret.key = splitted[0];
		ret.value = splitted[1];
		ret.trackIdx = 1;
	}

	else
	{
		ret.key = re_idx.cap(1).toLower();
		ret.value = splitted[1];
		ret.trackIdx = re_idx.cap(2).toInt();
	}

	return ret;
}

PLSParser::PLSParser(const QString& filename) :
	AbstractPlaylistParser(filename) {}

PLSParser::~PLSParser() = default;

void PLSParser::parse()
{
	QStringList lines = content().split("\n");

	MetaData md;
	int cur_trackIdx = -1;

	for(QString line: lines)
	{
		line = line.trimmed();
		if(line.isEmpty() || line.startsWith("#"))
		{
			continue;
		}

		LineEntry line_entry = split_line(line);

		if(line_entry.trackIdx < 0)
		{
			continue;
		}

		if(line_entry.trackIdx != cur_trackIdx)
		{

			if(cur_trackIdx > 0)
			{
				addTrack(md);
			}

			md = MetaData();
			cur_trackIdx = line_entry.trackIdx;
		}

		md.setTrackNumber(TrackNum(line_entry.trackIdx));

		if(line_entry.key.startsWith("file", Qt::CaseInsensitive))
		{
			QString filepath = getAbsoluteFilename(line_entry.value);
			md.setFilepath(filepath);
			md.setArtist(filepath);
		}

		else if(line_entry.key.startsWith("title", Qt::CaseInsensitive))
		{
			md.setTitle(line_entry.value);
		}

		else if(line_entry.key.startsWith("length", Qt::CaseInsensitive))
		{
			int len = line_entry.value.toInt();

			len = std::max(0, len);
			md.setDurationMs(len * 1000);
		}
	}

	if(!md.filepath().isEmpty())
	{
		addTrack(md);
	}
}

