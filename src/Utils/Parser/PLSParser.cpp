/* PLSParser.cpp */

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

#include "PLSParser.h"
#include "Utils/MetaData/MetaData.h"

#include <QRegExp>
#include <QStringList>

#include <algorithm>

struct LineEntry
{
	QString key;
	QString value;
	int track_idx;

	LineEntry()
	{
		track_idx = -1;
	}
};


static LineEntry split_line(const QString& line)
{
	LineEntry ret;

	int pos_idx;
	QRegExp re_idx("(\\S+)([0-9]+)");
	QStringList splitted = line.split("=");

	if(splitted.size() < 2){
		return ret;
	}

	pos_idx = re_idx.indexIn(splitted[0]);
	if(pos_idx < 0){
		ret.key = splitted[0];
		ret.value = splitted[1];
		ret.track_idx = 1;
	}

	else {
		ret.key = re_idx.cap(1).toLower();
		ret.value = splitted[1];
		ret.track_idx = re_idx.cap(2).toInt();
	}

	return ret;
}


PLSParser::PLSParser(const QString& filename) :
	AbstractPlaylistParser(filename) {}

PLSParser::~PLSParser() {}

void PLSParser::parse()
{
	QStringList lines = content().split("\n");

	MetaData md;
	int cur_track_idx = -1;

	for(QString line : lines) {
		line = line.trimmed();
		if(line.isEmpty() || line.startsWith("#")){
			continue;
		}

		LineEntry line_entry = split_line(line);

		if(line_entry.track_idx < 0){
			continue;
		}

		if(line_entry.track_idx != cur_track_idx){

			if(cur_track_idx > 0){
				add_track(md);
			}

			md = MetaData();
			cur_track_idx = line_entry.track_idx;
		}


		md.track_num = line_entry.track_idx;

		if(line_entry.key.startsWith("file", Qt::CaseInsensitive))
		{
			QString filepath = get_absolute_filename(line_entry.value);
			md.set_filepath(filepath);
			md.set_artist(filepath);
		}

		else if(line_entry.key.startsWith("title", Qt::CaseInsensitive))
		{
			md.set_title(line_entry.value);
		}

		else if(line_entry.key.startsWith("length", Qt::CaseInsensitive))
		{
			int len = line_entry.value.toInt();

			len = std::max(0, len);
			md.duration_ms = len * 1000;
		}
	}

	if(!md.filepath().isEmpty()){
		add_track(md);
	}
}

